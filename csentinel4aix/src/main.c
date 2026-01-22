/*
 * C-Sentinel - Semantic Observability for UNIX Systems
 * Copyright (c) 2025 William Murray
 *
 * Licensed under the MIT License.
 * See LICENSE file for details.
 *
 * https://github.com/williamofai/c-sentinel
 *
 * main.c - CLI entry point
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifndef _AIX
#include <getopt.h>
#endif
#include <signal.h>
#include <time.h>

#include "sentinel.h"
#ifndef _AIX
#include "audit.h"
#endif
#include "color.h"

#ifdef _AIX
/* AIX audit summary - from aix_audit.c */
extern int probe_aix_audit(aix_audit_summary_t *summary, time_t since);
extern int aix_audit_to_json(const aix_audit_summary_t *summary, char *buf, size_t buf_size);
#endif

/* Default config files to probe if none specified */
#ifdef _AIX
/* AIX: Use minimal default, full list available via get_aix_critical_files() */
static const char *default_configs[] = {
    "/etc/passwd",
    "/etc/group",
    "/etc/hosts",
    "/etc/ssh/sshd_config",
    "/etc/resolv.conf",
    "/etc/security/passwd",
    "/etc/security/user",
    "/etc/security/login.cfg",
    "/etc/security/audit/config",
    "/etc/inittab",
    "/etc/inetd.conf",
    "/etc/sudoers",
    NULL
};
#else
static const char *default_configs[] = {
    "/etc/hosts",
    "/etc/passwd",
    "/etc/ssh/sshd_config",
    "/etc/fstab",
    "/etc/resolv.conf",
    NULL
};
#endif

/* Global flag for clean shutdown in watch mode */
static volatile int keep_running = 1;

#ifdef _AIX
/* AIX audit summary for JSON output integration */
static aix_audit_summary_t g_aix_audit;
#else
/* Global audit summary for JSON output integration */
static audit_summary_t *g_audit_summary = NULL;
#endif

static void signal_handler(int signum) {
    (void)signum;
    keep_running = 0;
    fprintf(stderr, "\nShutting down...\n");
}

