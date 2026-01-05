/*
 * C-Sentinel - Semantic Observability for UNIX Systems
 * Copyright (c) 2025 William Murray
 *
 * Licensed under the MIT License.
 * See LICENSE file for details.
 *
 * https://github.com/williamofai/c-sentinel
 *
 * color.h - ANSI colour output support
 */

#ifndef SENTINEL_COLOR_H
#define SENTINEL_COLOR_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* ANSI colour codes */
#define COL_RESET   "\033[0m"
#define COL_BOLD    "\033[1m"
#define COL_DIM     "\033[2m"

#define COL_RED     "\033[31m"
#define COL_GREEN   "\033[32m"
#define COL_YELLOW  "\033[33m"
#define COL_BLUE    "\033[34m"
#define COL_MAGENTA "\033[35m"
#define COL_CYAN    "\033[36m"
#define COL_WHITE   "\033[37m"

#define COL_BRED    "\033[1;31m"
#define COL_BGREEN  "\033[1;32m"
#define COL_BYELLOW "\033[1;33m"
#define COL_BBLUE   "\033[1;34m"
#define COL_BCYAN   "\033[1;36m"

/* Global colour state */
static int g_color_enabled = 0;

/* Check if colours should be enabled (respects NO_COLOR env var) */
static inline int color_should_enable(void) {
    /* Respect NO_COLOR standard: https://no-color.org/ */
    if (getenv("NO_COLOR") != NULL) {
        return 0;
    }
    /* Only enable if stdout is a TTY */
    return isatty(STDOUT_FILENO);
}

/* Initialize colour output */
static inline void color_init(int force_color) {
    if (force_color == 1) {
        g_color_enabled = 1;
    } else if (force_color == -1) {
        g_color_enabled = 0;
    } else {
        g_color_enabled = color_should_enable();
    }
}

/* Colour output helpers */
static inline const char* col(const char *code) {
    return g_color_enabled ? code : "";
}

/* Semantic colour helpers */
static inline const char* col_ok(void)       { return col(COL_GREEN); }
static inline const char* col_warn(void)     { return col(COL_YELLOW); }
static inline const char* col_error(void)    { return col(COL_RED); }
static inline const char* col_critical(void) { return col(COL_BRED); }
static inline const char* col_info(void)     { return col(COL_CYAN); }
static inline const char* col_header(void)   { return col(COL_BOLD); }
static inline const char* col_dim(void)      { return col(COL_DIM); }
static inline const char* col_reset(void)    { return col(COL_RESET); }

/* Risk level colour */
static inline const char* col_risk(const char *level) {
    if (!g_color_enabled) return "";
    if (strcmp(level, "low") == 0) return COL_GREEN;
    if (strcmp(level, "medium") == 0) return COL_YELLOW;
    if (strcmp(level, "high") == 0) return COL_RED;
    if (strcmp(level, "critical") == 0) return COL_BRED;
    return "";
}

/* Print coloured status indicators */
static inline void print_status_ok(const char *msg) {
    printf("%s✓%s %s\n", col_ok(), col_reset(), msg);
}

static inline void print_status_warn(const char *msg) {
    printf("%s⚠%s %s\n", col_warn(), col_reset(), msg);
}

static inline void print_status_error(const char *msg) {
    printf("%s✗%s %s\n", col_error(), col_reset(), msg);
}

#endif /* SENTINEL_COLOR_H */
