/*
 * C-Sentinel - Semantic Observability for UNIX Systems
 * Copyright (c) 2025 William Murray
 *
 * Licensed under the MIT License.
 * See LICENSE file for details.
 *
 * https://github.com/williamofai/c-sentinel
 *
 * net_probe.c - Network state probing from /proc/net (Linux) or libperfstat (AIX)
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <arpa/inet.h>
#ifdef _AIX
#include <libperfstat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/procfs.h>
#include <fcntl.h>
#endif

#include "sentinel.h"

/* Common service ports - unusual if something else is listening */
static const uint16_t common_ports[] = {
    22,    /* SSH */
    25,    /* SMTP */
    53,    /* DNS */
    80,    /* HTTP */
    110,   /* POP3 */
    143,   /* IMAP */
    443,   /* HTTPS */
    465,   /* SMTPS */
    587,   /* Submission */
    993,   /* IMAPS */
    995,   /* POP3S */
    3306,  /* MySQL */
    5432,  /* PostgreSQL */
    6379,  /* Redis */
    8080,  /* HTTP Alt */
    8443,  /* HTTPS Alt */
    27017, /* MongoDB */
    0      /* Sentinel */
};

static int is_common_port(uint16_t port) {
    for (int i = 0; common_ports[i] != 0; i++) {
        if (common_ports[i] == port) return 1;
    }
    /* Ephemeral ports (>32768) are normal for outbound connections */
    if (port >= 32768) return 1;
    return 0;
}

/* Convert hex IP address from /proc/net to string */
static void hex_to_ip(const char *hex, char *ip, size_t ip_len, int is_ipv6) {
    if (is_ipv6) {
        /* IPv6 - simplified, just show as hex for now */
        snprintf(ip, ip_len, "%s", hex);
    } else {
        /* IPv4 - /proc stores as little-endian hex */
        unsigned int addr;
        sscanf(hex, "%X", &addr);
        snprintf(ip, ip_len, "%u.%u.%u.%u",
                 addr & 0xFF,
                 (addr >> 8) & 0xFF,
                 (addr >> 16) & 0xFF,
                 (addr >> 24) & 0xFF);
    }
}

/* Get process name from pid */
static void get_process_name(pid_t pid, char *name, size_t name_len) {
#ifdef _AIX
    /* AIX: Read from /proc/<pid>/psinfo */
    char path[64];
    struct psinfo psi;
    int fd;

    snprintf(path, sizeof(path), "/proc/%d/psinfo", (int)pid);
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        if (read(fd, &psi, sizeof(psi)) == sizeof(psi)) {
            snprintf(name, name_len, "%s", psi.pr_fname);
        } else {
            snprintf(name, name_len, "[unknown]");
        }
        close(fd);
    } else {
        snprintf(name, name_len, "[unknown]");
    }
#else
    /* Linux: Read from /proc/<pid>/comm */
    char path[64];
    FILE *f;

    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    f = fopen(path, "r");
    if (f) {
        if (fgets(name, name_len, f)) {
            /* Remove trailing newline */
            char *nl = strchr(name, '\n');
            if (nl) *nl = '\0';
        }
        fclose(f);
    } else {
        snprintf(name, name_len, "[unknown]");
    }
#endif
}

