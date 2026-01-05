/*
 * C-Sentinel - Semantic Observability for UNIX Systems
 * Copyright (c) 2025 William Murray
 *
 * Licensed under the MIT License.
 * See LICENSE file for details.
 *
 * https://github.com/williamofai/c-sentinel
 *
 * alert.c - Webhook alerting for critical findings
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sentinel.h"

/* Alert severity levels */
typedef enum {
    ALERT_INFO = 0,
    ALERT_WARNING = 1,
    ALERT_CRITICAL = 2
} alert_severity_t;

/* Alert structure */
typedef struct {
    alert_severity_t severity;
    char hostname[256];
    char title[256];
    char message[2048];
    time_t timestamp;
    int zombie_count;
    int unusual_ports;
    int config_changes;
    double memory_percent;
    double load_avg;
} alert_t;

/* Build alert message as JSON for webhook */
static void build_alert_json(const alert_t *alert, char *buf, size_t buf_size) {
    const char *severity_str = "info";
    const char *color = "#36a64f";  /* Green */
    
    switch (alert->severity) {
        case ALERT_WARNING:
            severity_str = "warning";
            color = "#ffa500";  /* Orange */
            break;
        case ALERT_CRITICAL:
            severity_str = "critical";
            color = "#ff0000";  /* Red */
            break;
        default:
            break;
    }
    
    /* Format timestamp */
    char time_buf[64];
    struct tm *tm = gmtime(&alert->timestamp);
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", tm);
    
    /* Build Slack-compatible webhook payload */
    snprintf(buf, buf_size,
        "{"
        "\"attachments\": [{"
            "\"color\": \"%s\","
            "\"title\": \"🛡️ C-Sentinel Alert: %s\","
            "\"text\": \"%s\","
            "\"fields\": ["
                "{\"title\": \"Hostname\", \"value\": \"%s\", \"short\": true},"
                "{\"title\": \"Severity\", \"value\": \"%s\", \"short\": true},"
                "{\"title\": \"Zombies\", \"value\": \"%d\", \"short\": true},"
                "{\"title\": \"Unusual Ports\", \"value\": \"%d\", \"short\": true},"
                "{\"title\": \"Memory\", \"value\": \"%.1f%%\", \"short\": true},"
                "{\"title\": \"Load\", \"value\": \"%.2f\", \"short\": true}"
            "],"
            "\"footer\": \"C-Sentinel\","
            "\"ts\": %ld"
        "}]"
        "}",
        color,
        alert->title,
        alert->message,
        alert->hostname,
        severity_str,
        alert->zombie_count,
        alert->unusual_ports,
        alert->memory_percent,
        alert->load_avg,
        (long)alert->timestamp
    );
}

/* Send webhook using curl (simple approach - no libcurl dependency) */
int alert_send_webhook(const char *url, const alert_t *alert) {
    if (!url || !url[0]) {
        return -1;  /* No webhook configured */
    }
    
    char json[4096];
    build_alert_json(alert, json, sizeof(json));
    
    /* Build curl command */
    char cmd[8192];
    snprintf(cmd, sizeof(cmd),
        "curl -s -X POST -H 'Content-Type: application/json' -d '%s' '%s' >/dev/null 2>&1",
        json, url);
    
    return system(cmd);
}

/* Create alert from fingerprint and analysis */
int alert_create_from_analysis(alert_t *alert, const fingerprint_t *fp, 
                                const quick_analysis_t *analysis,
                                alert_severity_t severity) {
    memset(alert, 0, sizeof(*alert));
    
    alert->severity = severity;
    alert->timestamp = time(NULL);
    snprintf(alert->hostname, sizeof(alert->hostname), "%.200s", fp->system.hostname);
    
    alert->zombie_count = analysis->zombie_process_count;
    alert->unusual_ports = analysis->unusual_listeners;
    alert->config_changes = analysis->config_permission_issues;
    alert->memory_percent = 100.0 * (1.0 - (double)fp->system.free_ram / fp->system.total_ram);
    alert->load_avg = fp->system.load_avg[0];
    
    /* Build title - truncate hostname if needed */
    char short_host[64];
    snprintf(short_host, sizeof(short_host), "%.60s", fp->system.hostname);
    
    if (severity == ALERT_CRITICAL) {
        snprintf(alert->title, sizeof(alert->title), "CRITICAL on %s", short_host);
    } else if (severity == ALERT_WARNING) {
        snprintf(alert->title, sizeof(alert->title), "Warning on %s", short_host);
    } else {
        snprintf(alert->title, sizeof(alert->title), "Info from %s", short_host);
    }
    
    /* Build message */
    char *p = alert->message;
    size_t remaining = sizeof(alert->message);
    int written;
    
    written = snprintf(p, remaining, "Issues detected:\\n");
    p += written; remaining -= written;
    
    if (analysis->zombie_process_count > 0) {
        written = snprintf(p, remaining, "• %d zombie process(es)\\n", 
                          analysis->zombie_process_count);
        p += written; remaining -= written;
    }
    
    if (analysis->unusual_listeners > 0) {
        written = snprintf(p, remaining, "• %d unusual listening port(s)\\n",
                          analysis->unusual_listeners);
        p += written; remaining -= written;
    }
    
    if (analysis->config_permission_issues > 0) {
        written = snprintf(p, remaining, "• %d config permission issue(s)\\n",
                          analysis->config_permission_issues);
        p += written; remaining -= written;
    }
    
    if (analysis->high_fd_process_count > 5) {
        written = snprintf(p, remaining, "• %d high FD process(es)\\n",
                          analysis->high_fd_process_count);
        p += written; remaining -= written;
    }
    
    return 0;
}

/* Print alert to console */
void alert_print(const alert_t *alert) {
    const char *severity_str = "INFO";
    const char *icon = "ℹ️";
    
    switch (alert->severity) {
        case ALERT_WARNING:
            severity_str = "WARNING";
            icon = "⚠️";
            break;
        case ALERT_CRITICAL:
            severity_str = "CRITICAL";
            icon = "🚨";
            break;
        default:
            break;
    }
    
    printf("\n");
    printf("%s %s ALERT: %s\n", icon, severity_str, alert->title);
    printf("────────────────────────────────────────────────\n");
    printf("Host: %s\n", alert->hostname);
    printf("Time: %s", ctime(&alert->timestamp));
    printf("\n");
    
    /* Print message with newlines converted */
    printf("Details:\n");
    char *msg = strdup(alert->message);
    char *line = strtok(msg, "\\n");
    while (line) {
        if (line[0]) printf("  %s\n", line);
        line = strtok(NULL, "\\n");
    }
    free(msg);
    
    printf("\nMetrics:\n");
    printf("  Zombies: %d\n", alert->zombie_count);
    printf("  Unusual ports: %d\n", alert->unusual_ports);
    printf("  Memory: %.1f%%\n", alert->memory_percent);
    printf("  Load: %.2f\n", alert->load_avg);
}

/* Check if alert should be sent based on severity */
int alert_should_send(alert_severity_t severity, int on_critical, int on_warning) {
    if (severity == ALERT_CRITICAL && on_critical) return 1;
    if (severity == ALERT_WARNING && on_warning) return 1;
    return 0;
}
