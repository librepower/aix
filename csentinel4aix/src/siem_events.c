/*
 * C-Sentinel - SIEM Events Module
 * Copyright (c) 2025 William Murray / LibrePower
 *
 * Generates security events for SIEM integration:
 * - Syslog (UDP/TCP) with CEF or JSON format
 * - Log file (JSON lines) for Wazuh/Filebeat/agents
 * - Email alerts for critical events
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>

#include "sentinel.h"

/* Event severity levels */
#define SEV_INFO     1
#define SEV_LOW      3
#define SEV_MEDIUM   5
#define SEV_HIGH     7
#define SEV_CRITICAL 9

/* Event types */
typedef enum {
    EVT_NONE = 0,
    EVT_AUTH_FAILURE,
    EVT_BRUTE_FORCE,
    EVT_PRIV_ESCALATION,
    EVT_NEW_LISTENER,
    EVT_LISTENER_GONE,
    EVT_CONFIG_CHANGE,
    EVT_PROCESS_ANOMALY,
    EVT_HIGH_RISK,
    EVT_FINGERPRINT        /* Full fingerprint (periodic) */
} event_type_t;

/* Event structure */
typedef struct {
    event_type_t type;
    int severity;
    time_t timestamp;
    char hostname[256];
    char message[512];
    char details[1024];
    int risk_score;
    /* Event-specific fields */
    char src_ip[64];
    char username[64];
    int port;
    char process_name[256];
    char file_path[512];
    int count;
} siem_event_t;

/* SIEM configuration */
typedef struct {
    int enabled;
    char syslog_host[256];
    int syslog_port;
    int syslog_proto;       /* SOCK_DGRAM (UDP) or SOCK_STREAM (TCP) */
    char syslog_format[16]; /* "cef" or "json" */
    char logfile_path[512];
    int logfile_fd;
    char alert_email[256];
    int alert_threshold;
    char smtp_host[256];
} siem_config_t;

static siem_config_t g_siem_config = {0};
static fingerprint_t g_last_fingerprint = {0};
static int g_has_last_fingerprint = 0;

/* Initialize SIEM module */
int siem_init(const char *syslog_host, int syslog_port, const char *format,
              const char *logfile, const char *alert_email, int threshold) {

    memset(&g_siem_config, 0, sizeof(g_siem_config));

    if (syslog_host && syslog_host[0]) {
        strncpy(g_siem_config.syslog_host, syslog_host, sizeof(g_siem_config.syslog_host) - 1);
        g_siem_config.syslog_port = syslog_port > 0 ? syslog_port : 514;
        g_siem_config.syslog_proto = SOCK_DGRAM; /* Default UDP */
        strncpy(g_siem_config.syslog_format, format ? format : "cef",
                sizeof(g_siem_config.syslog_format) - 1);
        g_siem_config.enabled = 1;
    }

    if (logfile && logfile[0]) {
        strncpy(g_siem_config.logfile_path, logfile, sizeof(g_siem_config.logfile_path) - 1);
        g_siem_config.logfile_fd = open(logfile, O_WRONLY | O_CREAT | O_APPEND, 0640);
        if (g_siem_config.logfile_fd < 0) {
            fprintf(stderr, "Warning: Cannot open logfile %s: %s\n", logfile, strerror(errno));
        }
        g_siem_config.enabled = 1;
    }

    if (alert_email && alert_email[0]) {
        strncpy(g_siem_config.alert_email, alert_email, sizeof(g_siem_config.alert_email) - 1);
        g_siem_config.alert_threshold = threshold > 0 ? threshold : 50;
    }

    return g_siem_config.enabled ? 0 : -1;
}

/* Cleanup */
void siem_cleanup(void) {
    if (g_siem_config.logfile_fd > 0) {
        close(g_siem_config.logfile_fd);
    }
}

/* Get current timestamp in ISO8601 */
static void get_timestamp(char *buf, size_t len) {
    time_t now = time(NULL);
    struct tm *tm = gmtime(&now);
    strftime(buf, len, "%Y-%m-%dT%H:%M:%SZ", tm);
}

