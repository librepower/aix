/*
 * C-Sentinel - Semantic Observability for UNIX Systems
 * Copyright (c) 2025 William Murray
 *
 * Licensed under the MIT License.
 * See LICENSE file for details.
 *
 * https://github.com/williamofai/c-sentinel
 *
 * audit.h - Auditd integration for C-Sentinel
 * 
 * Captures and summarises security-relevant audit events
 * for semantic analysis by LLMs.
 */

#ifndef SENTINEL_AUDIT_H
#define SENTINEL_AUDIT_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>  /* For pid_t */

/* Include sentinel.h for MAX_PATH_LEN */
#include "sentinel.h"

/* Limits */
#define MAX_AUDIT_USERS         32
#define MAX_AUDIT_FILES         32
#define MAX_AUDIT_ANOMALIES     16
#define MAX_PROCESS_CHAIN       8
#define MAX_SUSPICIOUS_PROCS    16
#define MAX_RISK_FACTORS        16
#define HASH_USERNAME_LEN       12      /* "user_xxxx" + null */
#define AUDIT_PATH_LEN          256     /* Shorter paths for audit */
#define RISK_FACTOR_REASON_LEN  128

/* Hashed username for privacy */
typedef struct {
    char hash[HASH_USERNAME_LEN];       /* e.g., "user_8f3d" */
    int  count;                         /* Number of events for this user */
} hashed_user_t;

/* Process chain for ancestry tracking */
typedef struct {
    char names[MAX_PROCESS_CHAIN][64];  /* Process names in chain */
    int  depth;                         /* How many levels */
} process_chain_t;

/* Sensitive file access record */
typedef struct {
    char path[AUDIT_PATH_LEN];
    char access_type[8];                /* "read", "write", "exec" */
    int  count;
    char process[64];                   /* Process that accessed it */
    process_chain_t chain;              /* Full process ancestry */
    bool suspicious;
} file_access_t;

/* Suspicious process execution */
typedef struct {
    char path[AUDIT_PATH_LEN];          /* What was executed */
    char parent[64];                    /* Parent process */
    process_chain_t chain;              /* Full ancestry */
    bool from_tmp;                      /* Executed from /tmp? */
    bool from_devshm;                   /* Executed from /dev/shm? */
    char description[128];              /* Why it's suspicious */
} suspicious_exec_t;

/* Anomaly record */
typedef struct {
    char type[32];                      /* e.g., "auth_failure_spike" */
    char description[128];
    char severity[12];                  /* "low", "medium", "high", "critical" */
    float current_value;
    float baseline_avg;
    float deviation_pct;
    time_t timestamp;
} audit_anomaly_t;

/* Risk factor - explains why the score is what it is */
typedef struct {
    char reason[RISK_FACTOR_REASON_LEN];
    int  weight;                         /* Points added to score */
} risk_factor_t;

/* Main audit summary structure */
typedef struct {
    /* Metadata */
    bool enabled;
    int  period_seconds;
    time_t capture_time;
    
    /* Authentication */
    int  auth_failures;
    int  auth_successes;                /* We track but don't output */
    hashed_user_t failure_users[MAX_AUDIT_USERS];
    int  failure_user_count;
    int  failure_sources;               /* Unique source addresses */
    float auth_baseline_avg;
    float auth_deviation_pct;
    bool brute_force_detected;
    
    /* Privilege escalation */
    int  sudo_count;
    float sudo_baseline_avg;
    float sudo_deviation_pct;
    int  su_count;
    int  setuid_executions;
    int  capability_changes;
    
    /* File integrity */
    int  permission_changes;
    int  ownership_changes;
    file_access_t sensitive_files[MAX_AUDIT_FILES];
    int  sensitive_file_count;
    
    /* Process activity */
    suspicious_exec_t suspicious_execs[MAX_SUSPICIOUS_PROCS];
    int  suspicious_exec_count;
    int  tmp_executions;
    int  devshm_executions;
    int  shell_spawns;
    int  cron_executions;
    
    /* Security framework */
    bool selinux_enforcing;
    int  selinux_avc_denials;
    int  apparmor_denials;
    
    /* Anomalies */
    audit_anomaly_t anomalies[MAX_AUDIT_ANOMALIES];
    int  anomaly_count;
    
    /* Risk assessment */
    int  risk_score;
    char risk_level[12];                /* "low", "medium", "high", "critical" */
    
    /* Risk factors - explains the score */
    risk_factor_t risk_factors[MAX_RISK_FACTORS];
    int  risk_factor_count;
    
    /* Baseline learning status */
    int  baseline_sample_count;         /* How many samples in baseline */
} audit_summary_t;

/* Rolling baseline for audit metrics (stored to disk) */
typedef struct {
    char magic[8];                      /* "SNTLAUDT" */
    uint32_t version;
    time_t created;
    time_t updated;
    uint32_t sample_count;
    
    /* Exponential moving averages */
    float avg_auth_failures;
    float avg_sudo_count;
    float avg_sensitive_access;
    float avg_tmp_executions;
    float avg_shell_spawns;
} audit_baseline_t;

/* ============================================================
 * Function Prototypes
 * ============================================================ */

/* Main probe function */
audit_summary_t* probe_audit(int window_seconds);

/* Cleanup */
void free_audit_summary(audit_summary_t *summary);

/* JSON output */
void audit_to_json(const audit_summary_t *summary, char *buf, size_t bufsize);

/* Baseline management */
bool load_audit_baseline(audit_baseline_t *baseline);
bool save_audit_baseline(const audit_baseline_t *baseline);
void update_audit_baseline(audit_baseline_t *baseline, const audit_summary_t *current);

/* Deviation calculation */
float calculate_deviation_pct(float current, float baseline_avg);
const char* deviation_significance(float deviation_pct);

/* Process chain utilities */
void build_process_chain(pid_t pid, process_chain_t *chain);
bool is_suspicious_chain(const process_chain_t *chain, const char **description);
void format_process_chain(const process_chain_t *chain, char *buf, size_t bufsize);

/* Username hashing (privacy) */
void hash_username(const char *username, char *output, size_t outsize);

/* Risk scoring */
void calculate_risk_score(audit_summary_t *summary);

#endif /* SENTINEL_AUDIT_H */