static void print_usage(const char *prog) {
    fprintf(stderr, "C-Sentinel v%s - Semantic Observability for UNIX Systems\n\n", SENTINEL_VERSION);
    fprintf(stderr, "Usage: %s [OPTIONS] [config_files...]\n\n", prog);
    fprintf(stderr, "Options:\n");
#ifdef _AIX
    /* AIX: Show short options only (AIX getopt doesn't support long options) */
    fprintf(stderr, "  -h          Show this help message\n");
    fprintf(stderr, "  -q          Only show quick analysis summary\n");
    fprintf(stderr, "  -v          Include all processes (not just notable ones)\n");
    fprintf(stderr, "  -j          Output JSON to stdout (even in quick mode)\n");
    fprintf(stderr, "  -w          Continuous monitoring mode\n");
    fprintf(stderr, "  -i SEC      Interval between probes in watch mode (default: 60)\n");
    fprintf(stderr, "  -n          Include network probe (listeners, connections)\n");
    fprintf(stderr, "  -a          Include security events (AIX audit - requires: audit start)\n");
    fprintf(stderr, "  -F          Full AIX file integrity check (~150 critical files)\n");
    fprintf(stderr, "  -b          Compare against learned baseline\n");
    fprintf(stderr, "  -l          Learn current state as baseline\n");
    fprintf(stderr, "  -c          Show current configuration\n");
    fprintf(stderr, "  -C          Create default config file\n");
    fprintf(stderr, "  -A          Learn audit baseline (Linux only)\n");
    fprintf(stderr, "  -K          Force coloured output\n");
    fprintf(stderr, "  -N          Disable coloured output\n");
    fprintf(stderr, "\nSIEM Integration:\n");
    fprintf(stderr, "  -S HOST:PORT  Send events via syslog (UDP) to SIEM\n");
    fprintf(stderr, "  -R FORMAT     Syslog format: cef (default) or json\n");
    fprintf(stderr, "  -L FILE       Write events to log file (JSON lines)\n");
    fprintf(stderr, "  -M EMAIL      Send email alerts for critical events\n");
    fprintf(stderr, "  -T SCORE      Alert threshold (1-100, default: 50)\n");
#else
    fprintf(stderr, "  -h, --help           Show this help message\n");
    fprintf(stderr, "  -q, --quick          Only show quick analysis summary\n");
    fprintf(stderr, "  -v, --verbose        Include all processes (not just notable ones)\n");
    fprintf(stderr, "  -j, --json           Output JSON to stdout (even in quick mode)\n");
    fprintf(stderr, "  -w, --watch          Continuous monitoring mode\n");
    fprintf(stderr, "  -i, --interval SEC   Interval between probes in watch mode (default: 60)\n");
    fprintf(stderr, "  -n, --network        Include network probe (listeners, connections)\n");
    fprintf(stderr, "  -a, --audit          Include auditd security events\n");
    fprintf(stderr, "  -b, --baseline       Compare against learned baseline\n");
    fprintf(stderr, "  -l, --learn          Learn current state as baseline\n");
    fprintf(stderr, "  -c, --config         Show current configuration\n");
    fprintf(stderr, "      --init-config    Create default config file\n");
    fprintf(stderr, "      --audit-learn    Learn audit baseline\n");
    fprintf(stderr, "      --color          Force coloured output\n");
    fprintf(stderr, "      --no-color       Disable coloured output\n");
#endif
    fprintf(stderr, "\n");
    fprintf(stderr, "Exit codes:\n");
    fprintf(stderr, "  0 - No issues detected\n");
    fprintf(stderr, "  1 - Warnings (minor issues)\n");
    fprintf(stderr, "  2 - Critical (zombies, permission issues, unusual ports, security events)\n");
    fprintf(stderr, "  3 - Error (probe failed)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "If no config files are specified, probes common system configs.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Baseline:\n");
#ifdef _AIX
    fprintf(stderr, "  First, learn what's normal:    %s -l -n\n", prog);
    fprintf(stderr, "  Then compare against baseline: %s -b -n\n", prog);
#else
    fprintf(stderr, "  First, learn what's normal:    %s --learn --network\n", prog);
    fprintf(stderr, "  Then compare against baseline: %s --baseline --network\n", prog);
#endif
    fprintf(stderr, "\n");
    fprintf(stderr, "Audit:\n");
#ifdef _AIX
    fprintf(stderr, "  Include security events:       %s -q -a\n", prog);
    fprintf(stderr, "  Learn audit baseline:          %s -A\n", prog);
    fprintf(stderr, "  Full analysis with audit:      %s -j -n -a\n", prog);
#else
    fprintf(stderr, "  Include security events:       %s --quick --audit\n", prog);
    fprintf(stderr, "  Learn audit baseline:          %s --audit-learn\n", prog);
    fprintf(stderr, "  Full analysis with audit:      %s --json --network --audit\n", prog);
#endif
    fprintf(stderr, "\n");
    fprintf(stderr, "Environment:\n");
    fprintf(stderr, "  NO_COLOR             Disable coloured output (standard)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Config file: ~/.sentinel/config\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Examples:\n");
#ifdef _AIX
    fprintf(stderr, "  %s -q                    One-shot quick analysis\n", prog);
    fprintf(stderr, "  %s -q -n                 Include network probe\n", prog);
    fprintf(stderr, "  %s -q -n -a              Include network + security events\n", prog);
    fprintf(stderr, "  %s -w -i 300             Monitor every 5 minutes\n", prog);
    fprintf(stderr, "  %s -j > fingerprint.json Save full JSON output\n", prog);
    fprintf(stderr, "  %s -l -n                 Learn current state as baseline\n", prog);
    fprintf(stderr, "  %s -b -n                 Compare against baseline\n", prog);
    fprintf(stderr, "\nSIEM Examples:\n");
    fprintf(stderr, "  %s -w -i 60 -n -a -S 10.0.0.50:514      Syslog to QRadar (CEF)\n", prog);
    fprintf(stderr, "  %s -w -i 60 -n -a -S 10.0.0.50:514 -R json    Syslog JSON format\n", prog);
    fprintf(stderr, "  %s -w -i 60 -n -a -L /var/log/sentinel.log   Log file for Wazuh\n", prog);
    fprintf(stderr, "  %s -w -i 60 -n -a -M admin@x.com -T 70       Email on high risk\n", prog);
#else
    fprintf(stderr, "  %s --quick                    One-shot quick analysis\n", prog);
    fprintf(stderr, "  %s --quick --network          Include network probe\n", prog);
    fprintf(stderr, "  %s --quick --network --audit  Include network + security events\n", prog);
    fprintf(stderr, "  %s --watch --interval 300     Monitor every 5 minutes\n", prog);
    fprintf(stderr, "  %s --json > fingerprint.json  Save full JSON output\n", prog);
    fprintf(stderr, "  %s --learn --network          Learn current state as baseline\n", prog);
    fprintf(stderr, "  %s --baseline --network       Compare against baseline\n", prog);
#endif
}

static void print_timestamp(void) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);
    printf("[%s] ", buf);
}