/* Find PID for a given inode from /proc/[pid]/fd/ */
static pid_t find_pid_for_inode(unsigned long inode) {
    DIR *proc_dir;
    struct dirent *proc_entry;
    char path[512], link_target[512];
    char inode_str[64];
    
    snprintf(inode_str, sizeof(inode_str), "socket:[%lu]", inode);
    
    proc_dir = opendir("/proc");
    if (!proc_dir) return 0;
    
    while ((proc_entry = readdir(proc_dir)) != NULL) {
        /* Only look at numeric directories (PIDs) */
        if (proc_entry->d_name[0] < '0' || proc_entry->d_name[0] > '9')
            continue;
            
        pid_t pid = atoi(proc_entry->d_name);
        char fd_path[128];
        DIR *fd_dir;
        struct dirent *fd_entry;
        
        snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd", pid);
        fd_dir = opendir(fd_path);
        if (!fd_dir) continue;
        
        while ((fd_entry = readdir(fd_dir)) != NULL) {
            snprintf(path, sizeof(path), "%s/%s", fd_path, fd_entry->d_name);
            ssize_t len = readlink(path, link_target, sizeof(link_target) - 1);
            if (len > 0) {
                link_target[len] = '\0';
                if (strcmp(link_target, inode_str) == 0) {
                    closedir(fd_dir);
                    closedir(proc_dir);
                    return pid;
                }
            }
        }
        closedir(fd_dir);
    }
    closedir(proc_dir);
    return 0;
}

/* TCP state names */
static const char *tcp_state_name(int state) {
    static const char *states[] = {
        "UNKNOWN",      /* 0 */
        "ESTABLISHED",  /* 1 */
        "SYN_SENT",     /* 2 */
        "SYN_RECV",     /* 3 */
        "FIN_WAIT1",    /* 4 */
        "FIN_WAIT2",    /* 5 */
        "TIME_WAIT",    /* 6 */
        "CLOSE",        /* 7 */
        "CLOSE_WAIT",   /* 8 */
        "LAST_ACK",     /* 9 */
        "LISTEN",       /* 10 (0x0A) */
        "CLOSING"       /* 11 */
    };
    if (state >= 0 && state <= 11) return states[state];
    return "UNKNOWN";
}

/* Parse /proc/net/tcp or /proc/net/tcp6 */
static int parse_tcp_file(const char *filename, network_info_t *net, int is_ipv6) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;
    
    char line[512];
    /* Skip header */
    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return -1;
    }
    
    while (fgets(line, sizeof(line), f)) {
        char local_addr_hex[64], remote_addr_hex[64];
        unsigned int local_port, remote_port;
        unsigned int state;
        unsigned long inode;
        
        /* Parse the line - format varies slightly between tcp and tcp6 */
        if (is_ipv6) {
            if (sscanf(line, "%*d: %32[0-9A-Fa-f]:%X %32[0-9A-Fa-f]:%X %X %*s %*s %*s %*d %*d %lu",
                       local_addr_hex, &local_port,
                       remote_addr_hex, &remote_port,
                       &state, &inode) != 6) continue;
        } else {
            if (sscanf(line, "%*d: %8[0-9A-Fa-f]:%X %8[0-9A-Fa-f]:%X %X %*s %*s %*s %*d %*d %lu",
                       local_addr_hex, &local_port,
                       remote_addr_hex, &remote_port,
                       &state, &inode) != 6) continue;
        }
        
        /* Is this a listener? */
        if (state == 0x0A && net->listener_count < MAX_LISTENERS) {
            net_listener_t *l = &net->listeners[net->listener_count];
            
            snprintf(l->protocol, sizeof(l->protocol), is_ipv6 ? "tcp6" : "tcp");
            hex_to_ip(local_addr_hex, l->local_addr, sizeof(l->local_addr), is_ipv6);
            l->local_port = local_port;
            snprintf(l->state, sizeof(l->state), "%s", tcp_state_name(state));
            
            /* Find owning process */
            l->pid = find_pid_for_inode(inode);
            if (l->pid > 0) {
                get_process_name(l->pid, l->process_name, sizeof(l->process_name));
            } else {
                snprintf(l->process_name, sizeof(l->process_name), "[kernel]");
            }
            
            net->listener_count++;
            net->total_listening++;
            
            if (!is_common_port(local_port)) {
                net->unusual_port_count++;
            }
        }
        /* Is this an established connection? */
        else if (state == 0x01 && net->connection_count < MAX_CONNECTIONS) {
            net_connection_t *c = &net->connections[net->connection_count];
            
            snprintf(c->protocol, sizeof(c->protocol), is_ipv6 ? "tcp6" : "tcp");
            hex_to_ip(local_addr_hex, c->local_addr, sizeof(c->local_addr), is_ipv6);
            c->local_port = local_port;
            hex_to_ip(remote_addr_hex, c->remote_addr, sizeof(c->remote_addr), is_ipv6);
            c->remote_port = remote_port;
            snprintf(c->state, sizeof(c->state), "%s", tcp_state_name(state));
            
            c->pid = find_pid_for_inode(inode);
            if (c->pid > 0) {
                get_process_name(c->pid, c->process_name, sizeof(c->process_name));
            } else {
                snprintf(c->process_name, sizeof(c->process_name), "[kernel]");
            }
            
            net->connection_count++;
            net->total_established++;
        }
    }
    
    fclose(f);
    return 0;
}

