/*
 * C-Sentinel - Semantic Observability for UNIX Systems
 * Copyright (c) 2025 William Murray
 *
 * Licensed under the MIT License.
 * See LICENSE file for details.
 *
 * https://github.com/williamofai/c-sentinel
 *
 * audit_json.c - JSON serialization for audit summary
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../include/audit.h"

/* Helper to append to buffer safely */
static int buf_append(char *buf, size_t bufsize, size_t *pos, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(buf + *pos, bufsize - *pos, fmt, args);
    va_end(args);
    if (written > 0 && *pos + written < bufsize) {
        *pos += written;
        return written;
    }
    return 0;
}


/*
 * Output audit summary as JSON
 */
void audit_to_json(const audit_summary_t *summary, char *buf, size_t bufsize) {
    size_t pos = 0;
    
    buf_append(buf, bufsize, &pos, "  \"audit_summary\": {\n");
    buf_append(buf, bufsize, &pos, "    \"enabled\": %s,\n", summary->enabled ? "true" : "false");
    buf_append(buf, bufsize, &pos, "    \"period_seconds\": %d,\n", summary->period_seconds);
    
    if (!summary->enabled) {
        buf_append(buf, bufsize, &pos, "    \"error\": \"auditd not available or not readable\"\n");
        buf_append(buf, bufsize, &pos, "  }");
        return;
    }
    
    /* Authentication section */
    buf_append(buf, bufsize, &pos, "    \"authentication\": {\n");
    buf_append(buf, bufsize, &pos, "      \"failures\": %d,\n", summary->auth_failures);
    
    /* Hashed usernames */
    buf_append(buf, bufsize, &pos, "      \"failure_users_hashed\": [");
    for (int i = 0; i < summary->failure_user_count; i++) {
        buf_append(buf, bufsize, &pos, "%s\"%s\"", 
                  i > 0 ? ", " : "", summary->failure_users[i].hash);
    }
    buf_append(buf, bufsize, &pos, "],\n");
    
    buf_append(buf, bufsize, &pos, "      \"baseline_avg\": %.2f,\n", summary->auth_baseline_avg);
    buf_append(buf, bufsize, &pos, "      \"deviation_pct\": %.1f,\n", summary->auth_deviation_pct);
    buf_append(buf, bufsize, &pos, "      \"brute_force_detected\": %s\n", 
              summary->brute_force_detected ? "true" : "false");
    buf_append(buf, bufsize, &pos, "    },\n");
    
    /* Privilege escalation section */
    buf_append(buf, bufsize, &pos, "    \"privilege_escalation\": {\n");
    buf_append(buf, bufsize, &pos, "      \"sudo_count\": %d,\n", summary->sudo_count);
    buf_append(buf, bufsize, &pos, "      \"sudo_baseline_avg\": %.2f,\n", summary->sudo_baseline_avg);
    buf_append(buf, bufsize, &pos, "      \"sudo_deviation_pct\": %.1f,\n", summary->sudo_deviation_pct);
    buf_append(buf, bufsize, &pos, "      \"su_count\": %d,\n", summary->su_count);
    buf_append(buf, bufsize, &pos, "      \"setuid_executions\": %d,\n", summary->setuid_executions);
    buf_append(buf, bufsize, &pos, "      \"capability_changes\": %d\n", summary->capability_changes);
    buf_append(buf, bufsize, &pos, "    },\n");
    
    /* File integrity section */
    buf_append(buf, bufsize, &pos, "    \"file_integrity\": {\n");
    buf_append(buf, bufsize, &pos, "      \"permission_changes\": %d,\n", summary->permission_changes);
    buf_append(buf, bufsize, &pos, "      \"ownership_changes\": %d,\n", summary->ownership_changes);
    buf_append(buf, bufsize, &pos, "      \"sensitive_file_access\": [\n");
    
    for (int i = 0; i < summary->sensitive_file_count; i++) {
        const file_access_t *fa = &summary->sensitive_files[i];
        buf_append(buf, bufsize, &pos, "        {\n");
        buf_append(buf, bufsize, &pos, "          \"path\": \"%s\",\n", fa->path);
        buf_append(buf, bufsize, &pos, "          \"access\": \"%s\",\n", fa->access_type);
        buf_append(buf, bufsize, &pos, "          \"count\": %d,\n", fa->count);
        buf_append(buf, bufsize, &pos, "          \"process\": \"%s\",\n", fa->process);
        
        /* Process chain array */
        buf_append(buf, bufsize, &pos, "          \"process_chain\": [");
        for (int j = 0; j < fa->chain.depth; j++) {
            buf_append(buf, bufsize, &pos, "%s\"%s\"", 
                      j > 0 ? ", " : "", fa->chain.names[j]);
        }
        buf_append(buf, bufsize, &pos, "],\n");
        
        buf_append(buf, bufsize, &pos, "          \"suspicious\": %s\n", fa->suspicious ? "true" : "false");
        buf_append(buf, bufsize, &pos, "        }%s\n", 
                  i < summary->sensitive_file_count - 1 ? "," : "");
    }
    
    buf_append(buf, bufsize, &pos, "      ]\n");
    buf_append(buf, bufsize, &pos, "    },\n");
    
    /* Process activity section */
    buf_append(buf, bufsize, &pos, "    \"process_activity\": {\n");
    buf_append(buf, bufsize, &pos, "      \"tmp_executions\": %d,\n", summary->tmp_executions);
    buf_append(buf, bufsize, &pos, "      \"devshm_executions\": %d,\n", summary->devshm_executions);
    buf_append(buf, bufsize, &pos, "      \"shell_spawns\": %d,\n", summary->shell_spawns);
    buf_append(buf, bufsize, &pos, "      \"cron_executions\": %d,\n", summary->cron_executions);
    buf_append(buf, bufsize, &pos, "      \"suspicious_exec_count\": %d\n", summary->suspicious_exec_count);
    buf_append(buf, bufsize, &pos, "    },\n");
    
    /* Security framework section */
    buf_append(buf, bufsize, &pos, "    \"security_framework\": {\n");
    buf_append(buf, bufsize, &pos, "      \"selinux_enforcing\": %s,\n", 
              summary->selinux_enforcing ? "true" : "false");
    buf_append(buf, bufsize, &pos, "      \"selinux_avc_denials\": %d,\n", summary->selinux_avc_denials);
    buf_append(buf, bufsize, &pos, "      \"apparmor_denials\": %d\n", summary->apparmor_denials);
    buf_append(buf, bufsize, &pos, "    },\n");
    
    /* Anomalies section */
    buf_append(buf, bufsize, &pos, "    \"anomalies\": [\n");
    for (int i = 0; i < summary->anomaly_count; i++) {
        const audit_anomaly_t *a = &summary->anomalies[i];
        buf_append(buf, bufsize, &pos, "      {\n");
        buf_append(buf, bufsize, &pos, "        \"type\": \"%s\",\n", a->type);
        buf_append(buf, bufsize, &pos, "        \"description\": \"%s\",\n", a->description);
        buf_append(buf, bufsize, &pos, "        \"severity\": \"%s\",\n", a->severity);
        buf_append(buf, bufsize, &pos, "        \"current\": %.1f,\n", a->current_value);
        buf_append(buf, bufsize, &pos, "        \"baseline_avg\": %.2f,\n", a->baseline_avg);
        buf_append(buf, bufsize, &pos, "        \"deviation_pct\": %.1f\n", a->deviation_pct);
        buf_append(buf, bufsize, &pos, "      }%s\n",
                  i < summary->anomaly_count - 1 ? "," : "");
    }
    buf_append(buf, bufsize, &pos, "    ],\n");
    
    /* Learning/confidence status */
    buf_append(buf, bufsize, &pos, "    \"learning\": {\n");
    buf_append(buf, bufsize, &pos, "      \"sample_count\": %d,\n", summary->baseline_sample_count);
    buf_append(buf, bufsize, &pos, "      \"confidence\": \"%s\"\n", 
              summary->baseline_sample_count < 5 ? "low" :
              summary->baseline_sample_count < 20 ? "medium" : "high");
    buf_append(buf, bufsize, &pos, "    },\n");
    
    /* Risk factors section */
    buf_append(buf, bufsize, &pos, "    \"risk_factors\": [\n");
    for (int i = 0; i < summary->risk_factor_count; i++) {
        const risk_factor_t *rf = &summary->risk_factors[i];
        buf_append(buf, bufsize, &pos, "      {\n");
        buf_append(buf, bufsize, &pos, "        \"reason\": \"%s\",\n", rf->reason);
        buf_append(buf, bufsize, &pos, "        \"weight\": %d\n", rf->weight);
        buf_append(buf, bufsize, &pos, "      }%s\n",
                  i < summary->risk_factor_count - 1 ? "," : "");
    }
    buf_append(buf, bufsize, &pos, "    ],\n");
    
    /* Risk assessment */
    buf_append(buf, bufsize, &pos, "    \"risk_score\": %d,\n", summary->risk_score);
    buf_append(buf, bufsize, &pos, "    \"risk_level\": \"%s\"\n", summary->risk_level);
    
    buf_append(buf, bufsize, &pos, "  }");
}