#ifndef _AIX
static void print_audit_summary_quick(const audit_summary_t *audit) {
    if (!audit || !audit->enabled) {
        printf("\n%sAudit:%s unavailable (auditd not running or not readable)\n",
               col_header(), col_reset());
        return;
    }

    printf("\n%sSecurity (audit):%s\n", col_header(), col_reset());
    printf("  Auth failures: %s%d%s",
           audit->auth_failures > 0 ? col_warn() : col_ok(),
           audit->auth_failures,
           col_reset());
    if (audit->auth_deviation_pct > 100.0f) {
        printf(" %s(%.0f%% above baseline) ⚠%s", col_warn(), audit->auth_deviation_pct, col_reset());
    }
    printf("\n");

    if (audit->brute_force_detected) {
        printf("  %s⚠ BRUTE FORCE PATTERN DETECTED%s\n", col_critical(), col_reset());
    }

    printf("  Sudo commands: %d", audit->sudo_count);
    if (audit->sudo_deviation_pct > 200.0f) {
        printf(" %s(%.0f%% above baseline) ⚠%s", col_warn(), audit->sudo_deviation_pct, col_reset());
    }
    printf("\n");

    if (audit->sensitive_file_count > 0) {
        printf("  Sensitive file access: %s%d%s\n", col_warn(), audit->sensitive_file_count, col_reset());
        for (int i = 0; i < audit->sensitive_file_count && i < 5; i++) {
            printf("    - %s by %s%s%s\n",
                   audit->sensitive_files[i].path,
                   audit->sensitive_files[i].process,
                   audit->sensitive_files[i].suspicious ? col_warn() : "",
                   audit->sensitive_files[i].suspicious ? " ⚠" : "");
            if (audit->sensitive_files[i].suspicious) printf("%s", col_reset());
        }
    }

    if (audit->tmp_executions > 0) {
        printf("  %s⚠ Executions from /tmp: %d%s\n", col_critical(), audit->tmp_executions, col_reset());
    }
    if (audit->devshm_executions > 0) {
        printf("  %s⚠ Executions from /dev/shm: %d%s\n", col_critical(), audit->devshm_executions, col_reset());
    }

    if (audit->selinux_avc_denials > 0) {
        printf("  SELinux denials: %s%d%s\n", col_warn(), audit->selinux_avc_denials, col_reset());
    }
    if (audit->apparmor_denials > 0) {
        printf("  AppArmor denials: %s%d%s\n", col_warn(), audit->apparmor_denials, col_reset());
    }

    /* Show anomalies */
    if (audit->anomaly_count > 0) {
        printf("\n  %sAnomalies detected:%s\n", col_warn(), col_reset());
        for (int i = 0; i < audit->anomaly_count; i++) {
            printf("    [%s] %s\n",
                   audit->anomalies[i].severity,
                   audit->anomalies[i].description);
        }
    }

    printf("\n  Risk: %s (score: %d)\n", audit->risk_level, audit->risk_score);
}
#endif /* !_AIX */