/* Parse /proc/net/udp or /proc/net/udp6 for listening UDP sockets */
static int parse_udp_file(const char *filename, network_info_t *net, int is_ipv6) {
    FILE *f = fopen(filename, "r");
    if (!f) return -1;
    
    char line[512];
    /* Skip header */
    if (!fgets(line, sizeof(line), f)) {
        fclose(f);
        return -1;
    }
    
    while (fgets(line, sizeof(line), f) && net->listener_count < MAX_LISTENERS) {
        char local_addr_hex[64], remote_addr_hex[64];
        unsigned int local_port, remote_port;
        unsigned int state;
        unsigned long inode;
        
        if (is_ipv6) {
            if (sscanf(line, "%*d: %32[0-9A-Fa-f]:%X %32[0-9A-Fa-f]:%X %X %*s %*s %*s %*d %*d %lu",
                       local_addr_hex, &local_port,
                       remote_addr_hex, &remote_port,
                       &state, &inode) != 6) continue;
        } else {
            if (sscanf(line, "%*d: %8[0-9A-Fa-f]:%X %8[0-9A-Fa-f]:%X %X %*s %*s %*s %*d %*d %lu",
                       local_addr_hex, &local_port,
                       remote_addr_hex, &remote_port,
                       &state, &inode) != 6) continue;
        }
        
        /* UDP sockets with state 07 are listening */
        if (state == 0x07 || local_port > 0) {
            net_listener_t *l = &net->listeners[net->listener_count];
            
            snprintf(l->protocol, sizeof(l->protocol), is_ipv6 ? "udp6" : "udp");
            hex_to_ip(local_addr_hex, l->local_addr, sizeof(l->local_addr), is_ipv6);
            l->local_port = local_port;
            snprintf(l->state, sizeof(l->state), "LISTEN");
            
            l->pid = find_pid_for_inode(inode);
            if (l->pid > 0) {
                get_process_name(l->pid, l->process_name, sizeof(l->process_name));
            } else {
                snprintf(l->process_name, sizeof(l->process_name), "[kernel]");
            }
            
            net->listener_count++;
            net->total_listening++;
            
            if (!is_common_port(local_port)) {
                net->unusual_port_count++;
            }
        }
    }
    
    fclose(f);
    return 0;
}

#ifdef _AIX
/* AIX doesn't have strcasestr, so implement it */
static const char *strcasestr(const char *haystack, const char *needle) {
    if (!haystack || !needle) return NULL;
    if (!*needle) return haystack;

    size_t needle_len = strlen(needle);
    for (; *haystack; haystack++) {
        if (strncasecmp(haystack, needle, needle_len) == 0) {
            return haystack;
        }
    }
    return NULL;
}

/* AIX-specific: Build a map of PIDs to listening ports using procfiles
 * This is a best-effort approach since AIX doesn't provide socket inodes like Linux */
typedef struct {
    pid_t pid;
    uint16_t port;
    char process_name[64];
} pid_port_map_t;

static pid_port_map_t pid_port_map[512];
static int pid_port_map_count = 0;

/* Well-known port to service name mappings */
typedef struct {
    uint16_t port;
    const char *service_name;
} well_known_port_t;