/* Format event as CEF (Common Event Format) */
static int format_cef(const siem_event_t *evt, char *buf, size_t len) {
    const char *event_names[] = {
        "None", "AuthFailure", "BruteForce", "PrivEscalation",
        "NewListener", "ListenerGone", "ConfigChange",
        "ProcessAnomaly", "HighRisk", "Fingerprint"
    };

    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));

    int n = snprintf(buf, len,
        "CEF:0|LibrePower|C-Sentinel|0.6.0|%d|%s|%d|"
        "rt=%s dhost=%s msg=%s cn1Label=risk_score cn1=%d",
        evt->type,
        event_names[evt->type < 10 ? evt->type : 0],
        evt->severity,
        timestamp,
        evt->hostname,
        evt->message,
        evt->risk_score
    );

    /* Add optional fields */
    if (evt->src_ip[0]) {
        n += snprintf(buf + n, len - n, " src=%s", evt->src_ip);
    }
    if (evt->username[0]) {
        n += snprintf(buf + n, len - n, " suser=%s", evt->username);
    }
    if (evt->port > 0) {
        n += snprintf(buf + n, len - n, " dpt=%d", evt->port);
    }
    if (evt->process_name[0]) {
        n += snprintf(buf + n, len - n, " sproc=%s", evt->process_name);
    }
    if (evt->file_path[0]) {
        n += snprintf(buf + n, len - n, " filePath=%s", evt->file_path);
    }
    if (evt->count > 0) {
        n += snprintf(buf + n, len - n, " cnt=%d", evt->count);
    }

    return n;
}

/* Format event as JSON */
static int format_json(const siem_event_t *evt, char *buf, size_t len) {
    const char *event_names[] = {
        "none", "auth_failure", "brute_force", "priv_escalation",
        "new_listener", "listener_gone", "config_change",
        "process_anomaly", "high_risk", "fingerprint"
    };

    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));

    int n = snprintf(buf, len,
        "{\"timestamp\":\"%s\",\"source\":\"csentinel\",\"host\":\"%s\","
        "\"event\":\"%s\",\"severity\":%d,\"risk_score\":%d,\"message\":\"%s\"",
        timestamp,
        evt->hostname,
        event_names[evt->type < 10 ? evt->type : 0],
        evt->severity,
        evt->risk_score,
        evt->message
    );

    /* Add optional fields */
    if (evt->src_ip[0]) {
        n += snprintf(buf + n, len - n, ",\"src_ip\":\"%s\"", evt->src_ip);
    }
    if (evt->username[0]) {
        n += snprintf(buf + n, len - n, ",\"username\":\"%s\"", evt->username);
    }
    if (evt->port > 0) {
        n += snprintf(buf + n, len - n, ",\"port\":%d", evt->port);
    }
    if (evt->process_name[0]) {
        n += snprintf(buf + n, len - n, ",\"process\":\"%s\"", evt->process_name);
    }
    if (evt->file_path[0]) {
        n += snprintf(buf + n, len - n, ",\"file\":\"%s\"", evt->file_path);
    }
    if (evt->count > 0) {
        n += snprintf(buf + n, len - n, ",\"count\":%d", evt->count);
    }
    if (evt->details[0]) {
        n += snprintf(buf + n, len - n, ",\"details\":%s", evt->details);
    }

    n += snprintf(buf + n, len - n, "}");

    return n;
}

/* Send event via syslog (UDP/TCP) */
static int send_syslog(const siem_event_t *evt) {
    if (!g_siem_config.syslog_host[0]) return 0;

    char buf[4096];
    char msg[4096];

    /* Format message */
    if (strcmp(g_siem_config.syslog_format, "json") == 0) {
        format_json(evt, msg, sizeof(msg));
    } else {
        format_cef(evt, msg, sizeof(msg));
    }

    /* Add syslog header (RFC 5424 simplified) */
    /* PRI = facility(1=user) * 8 + severity */
    int pri = 8 + (10 - evt->severity); /* Map our severity to syslog */
    if (pri < 8) pri = 8;
    if (pri > 15) pri = 15;

    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));

    int len = snprintf(buf, sizeof(buf), "<%d>1 %s %s csentinel - - - %s",
                       pri, timestamp, evt->hostname, msg);

    /* Create socket and send */
    int sock = socket(AF_INET, g_siem_config.syslog_proto, 0);
    if (sock < 0) {
        fprintf(stderr, "Syslog socket error: %s\n", strerror(errno));
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(g_siem_config.syslog_port);

    struct hostent *he = gethostbyname(g_siem_config.syslog_host);
    if (!he) {
        fprintf(stderr, "Cannot resolve syslog host: %s\n", g_siem_config.syslog_host);
        close(sock);
        return -1;
    }
    memcpy(&addr.sin_addr, he->h_addr, he->h_length);

    int ret;
    if (g_siem_config.syslog_proto == SOCK_STREAM) {
        /* TCP - connect first */
        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            fprintf(stderr, "Syslog connect error: %s\n", strerror(errno));
            close(sock);
            return -1;
        }
        ret = send(sock, buf, len, 0);
    } else {
        /* UDP - sendto */
        ret = sendto(sock, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr));
    }

    close(sock);
    return ret > 0 ? 0 : -1;
}