static int run_analysis(const char **configs, int config_count, 
                        int quick_mode, int json_mode, int network_mode, int audit_mode) {
    fingerprint_t fp;
    int result = capture_fingerprint(&fp, configs, config_count);
    
    if (result != 0) {
        fprintf(stderr, "Warning: Some probes failed (errors: %d)\n", fp.probe_errors);
    }
    
    /* Probe network if requested */
    if (network_mode) {
        probe_network(&fp.network);
    }
    
    /* Probe audit if requested */
#ifdef _AIX
    aix_audit_summary_t *aix_audit = NULL;
    if (audit_mode) {
        memset(&g_aix_audit, 0, sizeof(g_aix_audit));
        time_t since = time(NULL) - 300;  /* Last 5 minutes */
        probe_aix_audit(&g_aix_audit, since);
        aix_audit = &g_aix_audit;
    }
#else
    audit_summary_t *audit = NULL;
    if (audit_mode) {
        audit = probe_audit(300);  /* Last 5 minutes */
        g_audit_summary = audit;

        /* Auto-update baseline on each probe */
        if (audit && audit->enabled) {
            audit_baseline_t baseline = {0};
            load_audit_baseline(&baseline);
            update_audit_baseline(&baseline, audit);
            save_audit_baseline(&baseline);
            /* Update sample count in summary for JSON output */
            audit->baseline_sample_count = baseline.sample_count;
        }
    }
#endif
    
    /* Always do quick analysis for exit code calculation */
    quick_analysis_t analysis;
    analyze_fingerprint_quick(&fp, &analysis);
    
    if (json_mode) {
        /* Full JSON output */
        char *json = fingerprint_to_json(&fp);
        if (!json) {
            fprintf(stderr, "Error: Failed to serialize fingerprint to JSON\n");
#ifndef _AIX
            if (audit) free_audit_summary(audit);
#endif
            return EXIT_ERROR;
        }

#ifdef _AIX
        /* AIX: Inject AIX audit JSON if audit mode is on */
        if (audit_mode && aix_audit) {
            char *last_brace = strrchr(json, '}');
            if (last_brace && last_brace > json) {
                char audit_json[8192];
                if (aix_audit->enabled) {
                    aix_audit_to_json(aix_audit, audit_json, sizeof(audit_json));
                } else {
                    /* Audit not enabled - show instructions */
                    snprintf(audit_json, sizeof(audit_json),
                        "  \"audit_summary\": {\n"
                        "    \"enabled\": false,\n"
                        "    \"platform\": \"AIX\",\n"
                        "    \"message\": \"AIX audit subsystem not enabled\",\n"
                        "    \"enable_instructions\": \"/usr/sbin/audit start\"\n"
                        "  }");
                }
                *last_brace = '\0';
                printf("%s,\n%s\n}\n", json, audit_json);
            } else {
                printf("%s", json);
            }
        } else {
            printf("%s", json);
        }
#else
        /* Linux: If audit mode, we need to inject audit JSON before the closing brace */
        if (audit && audit->enabled) {
            /* Find the last closing brace */
            char *last_brace = strrchr(json, '}');
            if (last_brace && last_brace > json) {
                /* Build combined output */
                char audit_json[16384];
                audit_to_json(audit, audit_json, sizeof(audit_json));

                /* Print everything before the last }, then audit, then } */
                *last_brace = '\0';
                printf("%s,\n%s\n}\n", json, audit_json);
            } else {
                printf("%s", json);
            }
        } else {
            printf("%s", json);
        }
#endif
        free(json);
    } else if (quick_mode) {
        /* Quick analysis only */
        printf("%sC-Sentinel Quick Analysis%s\n", col_header(), col_reset());
        printf("========================\n");
        printf("Hostname: %s%s%s\n", col_info(), fp.system.hostname, col_reset());
        printf("Uptime: %.1f days\n", fp.system.uptime_seconds / 86400.0);
        printf("Load: %.2f %.2f %.2f\n", 
               fp.system.load_avg[0], fp.system.load_avg[1], fp.system.load_avg[2]);
        
        double mem_pct = 100.0 * (1.0 - (double)fp.system.free_ram / fp.system.total_ram);
        printf("Memory: %s%.1f%%%s used\n", 
               mem_pct > 90 ? col_error() : mem_pct > 75 ? col_warn() : col_ok(),
               mem_pct, col_reset());
        printf("Processes: %d total\n", fp.process_count);
        
        printf("\n%sPotential Issues:%s\n", col_header(), col_reset());
        printf("  Zombie processes: %s%d%s%s\n", 
               analysis.zombie_process_count > 0 ? col_error() : col_ok(),
               analysis.zombie_process_count, col_reset(),
               analysis.zombie_process_count > 0 ? " ⚠" : "");
        printf("  High FD processes: %s%d%s%s\n", 
               analysis.high_fd_process_count > 5 ? col_warn() : col_ok(),
               analysis.high_fd_process_count, col_reset(),
               analysis.high_fd_process_count > 5 ? " ⚠" : "");
        printf("  Long-running (>7d): %d\n", analysis.long_running_process_count);
        printf("  Config permission issues: %s%d%s%s\n", 
               analysis.config_permission_issues > 0 ? col_error() : col_ok(),
               analysis.config_permission_issues, col_reset(),
               analysis.config_permission_issues > 0 ? " ⚠" : "");
        
        if (network_mode) {
            printf("\n%sNetwork:%s\n", col_header(), col_reset());
            printf("  Listening ports: %d\n", fp.network.total_listening);
            printf("  Established connections: %d\n", fp.network.total_established);
            printf("  Unusual ports: %s%d%s%s\n", 
                   analysis.unusual_listeners > 0 ? col_warn() : col_ok(),
                   analysis.unusual_listeners, col_reset(),
                   analysis.unusual_listeners > 0 ? " ⚠" : "");
            
            /* Show listeners if any */
            if (fp.network.listener_count > 0) {
                printf("\n  Listeners:\n");
                for (int i = 0; i < fp.network.listener_count && i < 10; i++) {
                    net_listener_t *l = &fp.network.listeners[i];
                    printf("    %s%s:%d%s (%s) - %s\n", 
                           col_dim(), l->local_addr, l->local_port, col_reset(),
                           l->protocol, l->process_name);
                }
                if (fp.network.listener_count > 10) {
                    printf("    %s... and %d more%s\n", col_dim(), fp.network.listener_count - 10, col_reset());
                }
            }
        }
        
        /* Audit summary */
        if (audit_mode) {
#ifdef _AIX
            /* AIX audit quick summary */
            if (aix_audit && aix_audit->enabled) {
                printf("\n%sSecurity (AIX audit):%s\n", col_header(), col_reset());
                printf("  Auth failures: %s%d%s\n",
                       aix_audit->auth_failures > 0 ? col_warn() : col_ok(),
                       aix_audit->auth_failures, col_reset());
                if (aix_audit->brute_force_detected) {
                    printf("  %sBRUTE FORCE DETECTED%s\n", col_critical(), col_reset());
                }
                printf("  su success: %d, failures: %d\n", aix_audit->su_success, aix_audit->su_failures);
                printf("  sudo commands: %d\n", aix_audit->sudo_count);
                printf("  Sensitive reads: %d, writes: %d\n", aix_audit->sensitive_reads, aix_audit->sensitive_writes);
                printf("\n  Risk: %s (score: %d)\n", aix_audit->risk_level, aix_audit->risk_score);
            } else {
                printf("\n%sSecurity (AIX audit):%s\n", col_header(), col_reset());
                printf("  %sAudit not enabled%s\n", col_warn(), col_reset());
                printf("  To enable: /usr/sbin/audit start\n");
            }
#else
            print_audit_summary_quick(audit);
#endif
        }
    } else {
        /* Full JSON output (default) */
        char *json = fingerprint_to_json(&fp);
        if (!json) {
            fprintf(stderr, "Error: Failed to serialize fingerprint to JSON\n");
#ifndef _AIX
            if (audit) free_audit_summary(audit);
#endif
            return EXIT_ERROR;
        }

#ifdef _AIX
        /* AIX: Inject AIX audit JSON if audit mode is on */
        if (audit_mode && aix_audit) {
            char *last_brace = strrchr(json, '}');
            if (last_brace && last_brace > json) {
                char audit_json[8192];
                if (aix_audit->enabled) {
                    aix_audit_to_json(aix_audit, audit_json, sizeof(audit_json));
                } else {
                    snprintf(audit_json, sizeof(audit_json),
                        "  \"audit_summary\": {\n"
                        "    \"enabled\": false,\n"
                        "    \"platform\": \"AIX\",\n"
                        "    \"message\": \"AIX audit subsystem not enabled\",\n"
                        "    \"enable_instructions\": \"/usr/sbin/audit start\"\n"
                        "  }");
                }
                *last_brace = '\0';
                printf("%s,\n%s\n}\n", json, audit_json);
            } else {
                printf("%s", json);
            }
        } else {
            printf("%s", json);
        }
#else
        /* Same audit injection logic */
        if (audit && audit->enabled) {
            char *last_brace = strrchr(json, '}');
            if (last_brace && last_brace > json) {
                char audit_json[16384];
                audit_to_json(audit, audit_json, sizeof(audit_json));
                *last_brace = '\0';
                printf("%s,\n%s\n}\n", json, audit_json);
            } else {
                printf("%s", json);
            }
        } else {
            printf("%s", json);
        }
#endif
        free(json);
    }
    
    /* Calculate exit code based on issues */
    int exit_code = EXIT_OK;
    
    if (analysis.zombie_process_count > 0 || 
        analysis.config_permission_issues > 0 ||
        analysis.unusual_listeners > 3) {
        exit_code = EXIT_CRITICAL;
    } else if (analysis.high_fd_process_count > 5 ||
               analysis.unusual_listeners > 0) {
        exit_code = EXIT_WARNINGS;
    }
    
    /* Audit can also trigger critical */
#ifdef _AIX
    if (aix_audit && aix_audit->enabled) {
        if (aix_audit->risk_score >= 70) {  /* critical */
            exit_code = EXIT_CRITICAL;
        } else if (aix_audit->risk_score >= 20 && exit_code < EXIT_WARNINGS) {
            exit_code = EXIT_WARNINGS;
        }
    }

    /* Process SIEM events if enabled */
    if (siem_is_enabled()) {
        siem_process_fingerprint(&fp);
    }
#else
    if (audit && audit->enabled) {
        if (audit->risk_score >= 16) {  /* high or critical */
            exit_code = EXIT_CRITICAL;
        } else if (audit->risk_score >= 6 && exit_code < EXIT_WARNINGS) {
            exit_code = EXIT_WARNINGS;
        }
    }

    if (audit) {
        free_audit_summary(audit);
        g_audit_summary = NULL;
    }
#endif

    return exit_code;
}