static const well_known_port_t well_known_ports[] = {
    /* Standard Internet Services */
    {21, "ftp"},
    {22, "sshd"},
    {23, "telnet"},
    {25, "sendmail"},
    {53, "named"},         /* DNS */
    {80, "httpd"},
    {110, "pop"},
    {111, "rpc"},          /* RPC portmapper */
    {123, "ntpd"},         /* NTP */
    {143, "imap"},
    {389, "ldap"},         /* LDAP */
    {443, "httpd"},        /* HTTPS */
    {465, "sendmail"},     /* SMTPS */
    {514, "syslog"},
    {636, "ldap"},         /* LDAPS */
    {993, "imap"},         /* IMAPS */
    {995, "pop"},          /* POP3S */

    /* Databases */
    {1433, "sqlserv"},     /* Microsoft SQL Server */
    {1521, "oracle"},      /* Oracle TNS Listener */
    {1526, "oracle"},      /* Oracle alternate */
    {1527, "oracle"},      /* Oracle TLI */
    {3050, "firebird"},    /* Firebird/InterBase */
    {3306, "mysql"},       /* MySQL/MariaDB */
    {5432, "postgres"},    /* PostgreSQL */
    {9088, "informix"},    /* Informix Dynamic Server */
    {9089, "informix"},    /* Informix Dynamic Server SSL */
    {50000, "db2"},        /* DB2 default instance */
    {60000, "db2"},        /* DB2 alternate */

    /* IBM Middleware & Products */
    {94, "objcall"},       /* Tivoli Object Dispatcher */
    {200, "src"},          /* IBM System Resource Controller */
    {385, "ibm-app"},      /* IBM Application */
    {523, "db2"},          /* IBM DB2 */
    {627, "tivoli"},       /* PassGo Tivoli */
    {729, "netview"},      /* IBM NetView DM/6000 */
    {730, "netview"},      /* IBM NetView send */
    {731, "netview"},      /* IBM NetView receive */
    {1260, "ibm-ssd"},     /* IBM SSD */
    {1352, "domino"},      /* Lotus Notes/Domino */
    {1376, "ibm-pps"},     /* IBM Person to Person */
    {1405, "ibm-res"},     /* IBM Remote Execution Starter */
    {1414, "mq"},          /* IBM MQSeries/WebSphere MQ */
    {1435, "cics"},        /* IBM CICS */
    {2809, "was"},         /* WebSphere Application Server (bootstrap) */
    {5555, "was"},         /* WebSphere App Server (SOAP connector) */
    {7276, "was"},         /* WebSphere ORB listener */
    {7286, "was"},         /* WebSphere ORB listener SSL */
    {9043, "was"},         /* WebSphere Admin Console HTTPS */
    {9060, "was"},         /* WebSphere Admin Console HTTP */
    {9080, "was"},         /* WebSphere HTTP Transport */
    {9443, "was"},         /* WebSphere HTTPS Transport */

    /* SAP */
    {3200, "sap"},         /* SAP System Gateway */
    {3300, "sap"},         /* SAP System Message Server */
    {3600, "sap"},         /* SAP System Gateway (secure) */
    {8000, "sap"},         /* SAP ICM HTTP */
    {8001, "sap"},         /* SAP ICM HTTPS */

    /* Development & Common Apps */
    {3000, "rails"},       /* Ruby on Rails */
    {3389, "rdp"},         /* Remote Desktop */
    {4000, "app"},         /* Generic application server */
    {5000, "python"},      /* Python/Flask development */
    {5001, "app"},         /* Generic application */
    {6379, "redis"},       /* Redis */
    {8000, "django"},      /* Django development */
    {8080, "httpd"},       /* HTTP alternate/Tomcat */
    {8443, "httpd"},       /* HTTPS alternate */
    {9000, "app"},         /* Generic application */
    {11211, "memcached"},  /* Memcached */
    {27017, "mongo"},      /* MongoDB */

    /* AIX-Specific Services */
    {199, "smux"},         /* SNMP multiplexer */
    {657, "rmc"},          /* Resource Monitoring and Control */
    {1334, "writesrv"},    /* AIX write daemon */
    {5987, "wbem"},        /* WBEM HTTP */
    {5988, "wbem"},        /* WBEM HTTPS */
    {32768, "aso"},        /* Audit Subsystem Object */
    {32769, "clcomd"},     /* Cluster Communication Daemon */
    {16191, "db2admin"},   /* DB2 Admin Server */

    {0, NULL}
};