/* Write event to log file */
static int write_logfile(const siem_event_t *evt) {
    if (g_siem_config.logfile_fd <= 0) return 0;

    char buf[4096];
    int len = format_json(evt, buf, sizeof(buf) - 1);
    buf[len++] = '\n';

    return write(g_siem_config.logfile_fd, buf, len) > 0 ? 0 : -1;
}

/* Send email alert */
static int send_email_alert(const siem_event_t *evt) {
    if (!g_siem_config.alert_email[0]) return 0;
    if (evt->risk_score < g_siem_config.alert_threshold) return 0;

    char cmd[2048];
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));

    snprintf(cmd, sizeof(cmd),
        "echo 'Subject: [C-Sentinel] %s Alert on %s\n\n"
        "Time: %s\n"
        "Host: %s\n"
        "Event: %s\n"
        "Risk Score: %d\n"
        "Details: %s\n"
        "\n--\nC-Sentinel SIEM Integration' | /usr/sbin/sendmail %s 2>/dev/null",
        evt->severity >= SEV_HIGH ? "CRITICAL" : "Warning",
        evt->hostname,
        timestamp,
        evt->hostname,
        evt->message,
        evt->risk_score,
        evt->details[0] ? evt->details : "N/A",
        g_siem_config.alert_email
    );

    return system(cmd);
}

/* Emit a single event to all configured outputs */
static void emit_event(const siem_event_t *evt) {
    send_syslog(evt);
    write_logfile(evt);
    send_email_alert(evt);
}

/* Create and emit a simple event */
void siem_emit(event_type_t type, int severity, const char *hostname,
               const char *message, int risk_score) {
    siem_event_t evt = {0};
    evt.type = type;
    evt.severity = severity;
    evt.timestamp = time(NULL);
    evt.risk_score = risk_score;
    strncpy(evt.hostname, hostname, sizeof(evt.hostname) - 1);
    strncpy(evt.message, message, sizeof(evt.message) - 1);

    emit_event(&evt);
}