int main(int argc, char *argv[]) {
    int quick_mode = 0;
    int json_mode = 0;
    int watch_mode = 0;
    int network_mode = 0;
    int audit_mode = 0;
    int audit_learn = 0;
    int baseline_mode = 0;
    int learn_mode = 0;
    int show_config = 0;
    int init_config = 0;
    int full_mode = 0;    /* AIX: use full critical files list */
    int interval = 60;
    int force_color = 0;  /* 0=auto, 1=force on, -1=force off */
    int opt;

    /* SIEM integration options */
    char siem_syslog[256] = {0};
    char siem_format[16] = "cef";
    char siem_logfile[512] = {0};
    char siem_email[256] = {0};
    int siem_threshold = 50;

#ifndef _AIX
    /* Long options only supported on systems with getopt_long */
    static struct option long_options[] = {
        {"help",        no_argument,       0, 'h'},
        {"quick",       no_argument,       0, 'q'},
        {"verbose",     no_argument,       0, 'v'},
        {"json",        no_argument,       0, 'j'},
        {"watch",       no_argument,       0, 'w'},
        {"interval",    required_argument, 0, 'i'},
        {"network",     no_argument,       0, 'n'},
        {"audit",       no_argument,       0, 'a'},
        {"baseline",    no_argument,       0, 'b'},
        {"learn",       no_argument,       0, 'l'},
        {"config",      no_argument,       0, 'c'},
        {"init-config", no_argument,       0, 'C'},
        {"audit-learn", no_argument,       0, 'A'},
        {"color",       no_argument,       0, 'K'},
        {"colour",      no_argument,       0, 'K'},
        {"no-color",    no_argument,       0, 'N'},
        {"no-colour",   no_argument,       0, 'N'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "hqvjwi:nablcCAKN", long_options, NULL)) != -1) {
#else
    /* AIX: Use basic getopt (short options only) */
    /* SIEM options: S=syslog, R=format, L=logfile, M=mail, T=threshold */
    while ((opt = getopt(argc, argv, "hqvjwi:nablcCAFKNS:R:L:M:T:")) != -1) {
#endif
        switch (opt) {
            case 'h':
                print_usage(argv[0]);
                return EXIT_OK;
            case 'q':
                quick_mode = 1;
                break;
            case 'v':
                /* verbose mode - future enhancement */
                break;
            case 'j':
                json_mode = 1;
                break;
            case 'w':
                watch_mode = 1;
                break;
            case 'i':
                interval = atoi(optarg);
                if (interval < 1) interval = 1;
                if (interval > 86400) interval = 86400;
                break;
            case 'n':
                network_mode = 1;
                break;
            case 'a':
                audit_mode = 1;
                break;
            case 'b':
                baseline_mode = 1;
                break;
            case 'l':
                learn_mode = 1;
                break;
            case 'c':
                show_config = 1;
                break;
            case 'C':
                init_config = 1;
                break;
            case 'A':
                audit_learn = 1;
                break;
            case 'K':
                force_color = 1;
                break;
            case 'N':
                force_color = -1;
                break;
            case 'F':
#ifdef _AIX
                full_mode = 1;
#else
                fprintf(stderr, "Warning: -F (full mode) only available on AIX\n");
#endif
                break;
            case 'S':
                /* SIEM syslog server (host:port) */
                strncpy(siem_syslog, optarg, sizeof(siem_syslog) - 1);
                break;
            case 'R':
                /* SIEM syslog format (cef or json) */
                strncpy(siem_format, optarg, sizeof(siem_format) - 1);
                break;
            case 'L':
                /* SIEM log file path */
                strncpy(siem_logfile, optarg, sizeof(siem_logfile) - 1);
                break;
            case 'M':
                /* SIEM email alerts */
                strncpy(siem_email, optarg, sizeof(siem_email) - 1);
                break;
            case 'T':
                /* SIEM alert threshold */
                siem_threshold = atoi(optarg);
                if (siem_threshold < 1) siem_threshold = 1;
                if (siem_threshold > 100) siem_threshold = 100;
                break;
            default:
                print_usage(argv[0]);
                return EXIT_ERROR;
        }
    }
    
    /* Initialize colour output */
    color_init(force_color);

#ifdef _AIX
    /* Initialize SIEM integration if any SIEM options specified */
    if (siem_syslog[0] || siem_logfile[0] || siem_email[0]) {
        char syslog_host[256] = {0};
        int syslog_port = 514;

        /* Parse syslog option (host:port) */
        if (siem_syslog[0]) {
            char *colon = strchr(siem_syslog, ':');
            if (colon) {
                *colon = '\0';
                strncpy(syslog_host, siem_syslog, sizeof(syslog_host) - 1);
                syslog_port = atoi(colon + 1);
                if (syslog_port <= 0 || syslog_port > 65535) syslog_port = 514;
            } else {
                strncpy(syslog_host, siem_syslog, sizeof(syslog_host) - 1);
            }
        }

        siem_init(syslog_host, syslog_port, siem_format,
                  siem_logfile, siem_email, siem_threshold);

        if (siem_is_enabled()) {
            siem_print_config();
            fprintf(stderr, "\n");
        }
    }
#endif
    
    /* Handle --init-config */
    if (init_config) {
        if (config_create_default() == 0) {
            printf("Created default config file: ~/.sentinel/config\n");
            return EXIT_OK;
        } else {
            fprintf(stderr, "Failed to create config file\n");
            return EXIT_ERROR;
        }
    }
    
    /* Handle --config */
    if (show_config) {
        config_print();
        return EXIT_OK;
    }
    
    /* Handle --audit-learn */
    if (audit_learn) {
#ifdef _AIX
        fprintf(stderr, "Audit baseline learning not yet implemented for AIX.\n");
        fprintf(stderr, "AIX audit uses direct trail analysis without baseline.\n");
        return EXIT_OK;
#else
        printf("Learning audit baseline...\n");
        audit_summary_t *audit = probe_audit(300);
        if (!audit || !audit->enabled) {
            fprintf(stderr, "Auditd not available. Install and configure auditd first.\n");
            if (audit) free_audit_summary(audit);
            return EXIT_ERROR;
        }

        audit_baseline_t baseline = {0};
        load_audit_baseline(&baseline);  /* Load existing if any */
        update_audit_baseline(&baseline, audit);

        if (save_audit_baseline(&baseline)) {
            printf("Audit baseline saved.\n");
            printf("  Samples: %u\n", baseline.sample_count);
            printf("  Avg auth failures: %.2f\n", baseline.avg_auth_failures);
            printf("  Avg sudo commands: %.2f\n", baseline.avg_sudo_count);
            printf("  Avg sensitive file access: %.2f\n", baseline.avg_sensitive_access);
            free_audit_summary(audit);
            return EXIT_OK;
        } else {
            fprintf(stderr, "Failed to save audit baseline\n");
            free_audit_summary(audit);
            return EXIT_ERROR;
        }
#endif
    }
    
    /* Determine config files to probe */
    const char **configs;
    int config_count;

#ifdef _AIX
    /* AIX: Static buffer for full critical files list */
    static const char *aix_full_files[MAX_CONFIG_FILES];
#endif

    if (optind < argc) {
        /* Use command line specified configs */
        configs = (const char **)&argv[optind];
        config_count = argc - optind;
    }
#ifdef _AIX
    else if (full_mode) {
        /* AIX full mode: use comprehensive critical files list */
        config_count = get_aix_critical_files(aix_full_files, MAX_CONFIG_FILES);
        configs = aix_full_files;
        fprintf(stderr, "Full AIX file integrity mode: checking %d critical files\n", config_count);
    }
#endif
    else {
        /* Use defaults */
        configs = default_configs;
        config_count = 0;
        while (default_configs[config_count]) config_count++;
    }
    
    /* Handle --learn */
    if (learn_mode) {
        fingerprint_t fp;
        capture_fingerprint(&fp, configs, config_count);
        if (network_mode) {
            probe_network(&fp.network);
        }
        
        /* Load existing baseline or create new */
        static baseline_t baseline;
        
        if (baseline_load(&baseline) != 0) {
            baseline_init(&baseline);
            printf("Creating new baseline...\n");
        } else {
            printf("Updating existing baseline...\n");
        }
        
        baseline_learn(&baseline, &fp);
        
        if (baseline_save(&baseline) == 0) {
            printf("Baseline saved to ~/.sentinel/baseline.dat\n");
            baseline_print_info(&baseline);
            return EXIT_OK;
        } else {
            fprintf(stderr, "Failed to save baseline\n");
            return EXIT_ERROR;
        }
    }
    
    /* Handle --baseline */
    if (baseline_mode) {
        static baseline_t baseline;
        
        if (baseline_load(&baseline) != 0) {
            fprintf(stderr, "No baseline found. Run with --learn first.\n");
            return EXIT_ERROR;
        }
        
        fingerprint_t fp;
        capture_fingerprint(&fp, configs, config_count);
        if (network_mode) {
            probe_network(&fp.network);
        }
        
        /* Run quick analysis */
        quick_analysis_t analysis;
        analyze_fingerprint_quick(&fp, &analysis);
        
        /* Show quick summary */
        printf("%sC-Sentinel Quick Analysis%s\n", col_header(), col_reset());
        printf("========================\n");
        printf("Hostname: %s%s%s\n", col_info(), fp.system.hostname, col_reset());
        printf("Uptime: %.1f days\n", fp.system.uptime_seconds / 86400.0);
        printf("Load: %.2f %.2f %.2f\n", 
               fp.system.load_avg[0], fp.system.load_avg[1], fp.system.load_avg[2]);
        printf("Processes: %d total\n", fp.process_count);
        
        /* Compare against baseline */
        deviation_report_t report;
        
        int deviations = baseline_compare(&baseline, &fp, &report);
        baseline_print_report(&baseline, &report);
        
        /* Also show audit if requested */
        if (audit_mode) {
#ifdef _AIX
            aix_audit_summary_t aix_bl_audit;
            time_t since = time(NULL) - 300;
            probe_aix_audit(&aix_bl_audit, since);
            if (aix_bl_audit.enabled) {
                printf("\n%sSecurity (AIX audit):%s\n", col_header(), col_reset());
                printf("  Auth failures: %d\n", aix_bl_audit.auth_failures);
                printf("  Risk: %s (score: %d)\n", aix_bl_audit.risk_level, aix_bl_audit.risk_score);
            } else {
                printf("\n%sSecurity (AIX audit):%s Not enabled\n", col_header(), col_reset());
            }
#else
            audit_summary_t *audit = probe_audit(300);
            print_audit_summary_quick(audit);
            if (audit) free_audit_summary(audit);
#endif
        }
        
        if (deviations > 0) {
            return EXIT_CRITICAL;
        }
        return EXIT_OK;
    }
    
    /* Watch mode - continuous monitoring */
    if (watch_mode) {
        /* Setup signal handler for clean shutdown */
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        
        fprintf(stderr, "C-Sentinel v%s - Watch Mode (Ctrl+C to stop)\n", SENTINEL_VERSION);
        fprintf(stderr, "Interval: %d seconds\n", interval);
        if (audit_mode) {
            fprintf(stderr, "Audit: enabled\n");
        }
        fprintf(stderr, "\n");
        
        int worst_exit = EXIT_OK;
        
        while (keep_running) {
            print_timestamp();
            int exit_code = run_analysis(configs, config_count, 
                                         quick_mode || 1, json_mode, network_mode, audit_mode);
            
            if (exit_code > worst_exit) worst_exit = exit_code;
            
            if (exit_code == EXIT_CRITICAL) {
                printf(" [CRITICAL]\n");
            } else if (exit_code == EXIT_WARNINGS) {
                printf(" [WARNINGS]\n");
            } else {
                printf(" [OK]\n");
            }
            
            if (keep_running) {
                sleep(interval);
            }
        }

#ifdef _AIX
        /* Cleanup SIEM resources */
        if (siem_is_enabled()) {
            siem_cleanup();
        }
#endif

        return worst_exit;
    }
    
    /* One-shot mode */
    return run_analysis(configs, config_count, quick_mode, json_mode, network_mode, audit_mode);
}