/* Check if process has open sockets by examining /proc/[pid]/fd */
static int process_has_sockets(pid_t pid) {
    char fd_path[128];
    DIR *fd_dir;
    struct dirent *fd_entry;
    int has_socket = 0;

    snprintf(fd_path, sizeof(fd_path), "/proc/%d/fd", pid);
    fd_dir = opendir(fd_path);
    if (!fd_dir) return 0;

    /* Check if any FD is a socket (starts with 's' in ls -l output) */
    while ((fd_entry = readdir(fd_dir)) != NULL) {
        if (fd_entry->d_name[0] == '.') continue;

        char fd_full_path[256];
        struct stat st;
        snprintf(fd_full_path, sizeof(fd_full_path), "%s/%s", fd_path, fd_entry->d_name);

        if (stat(fd_full_path, &st) == 0) {
            /* Check if it's a socket (S_IFSOCK) */
            if (S_ISSOCK(st.st_mode)) {
                has_socket = 1;
                break;
            }
        }
    }

    closedir(fd_dir);
    return has_socket;
}

/* Build map of processes that have sockets open */
static void build_pid_port_map(void) {
    DIR *proc_dir;
    struct dirent *proc_entry;
    pid_port_map_count = 0;

    proc_dir = opendir("/proc");
    if (!proc_dir) return;

    while ((proc_entry = readdir(proc_dir)) != NULL && pid_port_map_count < 512) {
        /* Only look at numeric directories (PIDs) */
        if (proc_entry->d_name[0] < '0' || proc_entry->d_name[0] > '9')
            continue;

        pid_t pid = atoi(proc_entry->d_name);

        /* Check if this process has sockets open */
        if (!process_has_sockets(pid))
            continue;

        char proc_name[64] = "[unknown]";
        get_process_name(pid, proc_name, sizeof(proc_name));

        /* Store this PID and process name */
        pid_port_map[pid_port_map_count].pid = pid;
        pid_port_map[pid_port_map_count].port = 0; /* Port unknown at this stage */
        snprintf(pid_port_map[pid_port_map_count].process_name,
                sizeof(pid_port_map[pid_port_map_count].process_name),
                "%s", proc_name);
        pid_port_map_count++;
    }
    closedir(proc_dir);
}

/* Try to find PID for a given port using heuristics */
static pid_t find_pid_for_port(uint16_t port, char *proc_name_out, size_t proc_name_size) {
    /* Strategy: Use well-known port mappings to find likely process name,
     * then search our PID map for a process with that name */

    const char *expected_service = NULL;

    /* Look up well-known port */
    for (int i = 0; well_known_ports[i].service_name != NULL; i++) {
        if (well_known_ports[i].port == port) {
            expected_service = well_known_ports[i].service_name;
            break;
        }
    }

    if (!expected_service) {
        /* Unknown port - return 0 */
        if (proc_name_out) {
            strncpy(proc_name_out, "[unknown]", proc_name_size);
        }
        return 0;
    }

    /* Search for process with matching name */
    for (int i = 0; i < pid_port_map_count; i++) {
        /* Check if process name contains expected service name */
        if (strcasestr(pid_port_map[i].process_name, expected_service) != NULL) {
            if (proc_name_out) {
                strncpy(proc_name_out, pid_port_map[i].process_name, proc_name_size);
                proc_name_out[proc_name_size - 1] = '\0';
            }
            return pid_port_map[i].pid;
        }
    }

    /* Not found */
    if (proc_name_out) {
        strncpy(proc_name_out, "[unknown]", proc_name_size);
    }
    return 0;
}