/* Compare fingerprints and generate events for changes */
int siem_process_fingerprint(const fingerprint_t *fp) {
    if (!g_siem_config.enabled) return 0;

    siem_event_t evt = {0};
    strncpy(evt.hostname, fp->system.hostname, sizeof(evt.hostname) - 1);
    evt.timestamp = time(NULL);

    int events_generated = 0;

#ifdef _AIX
    /* Check AIX audit events */
    aix_audit_summary_t *audit = get_aix_audit_summary();
    if (audit && audit->enabled) {
        /* Brute force detection */
        if (audit->brute_force_detected) {
            evt.type = EVT_BRUTE_FORCE;
            evt.severity = SEV_CRITICAL;
            evt.risk_score = 90;
            evt.count = audit->auth_failures;
            strncpy(evt.message, "Brute force attack detected", sizeof(evt.message) - 1);
            strncpy(evt.username, audit->last_failed_user, sizeof(evt.username) - 1);
            emit_event(&evt);
            events_generated++;
        }
        /* Auth failures (if significant) */
        else if (audit->auth_failures > 3) {
            evt.type = EVT_AUTH_FAILURE;
            evt.severity = SEV_MEDIUM;
            evt.risk_score = 30 + audit->auth_failures * 5;
            evt.count = audit->auth_failures;
            snprintf(evt.message, sizeof(evt.message),
                     "%d authentication failures detected", audit->auth_failures);
            emit_event(&evt);
            events_generated++;
        }

        /* Privilege escalation */
        if (audit->su_success > 0 || audit->sudo_count > 0) {
            evt.type = EVT_PRIV_ESCALATION;
            evt.severity = SEV_LOW;
            evt.risk_score = 20;
            evt.count = audit->su_success + audit->sudo_count;
            snprintf(evt.message, sizeof(evt.message),
                     "Privilege escalation: %d su, %d sudo",
                     audit->su_success, audit->sudo_count);
            emit_event(&evt);
            events_generated++;
        }
    }
#endif

    /* Compare with previous fingerprint if available */
    if (g_has_last_fingerprint) {
        /* Check for new listeners */
        for (int i = 0; i < fp->network.listener_count; i++) {
            int found = 0;
            for (int j = 0; j < g_last_fingerprint.network.listener_count; j++) {
                if (fp->network.listeners[i].local_port ==
                    g_last_fingerprint.network.listeners[j].local_port) {
                    found = 1;
                    break;
                }
            }
            if (!found && fp->network.listeners[i].local_port > 0) {
                evt.type = EVT_NEW_LISTENER;
                evt.severity = SEV_HIGH;
                evt.risk_score = 70;
                evt.port = fp->network.listeners[i].local_port;
                strncpy(evt.process_name, fp->network.listeners[i].process_name,
                        sizeof(evt.process_name) - 1);
                snprintf(evt.message, sizeof(evt.message),
                         "New listener detected: port %d (%s)",
                         evt.port, evt.process_name);
                emit_event(&evt);
                events_generated++;
            }
        }

        /* Check for config changes */
        for (int i = 0; i < fp->config_count; i++) {
            for (int j = 0; j < g_last_fingerprint.config_count; j++) {
                if (strcmp(fp->configs[i].path, g_last_fingerprint.configs[j].path) == 0) {
                    if (strcmp(fp->configs[i].checksum,
                               g_last_fingerprint.configs[j].checksum) != 0) {
                        evt.type = EVT_CONFIG_CHANGE;
                        evt.severity = SEV_HIGH;
                        evt.risk_score = 60;
                        strncpy(evt.file_path, fp->configs[i].path,
                                sizeof(evt.file_path) - 1);
                        snprintf(evt.message, sizeof(evt.message),
                                 "Config file modified: %s", evt.file_path);
                        emit_event(&evt);
                        events_generated++;
                    }
                    break;
                }
            }
        }
    }

    /* Store current fingerprint for next comparison */
    memcpy(&g_last_fingerprint, fp, sizeof(fingerprint_t));
    g_has_last_fingerprint = 1;

    /* Emit periodic fingerprint event (for SIEM baseline) */
    evt.type = EVT_FINGERPRINT;
    evt.severity = SEV_INFO;
    evt.risk_score = 0;

    /* Calculate overall risk */
    quick_analysis_t analysis;
    if (analyze_fingerprint_quick(fp, &analysis) == 0) {
        evt.risk_score = analysis.total_issues * 10;
        if (evt.risk_score > 100) evt.risk_score = 100;
    }

    snprintf(evt.message, sizeof(evt.message),
             "Periodic fingerprint: %d processes, %d listeners, %d configs",
             fp->process_count, fp->network.listener_count, fp->config_count);

    /* Add full details as JSON for SIEM enrichment */
    snprintf(evt.details, sizeof(evt.details),
             "{\"processes\":%d,\"listeners\":%d,\"zombies\":%d,"
             "\"high_fd\":%d,\"memory_pct\":%.1f,\"load\":%.2f}",
             fp->process_count,
             fp->network.listener_count,
             analysis.zombie_process_count,
             analysis.high_fd_process_count,
             100.0 - (fp->system.free_ram * 100.0 / fp->system.total_ram),
             fp->system.load_avg[0]
    );

    emit_event(&evt);
    events_generated++;

    return events_generated;
}

/* Check if SIEM integration is enabled */
int siem_is_enabled(void) {
    return g_siem_config.enabled;
}

/* Get current config summary */
void siem_print_config(void) {
    fprintf(stderr, "SIEM Integration:\n");
    if (g_siem_config.syslog_host[0]) {
        fprintf(stderr, "  Syslog: %s:%d (%s format)\n",
                g_siem_config.syslog_host,
                g_siem_config.syslog_port,
                g_siem_config.syslog_format);
    }
    if (g_siem_config.logfile_path[0]) {
        fprintf(stderr, "  Logfile: %s\n", g_siem_config.logfile_path);
    }
    if (g_siem_config.alert_email[0]) {
        fprintf(stderr, "  Email alerts: %s (threshold: %d)\n",
                g_siem_config.alert_email, g_siem_config.alert_threshold);
    }
}