/* AIX-specific: Parse netstat output for network connections
 * Enhanced version that attempts to correlate with process information */
static int probe_network_aix_netstat(network_info_t *net) {
    FILE *fp;
    char line[512];

    /* Build PID map (best effort) */
    build_pid_port_map();

    /* Use netstat to get TCP connections and listeners */
    fp = popen("/usr/bin/netstat -an -f inet -f inet6 | grep -E '(LISTEN|ESTABLISHED)'", "r");
    if (!fp) return -1;

    while (fgets(line, sizeof(line), fp) &&
           (net->listener_count < MAX_LISTENERS || net->connection_count < MAX_CONNECTIONS)) {
        char proto[16], local[128], remote[128], state[32];

        /* Parse netstat output: tcp4  0  0  127.0.0.1.22  *.*  LISTEN */
        if (sscanf(line, "%15s %*d %*d %127s %127s %31s", proto, local, remote, state) == 4) {
            char *port_str;
            uint16_t port;

            /* Extract port from local address (format: addr.port) */
            port_str = strrchr(local, '.');
            if (!port_str) continue;
            port = atoi(port_str + 1);
            *port_str = '\0'; /* Null-terminate address part */

            if (strncmp(state, "LISTEN", 6) == 0 && net->listener_count < MAX_LISTENERS) {
                /* Add listener */
                net_listener_t *l = &net->listeners[net->listener_count];
                snprintf(l->protocol, sizeof(l->protocol), "%s", proto);
                snprintf(l->local_addr, sizeof(l->local_addr), "%s", local);
                l->local_port = port;
                snprintf(l->state, sizeof(l->state), "LISTEN");

                /* Try to find PID using well-known port heuristics */
                l->pid = find_pid_for_port(port, l->process_name, sizeof(l->process_name));

                net->listener_count++;
                net->total_listening++;

                if (!is_common_port(port)) {
                    net->unusual_port_count++;
                }
            } else if (strncmp(state, "ESTABLISHED", 11) == 0 && net->connection_count < MAX_CONNECTIONS) {
                /* Add connection */
                char *remote_port_str = strrchr(remote, '.');
                uint16_t remote_port = 0;
                if (remote_port_str) {
                    remote_port = atoi(remote_port_str + 1);
                    *remote_port_str = '\0';
                }

                net_connection_t *c = &net->connections[net->connection_count];
                snprintf(c->protocol, sizeof(c->protocol), "%s", proto);
                snprintf(c->local_addr, sizeof(c->local_addr), "%s", local);
                c->local_port = port;
                snprintf(c->remote_addr, sizeof(c->remote_addr), "%s", remote);
                c->remote_port = remote_port;
                snprintf(c->state, sizeof(c->state), "ESTABLISHED");

                /* Try to find PID using well-known port heuristics */
                c->pid = find_pid_for_port(port, c->process_name, sizeof(c->process_name));

                net->connection_count++;
                net->total_established++;
            }
        }
    }

    pclose(fp);
    return 0;
}
#endif

/* Main network probe function */
int probe_network(network_info_t *net) {
    memset(net, 0, sizeof(network_info_t));

#ifdef _AIX
    /* AIX: Use netstat parsing as primary method
     * libperfstat doesn't provide the granular per-connection data we need */
    return probe_network_aix_netstat(net);
#else
    /* Linux: Parse /proc/net files */

    /* Probe TCP */
    parse_tcp_file("/proc/net/tcp", net, 0);
    parse_tcp_file("/proc/net/tcp6", net, 1);

    /* Probe UDP */
    parse_udp_file("/proc/net/udp", net, 0);
    parse_udp_file("/proc/net/udp6", net, 1);

    return 0;
#endif
}
