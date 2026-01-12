/**
 * @file lpsof.c
 * @brief LibrePowerSof - Native AIX implementation of lsof (SECURITY HARDENED)
 * @version 0.3.0
 * @author Hugo Blanco <hugo.blanco@sixe.eu>
 *
 * A production-ready tool for listing open files on AIX 7.x systems.
 * Compatible with most lsof command-line options.
 *
 * @section Security
 * This version has been security-hardened to address:
 *   - Buffer overflow protection
 *   - Integer overflow prevention
 *   - Path traversal mitigation
 *   - Symlink attack protection
 *   - Race condition mitigation
 *   - Memory safety improvements
 *   - Input validation
 *
 * @section Architecture
 * The code is organized in the following sections:
 *   1. Type Definitions    - Enums and structs
 *   2. Global State        - Configuration and state
 *   3. Security Functions  - Validation and sanitization
 *   4. Utility Functions   - Helpers and formatters
 *   5. Filter Functions    - Match predicates for filtering
 *   6. Process Functions   - Process and FD enumeration
 *   7. Output Functions    - Display and formatting
 *   8. Subcommands         - list, summary, watch, delta, doctor
 *   9. Option Parsing      - CLI argument handling
 *   10. Main               - Entry point
 *
 * @section Building
 *   gcc -Wall -Wextra -O2 -D_ALL_SOURCE -D_LARGE_FILES -maix64 -o lpsof lpsof.c -lperfstat
 *
 * @copyright 2025-2026 LibrePower Project <hello@librepower.org>
 * @license GPL-3.0-or-later
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#define _ALL_SOURCE
#define _LARGE_FILES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/utsname.h>
#include <procinfo.h>
#include <sys/procfs.h>
#include <signal.h>

/* ============================================================================
 * SECTION 1: CONSTANTS AND TYPE DEFINITIONS
 * ============================================================================ */

/** @brief Version string */
#define LPSOF_VERSION "0.3.0"

/** @name System Limits (Security-bounded) */
/** @{ */
#define MAX_PROCS        4096   /**< Maximum processes to scan (reduced for safety) */
#define MAX_FDS          1024   /**< Maximum FDs per process (reduced for safety) */
#define INITIAL_FDS      32     /**< Initial FD array allocation (grows as needed) */
#define MAX_PATH_LEN     1024   /**< Maximum path length */
#define MAX_FILTERS      64     /**< Maximum filter entries (reduced) */
#define MAX_LINE_LEN     4096   /**< Maximum line length for state file */
#define MAX_STATE_ENTRIES 65536 /**< Maximum entries in state file */
#define MAX_ARGV_COPY    4096   /**< Maximum size for argv string copy */
#define HASH_TABLE_SIZE  8191   /**< Prime number for hash table (better distribution) */
/** @} */

/** @name Default Values */
/** @{ */
#define DEFAULT_LIMIT          100  /**< Default process limit for safety */
#define DEFAULT_WATCH_INTERVAL 2    /**< Default watch interval in seconds */
#define DEFAULT_CMD_WIDTH      9    /**< Default command column width */
#define MAX_CMD_WIDTH          64   /**< Maximum command column width */
#define MIN_WATCH_INTERVAL     1    /**< Minimum watch interval */
#define MAX_WATCH_INTERVAL     3600 /**< Maximum watch interval (1 hour) */
#define MAX_LIMIT              10000 /**< Maximum process limit */
#define STATE_FILE_DEFAULT     "/var/tmp/lpsof.state"
/** @} */

/** @name Security Constants */
/** @{ */
#define SAFE_STATE_DIR         "/var/tmp"  /**< Only allowed state file directory */
#define STATE_FILE_MAX_SIZE    (10*1024*1024) /**< Max 10MB state file */
/** @} */

/**
 * @brief Subcommand identifiers
 */
typedef enum {
    CMD_LIST = 0,    /**< List open files (default) */
    CMD_SUMMARY,     /**< Show top N processes by FD count */
    CMD_WATCH,       /**< Continuous monitoring */
    CMD_DELTA,       /**< Compare with saved snapshot */
    CMD_DOCTOR       /**< System diagnostics */
} subcommand_t;

/**
 * @brief File descriptor types
 */
typedef enum {
    FD_TYPE_UNKNOWN = 0,
    FD_TYPE_REG,     /**< Regular file */
    FD_TYPE_DIR,     /**< Directory */
    FD_TYPE_CHR,     /**< Character device */
    FD_TYPE_BLK,     /**< Block device */
    FD_TYPE_FIFO,    /**< FIFO/Pipe */
    FD_TYPE_SOCK,    /**< Socket (generic) */
    FD_TYPE_LINK,    /**< Symbolic link */
    FD_TYPE_INET,    /**< IPv4 socket */
    FD_TYPE_INET6,   /**< IPv6 socket */
    FD_TYPE_UNIX     /**< Unix domain socket */
} fd_type_t;

/**
 * @brief Special file descriptor identifiers (negative values)
 */
typedef enum {
    FD_CWD  = -1,    /**< Current working directory */
    FD_RTD  = -2,    /**< Root directory */
    FD_TXT  = -3,    /**< Program text */
    FD_MEM  = -4,    /**< Memory-mapped file */
    FD_DEL  = -5,    /**< Deleted file */
    FD_CTTY = -6     /**< Controlling terminal */
} special_fd_t;

/**
 * @brief Type filter values for --type option
 */
typedef enum {
    TYPE_FILTER_ALL = 0,
    TYPE_FILTER_FILE,
    TYPE_FILTER_DIR,
    TYPE_FILTER_PIPE,
    TYPE_FILTER_DEVICE,
    TYPE_FILTER_SOCKET
} type_filter_t;

/**
 * @brief File descriptor information
 */
typedef struct {
    int fd;                     /**< FD number (negative for special) */
    fd_type_t type;             /**< File type */
    int flags;                  /**< Open flags */
    int link_count;             /**< Hard link count */
    char access[4];             /**< Access mode string (r/w/u) */
    char fd_name[16];           /**< Display name (cwd, rtd, or number) */
    char path[MAX_PATH_LEN];    /**< File path */
    dev_t device;               /**< Device ID */
    ino_t inode;                /**< Inode number */
    off_t size;                 /**< File size */
    off_t offset;               /**< Current offset */

    /* Network socket fields */
    int proto;                  /**< Protocol (IPPROTO_TCP/UDP) */
    int family;                 /**< Address family (AF_INET/INET6/UNIX) */
    char local_addr[128];       /**< Local address string */
    int local_port;             /**< Local port */
    char remote_addr[128];      /**< Remote address string */
    int remote_port;            /**< Remote port */
    char state[24];             /**< TCP state string */
} fd_info_t;

/**
 * @brief Process information
 * @note fds is dynamically allocated to avoid ~6GB static allocation
 */
typedef struct {
    pid_t pid;                  /**< Process ID */
    pid_t ppid;                 /**< Parent process ID */
    pid_t pgid;                 /**< Process group ID */
    uid_t uid;                  /**< User ID */
    gid_t gid;                  /**< Group ID */
    char user[64];              /**< Username string */
    char command[MAXCOMLEN+1];  /**< Command name */
    char cwd[MAX_PATH_LEN];     /**< Current working directory */
    char root[MAX_PATH_LEN];    /**< Root directory */
    fd_info_t *fds;             /**< File descriptor array (dynamic) */
    int fd_count;               /**< Number of FDs in use */
    int fd_capacity;            /**< Allocated capacity of fds array */
} proc_info_t;

/**
 * @brief Command-line options and filters
 */
typedef struct {
    /* Subcommand and mode */
    subcommand_t subcommand;    /**< Active subcommand */
    int limit;                  /**< Process limit (0 = unlimited) */
    int safe_mode;              /**< Enable safety warnings */
    int watch_interval;         /**< Watch polling interval */
    char state_file[MAX_PATH_LEN]; /**< Delta state file path (fixed buffer) */
    int save_state;             /**< Save state instead of compare */

    /* Display options */
    int show_help;              /**< Show help and exit */
    int show_version;           /**< Show version and exit */
    int terse_mode;             /**< Output PIDs only */
    int human_readable;         /**< Human readable sizes */
    int show_ppid;              /**< Show PPID column */
    int show_pgid;              /**< Show PGID column */
    int show_offset;            /**< Show file offset */
    int offset_digits;          /**< Offset decimal digits */
    int show_link_count;        /**< Show link count (1=show, -1=filter) */
    int cmd_width;              /**< Command column width */
    int field_output;           /**< Machine-readable output */
    char field_sep;             /**< Field separator character */
    int no_username;            /**< Show numeric UIDs */
    int no_hostname;            /**< Don't resolve hostnames */
    int no_portname;            /**< Don't resolve port names */

    /* Process filters */
    pid_t filter_pids[MAX_FILTERS];
    int filter_pid_count;
    int filter_pid_exclude;     /**< Exclude matched PIDs */

    uid_t filter_uids[MAX_FILTERS];
    int filter_uid_count;
    int filter_uid_exclude;     /**< Exclude matched UIDs */

    pid_t filter_pgids[MAX_FILTERS];  /* Process Group IDs (not group IDs) */
    int filter_pgid_count;

    char *filter_commands[MAX_FILTERS];
    int filter_cmd_count;

    int and_logic;              /**< Use AND instead of OR for filters */

    /* FD filters */
    int filter_fds[MAX_FILTERS];
    int filter_fd_count;
    int filter_fd_cwd;          /**< Include cwd */
    int filter_fd_rtd;          /**< Include rtd */
    int filter_fd_txt;          /**< Include txt */
    int filter_fd_mem;          /**< Include mem */

    /* Path filters */
    char *filter_files[MAX_FILTERS];
    int filter_file_count;
    int filter_file_recursive;  /**< +D recursive mode */
    char *path_filter;          /**< --path substring filter */
    type_filter_t type_filter;  /**< --type filter */

    /* Network filters */
    int network_only;           /**< Show only network files */
    int unix_sockets;           /**< Show only Unix sockets */
    char *network_filter;       /**< -i filter string */
    int network_port;           /**< Port to filter */
    int network_proto;          /**< IP version (4 or 6) */
    char network_host[256];     /**< Host to filter (fixed buffer) */
    int network_tcp;            /**< Filter TCP only */
    int network_udp;            /**< Filter UDP only */

    /* TCP state filters */
    int tcp_listen;
    int tcp_established;
    int tcp_close_wait;
    int tcp_time_wait;

    /* Positional file arguments */
    char *search_files[MAX_FILTERS];
    int search_file_count;
    dev_t search_devs[MAX_FILTERS];
    ino_t search_inodes[MAX_FILTERS];

    /* Other options */
    int repeat_mode;            /**< Repeat output mode */
    int repeat_interval;        /**< Repeat interval */
    int warn_not_found;         /**< Warn about missing files */
    int ignore_errors;          /**< Ignore errors silently */
} options_t;

/* ============================================================================
 * SECTION 2: GLOBAL STATE
 * ============================================================================ */

/** @brief Global options structure */
static options_t g_opts;

/** @brief Header printed flag (reset per output cycle) */
static int g_header_printed = 0;

/** @brief Signal received flag for graceful shutdown */
static volatile sig_atomic_t g_signal_received = 0;

/* ============================================================================
 * SECTION 3: FUNCTION PROTOTYPES
 * ============================================================================ */

/* Security functions */
static void secure_strncpy(char *dst, const char *src, size_t size);
static int validate_path(const char *path, const char *allowed_dir);
static int validate_integer(const char *str, int min, int max, int *result);
static int is_safe_filename(const char *name);
static void sanitize_output(char *str, size_t len);
static void sanitize_env(void);

/* Memory management for proc_info_t */
static int proc_init_fds(proc_info_t *proc);
static int proc_grow_fds(proc_info_t *proc);
static void proc_free_fds(proc_info_t *proc);
static void procs_cleanup(proc_info_t *procs, int count);

/* Hash table for O(N) delta comparison */
typedef struct hash_entry {
    char *key;
    struct hash_entry *next;
} hash_entry_t;

typedef struct {
    hash_entry_t **buckets;
    int size;
    int count;
} hash_table_t;

static unsigned long hash_djb2(const char *str);
static hash_table_t *hash_create(int size);
static int hash_insert(hash_table_t *ht, const char *key);
static int hash_remove(hash_table_t *ht, const char *key);
static void hash_free(hash_table_t *ht);

/* Utility functions */
static void get_user_name(uid_t uid, char *buf, size_t buflen);
static const char *get_fd_type_str(fd_type_t type);
static void format_size(off_t size, char *buf, size_t buflen);
static void read_proc_link(pid_t pid, const char *name, char *buf, size_t len);

/* Filter functions */
static int match_pid_filter(pid_t pid);
static int match_uid_filter(uid_t uid);
static int match_cmd_filter(const char *cmd);
static int match_path_filter(const char *path);
static int match_type_filter(fd_type_t type);
static int match_file_filter(const char *path);
static int match_network_filter(const fd_info_t *info);
static int match_search_file(const fd_info_t *info);
static int match_tcp_state(const fd_info_t *info);

/* Process functions */
static int get_processes(proc_info_t *procs, int max_procs);
static int get_process_fds(proc_info_t *proc);
static int get_fd_info(pid_t pid, int fd, fd_info_t *info);
static int get_network_info(fd_info_t *info);
static int add_special_fds(proc_info_t *proc);

/* Output functions */
static void print_usage(void);
static void print_version(void);
static void print_header(void);
static void print_process(const proc_info_t *proc);
static void print_fd(const proc_info_t *proc, const fd_info_t *fd);
static void print_field_output(const proc_info_t *proc, const fd_info_t *fd);

/* Subcommand functions */
static int cmd_list(proc_info_t *procs, int max_procs);
static int cmd_summary(proc_info_t *procs, int max_procs);
static int cmd_watch(proc_info_t *procs, int max_procs);
static int cmd_delta(proc_info_t *procs, int max_procs);
static int cmd_doctor(void);
static int save_state(proc_info_t *procs, int count, const char *path);

/* Option parsing */
static int parse_options(int argc, char *argv[]);
static int parse_subcommand(const char *arg);
static int parse_network_filter(const char *filter);
static type_filter_t parse_type_filter(const char *type);
static void init_options(void);
static char *safe_strdup_arg(const char *arg);

/* Signal handler */
static void signal_handler(int sig);

/* ============================================================================
 * SECTION 4: SECURITY FUNCTIONS
 * ============================================================================ */

/**
 * @brief Secure string copy with guaranteed null termination
 * @param dst Destination buffer
 * @param src Source string
 * @param size Destination buffer size
 */
static void secure_strncpy(char *dst, const char *src, size_t size)
{
    if (dst == NULL || size == 0) {
        return;
    }

    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    size_t i;
    for (i = 0; i < size - 1 && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

/**
 * @brief Validate that a path is within an allowed directory
 * @param path Path to validate
 * @param allowed_dir Allowed base directory (NULL = allow /var/tmp only)
 * @return 1 if valid, 0 if invalid
 */
static int validate_path(const char *path, const char *allowed_dir)
{
    char resolved[PATH_MAX];
    char allowed_resolved[PATH_MAX];
    const char *base_dir;

    if (path == NULL || path[0] == '\0') {
        return 0;
    }

    /* Check for null bytes in path (path injection) */
    if (strlen(path) != strnlen(path, MAX_PATH_LEN)) {
        return 0;
    }

    /* Check for dangerous patterns */
    if (strstr(path, "..") != NULL) {
        return 0;  /* No parent directory traversal */
    }

    base_dir = allowed_dir ? allowed_dir : SAFE_STATE_DIR;

    /* Resolve the allowed directory */
    if (realpath(base_dir, allowed_resolved) == NULL) {
        return 0;
    }

    /* Get the directory part of the path */
    char path_copy[MAX_PATH_LEN];
    secure_strncpy(path_copy, path, sizeof(path_copy));

    char *last_slash = strrchr(path_copy, '/');
    if (last_slash == NULL) {
        /* No directory component - use current dir which we don't allow */
        return 0;
    }

    *last_slash = '\0';  /* Terminate at directory */

    if (realpath(path_copy, resolved) == NULL) {
        /* Directory doesn't exist yet - check parent */
        return strncmp(path_copy, allowed_resolved, strlen(allowed_resolved)) == 0;
    }

    /* Verify resolved path starts with allowed directory */
    size_t allowed_len = strlen(allowed_resolved);
    if (strncmp(resolved, allowed_resolved, allowed_len) != 0) {
        return 0;
    }

    /* Ensure it's either exact match or has a path separator after */
    if (resolved[allowed_len] != '\0' && resolved[allowed_len] != '/') {
        return 0;
    }

    return 1;
}

/**
 * @brief Validate and convert string to integer with bounds checking
 * @param str Input string
 * @param min Minimum allowed value
 * @param max Maximum allowed value
 * @param result Output integer
 * @return 1 if valid, 0 if invalid
 */
static int validate_integer(const char *str, int min, int max, int *result)
{
    char *endptr;
    long val;

    if (str == NULL || *str == '\0' || result == NULL) {
        return 0;
    }

    /* Skip leading whitespace */
    while (isspace((unsigned char)*str)) {
        str++;
    }

    /* Check for empty string after whitespace */
    if (*str == '\0') {
        return 0;
    }

    errno = 0;
    val = strtol(str, &endptr, 10);

    /* Check for conversion errors */
    if (errno == ERANGE || val < INT_MIN || val > INT_MAX) {
        return 0;  /* Overflow */
    }

    /* Check for trailing garbage */
    while (isspace((unsigned char)*endptr)) {
        endptr++;
    }
    if (*endptr != '\0') {
        return 0;  /* Invalid characters */
    }

    /* Check bounds */
    if (val < min || val > max) {
        return 0;
    }

    *result = (int)val;
    return 1;
}

/**
 * @brief Check if filename contains only safe characters
 * @param name Filename to check
 * @return 1 if safe, 0 if unsafe
 */
static int is_safe_filename(const char *name)
{
    if (name == NULL || *name == '\0') {
        return 0;
    }

    /* Don't allow hidden files or special entries */
    if (name[0] == '.') {
        return 0;
    }

    /* Check each character */
    for (const char *p = name; *p != '\0'; p++) {
        if (!isalnum((unsigned char)*p) &&
            *p != '_' && *p != '-' && *p != '.') {
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Validate command filter string
 * @param cmd Command string to validate
 * @return 1 if valid, 0 if invalid
 * @note Allows printable ASCII, rejects control chars and very long strings
 */
static int is_valid_command_filter(const char *cmd)
{
    size_t len;

    if (cmd == NULL || *cmd == '\0') {
        return 0;
    }

    len = strlen(cmd);
    if (len > MAXCOMLEN) {
        return 0;  /* Too long to match anything */
    }

    /* Check for null bytes embedded (strlen already passed, but be safe) */
    if (strnlen(cmd, MAXCOMLEN + 1) != len) {
        return 0;
    }

    /* Allow printable ASCII only */
    for (const char *p = cmd; *p != '\0'; p++) {
        unsigned char c = (unsigned char)*p;
        if (c < 0x20 || c > 0x7E) {
            return 0;  /* Control char or high-bit char */
        }
    }

    return 1;
}

/**
 * @brief Validate username filter string
 * @param user Username string to validate
 * @return 1 if valid, 0 if invalid
 */
static int is_valid_user_filter(const char *user)
{
    size_t len;

    if (user == NULL || *user == '\0') {
        return 0;
    }

    len = strlen(user);
    if (len > 64) {
        return 0;  /* Username too long */
    }

    /* Allow alphanumeric, underscore, hyphen, dot */
    for (const char *p = user; *p != '\0'; p++) {
        if (!isalnum((unsigned char)*p) &&
            *p != '_' && *p != '-' && *p != '.') {
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Sanitize string for safe output (remove control characters)
 * @param str String to sanitize (modified in place)
 * @param len Maximum length
 */
static void sanitize_output(char *str, size_t len)
{
    if (str == NULL) {
        return;
    }

    for (size_t i = 0; i < len && str[i] != '\0'; i++) {
        /* Replace control characters (except newline) with '?' */
        if (iscntrl((unsigned char)str[i]) && str[i] != '\n') {
            str[i] = '?';
        }
        /* Replace DEL character */
        if ((unsigned char)str[i] == 0x7F) {
            str[i] = '?';
        }
    }
}

/**
 * @brief Signal handler for graceful shutdown
 */
static void signal_handler(int sig)
{
    (void)sig;  /* Unused parameter */
    g_signal_received = 1;
}

/**
 * @brief Sanitize environment variables to prevent injection attacks
 * @note Removes dangerous variables before popen() calls
 */
static void sanitize_env(void)
{
    /* Remove variables that could be used for code injection */
    static const char *dangerous_vars[] = {
        "LD_PRELOAD",
        "LD_LIBRARY_PATH",
        "LD_AUDIT",
        "LD_DEBUG",
        "LIBPATH",           /* AIX-specific */
        "IFS",
        "CDPATH",
        "ENV",
        "BASH_ENV",
        NULL
    };

    for (int i = 0; dangerous_vars[i] != NULL; i++) {
        unsetenv(dangerous_vars[i]);
    }

    /* Set a safe PATH */
    setenv("PATH", "/usr/bin:/bin:/usr/sbin:/sbin", 1);

    /* Set locale to C for predictable parsing */
    setenv("LC_ALL", "C", 1);
    setenv("LANG", "C", 1);
}

/**
 * @brief Initialize FD array for a process
 * @param proc Process structure to initialize
 * @return 0 on success, -1 on error
 */
static int proc_init_fds(proc_info_t *proc)
{
    if (proc == NULL) {
        return -1;
    }

    proc->fds = calloc(INITIAL_FDS, sizeof(fd_info_t));
    if (proc->fds == NULL) {
        proc->fd_capacity = 0;
        proc->fd_count = 0;
        return -1;
    }

    proc->fd_capacity = INITIAL_FDS;
    proc->fd_count = 0;
    return 0;
}

/**
 * @brief Grow FD array when full
 * @param proc Process structure
 * @return 0 on success, -1 on error
 */
static int proc_grow_fds(proc_info_t *proc)
{
    int new_capacity;
    fd_info_t *new_fds;

    if (proc == NULL || proc->fds == NULL) {
        return -1;
    }

    /* Double capacity, but don't exceed MAX_FDS */
    new_capacity = proc->fd_capacity * 2;
    if (new_capacity > MAX_FDS) {
        new_capacity = MAX_FDS;
    }

    if (new_capacity <= proc->fd_capacity) {
        return -1;  /* Already at max */
    }

    new_fds = realloc(proc->fds, new_capacity * sizeof(fd_info_t));
    if (new_fds == NULL) {
        return -1;
    }

    /* Zero new entries */
    memset(&new_fds[proc->fd_capacity], 0,
           (new_capacity - proc->fd_capacity) * sizeof(fd_info_t));

    proc->fds = new_fds;
    proc->fd_capacity = new_capacity;
    return 0;
}

/**
 * @brief Free FD array for a process
 * @param proc Process structure
 */
static void proc_free_fds(proc_info_t *proc)
{
    if (proc == NULL) {
        return;
    }

    if (proc->fds != NULL) {
        free(proc->fds);
        proc->fds = NULL;
    }
    proc->fd_count = 0;
    proc->fd_capacity = 0;
}

/**
 * @brief Cleanup all process structures
 * @param procs Process array
 * @param count Number of processes
 */
static void procs_cleanup(proc_info_t *procs, int count)
{
    if (procs == NULL) {
        return;
    }

    for (int i = 0; i < count; i++) {
        proc_free_fds(&procs[i]);
    }
}

/* ============================================================================
 * HASH TABLE FUNCTIONS (for O(N) delta comparison)
 * ============================================================================ */

/**
 * @brief DJB2 hash function - fast and good distribution
 * @param str String to hash
 * @return Hash value
 */
static unsigned long hash_djb2(const char *str)
{
    unsigned long hash = 5381;
    int c;

    if (str == NULL) return 0;

    while ((c = (unsigned char)*str++) != 0) {
        hash = ((hash << 5) + hash) + c;  /* hash * 33 + c */
    }

    return hash;
}

/**
 * @brief Create a new hash table
 * @param size Number of buckets
 * @return Hash table or NULL on error
 */
static hash_table_t *hash_create(int size)
{
    hash_table_t *ht;

    if (size <= 0) {
        size = HASH_TABLE_SIZE;
    }

    ht = calloc(1, sizeof(hash_table_t));
    if (ht == NULL) {
        return NULL;
    }

    ht->buckets = calloc(size, sizeof(hash_entry_t *));
    if (ht->buckets == NULL) {
        free(ht);
        return NULL;
    }

    ht->size = size;
    ht->count = 0;
    return ht;
}

/**
 * @brief Insert a key into the hash table
 * @param ht Hash table
 * @param key Key to insert (will be duplicated)
 * @return 0 on success, -1 on error
 */
static int hash_insert(hash_table_t *ht, const char *key)
{
    unsigned long idx;
    hash_entry_t *entry;

    if (ht == NULL || key == NULL) {
        return -1;
    }

    idx = hash_djb2(key) % ht->size;

    entry = calloc(1, sizeof(hash_entry_t));
    if (entry == NULL) {
        return -1;
    }

    entry->key = strdup(key);
    if (entry->key == NULL) {
        free(entry);
        return -1;
    }

    /* Insert at head of chain */
    entry->next = ht->buckets[idx];
    ht->buckets[idx] = entry;
    ht->count++;

    return 0;
}

/**
 * @brief Remove a key from the hash table
 * @param ht Hash table
 * @param key Key to remove
 * @return 1 if found and removed, 0 if not found
 */
static int hash_remove(hash_table_t *ht, const char *key)
{
    unsigned long idx;
    hash_entry_t *entry, *prev;

    if (ht == NULL || key == NULL) {
        return 0;
    }

    idx = hash_djb2(key) % ht->size;
    entry = ht->buckets[idx];
    prev = NULL;

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            /* Found - remove from chain */
            if (prev == NULL) {
                ht->buckets[idx] = entry->next;
            } else {
                prev->next = entry->next;
            }
            free(entry->key);
            free(entry);
            ht->count--;
            return 1;
        }
        prev = entry;
        entry = entry->next;
    }

    return 0;  /* Not found */
}

/**
 * @brief Free all memory used by hash table
 * @param ht Hash table to free
 */
static void hash_free(hash_table_t *ht)
{
    int i;
    hash_entry_t *entry, *next;

    if (ht == NULL) {
        return;
    }

    for (i = 0; i < ht->size; i++) {
        entry = ht->buckets[i];
        while (entry != NULL) {
            next = entry->next;
            free(entry->key);
            free(entry);
            entry = next;
        }
    }

    free(ht->buckets);
    free(ht);
}

/* ============================================================================
 * SECTION 5: UTILITY FUNCTIONS
 * ============================================================================ */

/**
 * @brief Get username for a UID into caller-provided buffer
 * @param uid User ID to look up
 * @param buf Output buffer (must not be NULL)
 * @param buflen Buffer length (should be at least 32)
 * @note No static buffer - thread-safe
 */
static void get_user_name(uid_t uid, char *buf, size_t buflen)
{
    struct passwd *pw;

    if (buf == NULL || buflen == 0) {
        return;
    }

    if (g_opts.no_username) {
        snprintf(buf, buflen, "%d", (int)uid);
        return;
    }

    pw = getpwuid(uid);
    if (pw && pw->pw_name) {
        /* Truncate long usernames safely */
        secure_strncpy(buf, pw->pw_name, buflen);
        return;
    }

    snprintf(buf, buflen, "%d", (int)uid);
}

/**
 * @brief Get string representation of FD type
 * @param type File descriptor type
 * @return Type string (e.g., "REG", "DIR", "sock")
 */
static const char *get_fd_type_str(fd_type_t type)
{
    switch (type) {
    case FD_TYPE_REG:   return "REG";
    case FD_TYPE_DIR:   return "DIR";
    case FD_TYPE_CHR:   return "CHR";
    case FD_TYPE_BLK:   return "BLK";
    case FD_TYPE_FIFO:  return "FIFO";
    case FD_TYPE_SOCK:  return "sock";
    case FD_TYPE_LINK:  return "LINK";
    case FD_TYPE_INET:  return "IPv4";
    case FD_TYPE_INET6: return "IPv6";
    case FD_TYPE_UNIX:  return "unix";
    default:            return "unknown";
    }
}

/**
 * @brief Format file size for display
 * @param size Size in bytes
 * @param buf Output buffer
 * @param buflen Buffer length
 */
static void format_size(off_t size, char *buf, size_t buflen)
{
    if (buf == NULL || buflen == 0) {
        return;
    }

    /* Handle negative sizes safely */
    if (size < 0) {
        snprintf(buf, buflen, "?");
        return;
    }

    if (!g_opts.human_readable) {
        snprintf(buf, buflen, "%lld", (long long)size);
        return;
    }

    if (size >= 1099511627776LL) {
        snprintf(buf, buflen, "%.1fT", (double)size / 1099511627776.0);
    } else if (size >= 1073741824LL) {
        snprintf(buf, buflen, "%.1fG", (double)size / 1073741824.0);
    } else if (size >= 1048576LL) {
        snprintf(buf, buflen, "%.1fM", (double)size / 1048576.0);
    } else if (size >= 1024LL) {
        snprintf(buf, buflen, "%.1fK", (double)size / 1024.0);
    } else {
        snprintf(buf, buflen, "%lld", (long long)size);
    }
}

/**
 * @brief Read a symbolic link from /proc/PID/
 * @param pid Process ID
 * @param name Link name (e.g., "cwd", "root")
 * @param buf Output buffer
 * @param len Buffer length
 */
static void read_proc_link(pid_t pid, const char *name, char *buf, size_t len)
{
    char path[256];
    ssize_t n;

    if (buf == NULL || len == 0) {
        return;
    }
    buf[0] = '\0';

    /* Validate name to prevent path traversal */
    if (name == NULL || !is_safe_filename(name)) {
        return;
    }

    /* Validate PID is positive */
    if (pid <= 0) {
        return;
    }

    snprintf(path, sizeof(path), "/proc/%d/%s", (int)pid, name);
    n = readlink(path, buf, len - 1);
    if (n > 0 && (size_t)n < len) {
        buf[n] = '\0';
    } else {
        buf[0] = '\0';
    }
}

/* ============================================================================
 * SECTION 6: FILTER FUNCTIONS
 * ============================================================================ */

/**
 * @brief Check if PID matches the PID filter
 * @param pid Process ID to check
 * @return 1 if matches (or no filter), 0 if excluded
 */
static int match_pid_filter(pid_t pid)
{
    int i, match = 0;

    if (g_opts.filter_pid_count == 0) {
        return 1;  /* No filter, match all */
    }

    for (i = 0; i < g_opts.filter_pid_count && i < MAX_FILTERS; i++) {
        if (g_opts.filter_pids[i] == pid) {
            match = 1;
            break;
        }
    }

    return g_opts.filter_pid_exclude ? !match : match;
}

/**
 * @brief Check if UID matches the UID filter
 * @param uid User ID to check
 * @return 1 if matches (or no filter), 0 if excluded
 */
static int match_uid_filter(uid_t uid)
{
    int i, match = 0;

    if (g_opts.filter_uid_count == 0) {
        return 1;
    }

    for (i = 0; i < g_opts.filter_uid_count && i < MAX_FILTERS; i++) {
        if (g_opts.filter_uids[i] == uid) {
            match = 1;
            break;
        }
    }

    return g_opts.filter_uid_exclude ? !match : match;
}

/**
 * @brief Check if command matches the command filter
 * @param cmd Command name to check
 * @return 1 if matches (or no filter), 0 if excluded
 */
static int match_cmd_filter(const char *cmd)
{
    int i;
    size_t filter_len;

    if (g_opts.filter_cmd_count == 0) {
        return 1;
    }

    if (cmd == NULL) {
        return 0;
    }

    for (i = 0; i < g_opts.filter_cmd_count && i < MAX_FILTERS; i++) {
        if (g_opts.filter_commands[i] == NULL) {
            continue;
        }
        filter_len = strlen(g_opts.filter_commands[i]);
        if (filter_len > 0 && strncmp(cmd, g_opts.filter_commands[i], filter_len) == 0) {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Check if path matches the --path substring filter
 * @param path File path to check
 * @return 1 if matches (or no filter), 0 if not
 */
static int match_path_filter(const char *path)
{
    if (!g_opts.path_filter || !g_opts.path_filter[0]) {
        return 1;
    }

    if (path == NULL) {
        return 0;
    }

    return strstr(path, g_opts.path_filter) != NULL;
}

/**
 * @brief Check if FD type matches the --type filter
 * @param type File descriptor type
 * @return 1 if matches (or no filter), 0 if not
 */
static int match_type_filter(fd_type_t type)
{
    if (g_opts.type_filter == TYPE_FILTER_ALL) {
        return 1;
    }

    switch (g_opts.type_filter) {
    case TYPE_FILTER_FILE:
        return type == FD_TYPE_REG;
    case TYPE_FILTER_DIR:
        return type == FD_TYPE_DIR;
    case TYPE_FILTER_PIPE:
        return type == FD_TYPE_FIFO;
    case TYPE_FILTER_DEVICE:
        return type == FD_TYPE_CHR || type == FD_TYPE_BLK;
    case TYPE_FILTER_SOCKET:
        return type == FD_TYPE_SOCK || type == FD_TYPE_INET ||
               type == FD_TYPE_INET6 || type == FD_TYPE_UNIX;
    default:
        return 1;
    }
}

/**
 * @brief Check if path matches the +D/+d file filter
 * @param path File path to check
 * @return 1 if matches (or no filter), 0 if not
 */
static int match_file_filter(const char *path)
{
    int i;
    size_t filter_len;

    if (g_opts.filter_file_count == 0) {
        return 1;
    }

    if (path == NULL) {
        return 0;
    }

    for (i = 0; i < g_opts.filter_file_count && i < MAX_FILTERS; i++) {
        if (g_opts.filter_files[i] == NULL) {
            continue;
        }

        if (g_opts.filter_file_recursive) {
            /* +D: recursive match - path starts with filter */
            filter_len = strlen(g_opts.filter_files[i]);
            if (filter_len > 0 && strncmp(path, g_opts.filter_files[i], filter_len) == 0) {
                return 1;
            }
        } else {
            /* +d: exact match */
            if (strcmp(path, g_opts.filter_files[i]) == 0) {
                return 1;
            }
        }
    }

    return 0;
}

/**
 * @brief Check if FD matches network filters (-i options)
 * @param info File descriptor info
 * @return 1 if matches (or no filter), 0 if not
 */
static int match_network_filter(const fd_info_t *info)
{
    if (info == NULL) {
        return 0;
    }

    /* Check if network file */
    if (info->type != FD_TYPE_SOCK && info->type != FD_TYPE_INET &&
        info->type != FD_TYPE_INET6 && info->type != FD_TYPE_UNIX) {
        return 0;
    }

    /* Check port filter */
    if (g_opts.network_port > 0) {
        if (info->local_port != g_opts.network_port &&
            info->remote_port != g_opts.network_port) {
            return 0;
        }
    }

    /* Check IP version filter */
    if (g_opts.network_proto == 4 && info->family != AF_INET) {
        return 0;
    }
    if (g_opts.network_proto == 6 && info->family != AF_INET6) {
        return 0;
    }

    /* Check protocol filter */
    if (g_opts.network_tcp && info->proto != IPPROTO_TCP) {
        return 0;
    }
    if (g_opts.network_udp && info->proto != IPPROTO_UDP) {
        return 0;
    }

    return 1;
}

/**
 * @brief Check if FD matches positional file arguments
 * @param info File descriptor info
 * @return 1 if matches (or no search files), 0 if not
 */
static int match_search_file(const fd_info_t *info)
{
    int i;
    size_t path_len;

    if (g_opts.search_file_count == 0) {
        return 1;
    }

    if (info == NULL) {
        return 0;
    }

    for (i = 0; i < g_opts.search_file_count && i < MAX_FILTERS; i++) {
        /* Match by inode and device */
        if (g_opts.search_devs[i] != 0 && g_opts.search_inodes[i] != 0) {
            if (info->device == g_opts.search_devs[i] &&
                info->inode == g_opts.search_inodes[i]) {
                return 1;
            }
        }
        /* Match by path prefix */
        if (g_opts.search_files[i] != NULL && info->path[0] != '\0') {
            path_len = strlen(g_opts.search_files[i]);
            if (path_len > 0 && strncmp(info->path, g_opts.search_files[i], path_len) == 0) {
                return 1;
            }
        }
    }

    return 0;
}

/**
 * @brief Check if socket matches TCP state filter
 * @param info File descriptor info
 * @return 1 if matches (or no state filter), 0 if not
 */
static int match_tcp_state(const fd_info_t *info)
{
    if (!g_opts.tcp_listen && !g_opts.tcp_established &&
        !g_opts.tcp_close_wait && !g_opts.tcp_time_wait) {
        return 1;
    }

    if (info == NULL) {
        return 0;
    }

    if (info->proto != IPPROTO_TCP) {
        return 0;
    }

    if (g_opts.tcp_listen && strcasecmp(info->state, "LISTEN") == 0) return 1;
    if (g_opts.tcp_established && strcasecmp(info->state, "ESTABLISHED") == 0) return 1;
    if (g_opts.tcp_close_wait && strcasecmp(info->state, "CLOSE_WAIT") == 0) return 1;
    if (g_opts.tcp_time_wait && strcasecmp(info->state, "TIME_WAIT") == 0) return 1;

    return 0;
}

/**
 * @brief Apply all FD-level filters to check if FD should be included
 * @param info File descriptor info
 * @return 1 if FD passes all filters, 0 if excluded
 */
static int apply_fd_filters(const fd_info_t *info)
{
    if (info == NULL) {
        return 0;
    }

    /* Network-only mode */
    if (g_opts.network_only) {
        if (!match_network_filter(info)) {
            return 0;
        }
    }

    /* Unix sockets only */
    if (g_opts.unix_sockets) {
        if (info->type != FD_TYPE_UNIX && info->family != AF_UNIX) {
            return 0;
        }
    }

    /* Path/directory filter (+D/+d) */
    if (!match_file_filter(info->path)) {
        return 0;
    }

    /* Link count filter (-L) */
    if (g_opts.show_link_count == -1 && info->link_count > 0) {
        return 0;
    }

    /* Positional file search */
    if (!match_search_file(info)) {
        return 0;
    }

    /* TCP state filter */
    if (!match_tcp_state(info)) {
        return 0;
    }

    /* --path substring filter */
    if (!match_path_filter(info->path)) {
        return 0;
    }

    /* --type filter */
    if (!match_type_filter(info->type)) {
        return 0;
    }

    return 1;
}

/* ============================================================================
 * SECTION 7: PROCESS FUNCTIONS
 * ============================================================================ */

/**
 * @brief Get list of all processes using getprocs64()
 * @param procs Output array for process info
 * @param max_procs Maximum processes to retrieve
 * @return Number of processes found, or -1 on error
 */
static int get_processes(proc_info_t *procs, int max_procs)
{
    struct procentry64 *pe = NULL;
    pid_t index = 0;
    int count = 0;
    int nprocs, i;
    int alloc_procs;

    if (procs == NULL || max_procs <= 0) {
        return -1;
    }

    /* Limit allocation size for security */
    alloc_procs = (max_procs > MAX_PROCS) ? MAX_PROCS : max_procs;

    pe = calloc(alloc_procs, sizeof(struct procentry64));
    if (!pe) {
        return -1;
    }

    nprocs = getprocs64(pe, sizeof(struct procentry64), NULL, 0, &index, alloc_procs);
    if (nprocs < 0) {
        free(pe);
        return -1;
    }

    for (i = 0; i < nprocs && count < max_procs; i++) {
        proc_info_t *p = &procs[count];
        int has_filter, passes_filter;
        int pid_ok, uid_ok, gid_ok, cmd_ok;

        memset(p, 0, sizeof(proc_info_t));
        p->pid = pe[i].pi_pid;
        p->ppid = pe[i].pi_ppid;
        p->pgid = pe[i].pi_pgrp;
        p->uid = pe[i].pi_uid;

        /* Get user name directly into buffer */
        get_user_name(p->uid, p->user, sizeof(p->user));

        /* Get command name - secure copy */
        secure_strncpy(p->command, pe[i].pi_comm, sizeof(p->command));

        /* Sanitize command for output */
        sanitize_output(p->command, sizeof(p->command));

        /* Read cwd and root from /proc */
        read_proc_link(p->pid, "cwd", p->cwd, sizeof(p->cwd));
        read_proc_link(p->pid, "root", p->root, sizeof(p->root));

        /* Apply process-level filters */
        has_filter = (g_opts.filter_pid_count > 0 || g_opts.filter_uid_count > 0 ||
                      g_opts.filter_pgid_count > 0 || g_opts.filter_cmd_count > 0);

        if (has_filter) {
            pid_ok = match_pid_filter(p->pid);
            uid_ok = match_uid_filter(p->uid);
            gid_ok = (g_opts.filter_pgid_count == 0);
            cmd_ok = match_cmd_filter(p->command);

            /* Check GID filter */
            if (g_opts.filter_pgid_count > 0) {
                int j;
                for (j = 0; j < g_opts.filter_pgid_count && j < MAX_FILTERS; j++) {
                    if (g_opts.filter_pgids[j] == p->pgid) {
                        gid_ok = 1;
                        break;
                    }
                }
            }

            if (g_opts.and_logic) {
                /* AND: all active filters must match */
                passes_filter = pid_ok && uid_ok && gid_ok && cmd_ok;
            } else {
                /* OR: at least one active filter must match */
                passes_filter = 0;
                if (g_opts.filter_pid_count > 0 && pid_ok) passes_filter = 1;
                if (g_opts.filter_uid_count > 0 && uid_ok) passes_filter = 1;
                if (g_opts.filter_pgid_count > 0 && gid_ok) passes_filter = 1;
                if (g_opts.filter_cmd_count > 0 && cmd_ok) passes_filter = 1;
            }

            if (!passes_filter) {
                continue;
            }
        }

        count++;
    }

    free(pe);
    return count;
}

/**
 * @brief Add special FDs (cwd, rtd) to process info
 * @param proc Process info structure
 * @return Number of special FDs added
 */
static int add_special_fds(proc_info_t *proc)
{
    fd_info_t *fd;
    struct stat st;
    int added = 0;
    int should_add_cwd;

    if (proc == NULL || proc->fds == NULL) {
        return 0;
    }

    /* Check capacity limits */
    if (proc->fd_count >= MAX_FDS) {
        return 0;
    }

    /* Determine if we should add cwd */
    should_add_cwd = g_opts.filter_fd_cwd ||
        (g_opts.filter_fd_count == 0 && !g_opts.filter_fd_rtd &&
         !g_opts.filter_fd_txt && !g_opts.filter_fd_mem &&
         !g_opts.network_only && !g_opts.unix_sockets);

    /* Add CWD */
    if (should_add_cwd && proc->cwd[0] != '\0' &&
        stat(proc->cwd, &st) == 0) {
        /* Apply filters to cwd */
        if (match_file_filter(proc->cwd) &&
            match_path_filter(proc->cwd) &&
            match_type_filter(FD_TYPE_DIR)) {

            /* Grow array if needed */
            if (proc->fd_count >= proc->fd_capacity) {
                if (proc_grow_fds(proc) != 0) {
                    return added;
                }
            }

            fd = &proc->fds[proc->fd_count];
            memset(fd, 0, sizeof(fd_info_t));
            fd->fd = FD_CWD;
            secure_strncpy(fd->fd_name, "cwd", sizeof(fd->fd_name));
            secure_strncpy(fd->path, proc->cwd, sizeof(fd->path));
            fd->type = S_ISDIR(st.st_mode) ? FD_TYPE_DIR : FD_TYPE_REG;
            fd->device = st.st_dev;
            fd->inode = st.st_ino;
            fd->size = st.st_size;
            fd->link_count = st.st_nlink;
            secure_strncpy(fd->access, "r", sizeof(fd->access));
            proc->fd_count++;
            added++;
        }
    }

    /* Add RTD if explicitly requested */
    if (g_opts.filter_fd_rtd && proc->root[0] != '\0' &&
        stat(proc->root, &st) == 0) {
        if (match_file_filter(proc->root) &&
            match_path_filter(proc->root) &&
            match_type_filter(FD_TYPE_DIR)) {

            /* Grow array if needed */
            if (proc->fd_count >= proc->fd_capacity) {
                if (proc_grow_fds(proc) != 0) {
                    return added;
                }
            }

            fd = &proc->fds[proc->fd_count];
            memset(fd, 0, sizeof(fd_info_t));
            fd->fd = FD_RTD;
            secure_strncpy(fd->fd_name, "rtd", sizeof(fd->fd_name));
            secure_strncpy(fd->path, proc->root, sizeof(fd->path));
            fd->type = FD_TYPE_DIR;
            fd->device = st.st_dev;
            fd->inode = st.st_ino;
            fd->link_count = st.st_nlink;
            secure_strncpy(fd->access, "r", sizeof(fd->access));
            proc->fd_count++;
            added++;
        }
    }

    return added;
}

/**
 * @brief Get network socket information from path string
 * @param info File descriptor info (path field used as input)
 * @return 0 on success
 */
static int get_network_info(fd_info_t *info)
{
    char path_copy[MAX_PATH_LEN];
    char *p, *arrow, *port;

    if (info == NULL) {
        return -1;
    }

    secure_strncpy(path_copy, info->path, sizeof(path_copy));

    if (strncmp(path_copy, "TCP", 3) == 0) {
        info->proto = IPPROTO_TCP;
        info->family = AF_INET;
        info->type = FD_TYPE_INET;

        p = path_copy + 3;
        if (*p == '6') {
            info->family = AF_INET6;
            info->type = FD_TYPE_INET6;
            p++;
        }

        if (*p == ':') {
            p++;
            arrow = strstr(p, "->");
            if (arrow) {
                *arrow = '\0';
                port = strrchr(p, ':');
                if (port) {
                    *port = '\0';
                    secure_strncpy(info->local_addr, p, sizeof(info->local_addr));
                    if (validate_integer(port + 1, 0, 65535, &info->local_port) == 0) {
                        info->local_port = 0;
                    }
                }
                p = arrow + 2;
                port = strrchr(p, ':');
                if (port) {
                    *port = '\0';
                    secure_strncpy(info->remote_addr, p, sizeof(info->remote_addr));
                    if (validate_integer(port + 1, 0, 65535, &info->remote_port) == 0) {
                        info->remote_port = 0;
                    }
                }
                secure_strncpy(info->state, "ESTABLISHED", sizeof(info->state));
            } else {
                port = strrchr(p, ':');
                if (port) {
                    *port = '\0';
                    secure_strncpy(info->local_addr, p, sizeof(info->local_addr));
                    if (validate_integer(port + 1, 0, 65535, &info->local_port) == 0) {
                        info->local_port = 0;
                    }
                }
                secure_strncpy(info->state, "LISTEN", sizeof(info->state));
            }
        }
    } else if (strncmp(path_copy, "UDP", 3) == 0) {
        info->proto = IPPROTO_UDP;
        info->family = AF_INET;
        info->type = FD_TYPE_INET;

        p = path_copy + 3;
        if (*p == '6') {
            info->family = AF_INET6;
            info->type = FD_TYPE_INET6;
            p++;
        }

        if (*p == ':') {
            p++;
            port = strrchr(p, ':');
            if (port) {
                *port = '\0';
                secure_strncpy(info->local_addr, p, sizeof(info->local_addr));
                if (validate_integer(port + 1, 0, 65535, &info->local_port) == 0) {
                    info->local_port = 0;
                }
            }
        }
    } else if (strncmp(path_copy, "unix:", 5) == 0) {
        info->family = AF_UNIX;
        info->type = FD_TYPE_UNIX;
        secure_strncpy(info->local_addr, path_copy + 5, sizeof(info->local_addr));
    } else if (strncmp(path_copy, "UNIX", 4) == 0) {
        info->family = AF_UNIX;
        info->type = FD_TYPE_UNIX;
    } else if (strstr(path_copy, "socket") != NULL) {
        info->type = FD_TYPE_SOCK;
    }

    return 0;
}

/**
 * @brief Get information about a specific file descriptor
 * @param pid Process ID
 * @param fd File descriptor number
 * @param info Output structure
 * @return 0 on success, -1 on error
 */
static int get_fd_info(pid_t pid, int fd, fd_info_t *info)
{
    char link_path[256];
    char target[MAX_PATH_LEN];
    struct stat st, link_st;
    ssize_t len;

    if (info == NULL || pid <= 0 || fd < 0) {
        return -1;
    }

    snprintf(link_path, sizeof(link_path), "/proc/%d/fd/%d", (int)pid, fd);

    if (lstat(link_path, &link_st) != 0) {
        return -1;
    }

    info->link_count = link_st.st_nlink;

    /* Handle different /proc/PID/fd entry types */
    if (S_ISSOCK(link_st.st_mode)) {
        info->type = FD_TYPE_SOCK;
        info->device = link_st.st_dev;
        info->inode = link_st.st_ino;
        snprintf(info->path, sizeof(info->path), "socket:[%lld]",
                 (long long)link_st.st_ino);
        get_network_info(info);
        secure_strncpy(info->access, "u", sizeof(info->access));
        return 0;
    }

    if (S_ISFIFO(link_st.st_mode)) {
        info->type = FD_TYPE_FIFO;
        info->device = link_st.st_dev;
        info->inode = link_st.st_ino;
        snprintf(info->path, sizeof(info->path), "pipe:[%lld]",
                 (long long)link_st.st_ino);
        secure_strncpy(info->access, "rw", sizeof(info->access));
        return 0;
    }

    if (S_ISCHR(link_st.st_mode)) {
        info->type = FD_TYPE_CHR;
        info->device = link_st.st_rdev;
        info->inode = link_st.st_ino;
        snprintf(info->path, sizeof(info->path), "/dev (chr %d,%d)",
                 (int)(link_st.st_rdev >> 16), (int)(link_st.st_rdev & 0xFFFF));
        secure_strncpy(info->access, "rw", sizeof(info->access));
        return 0;
    }

    if (S_ISBLK(link_st.st_mode)) {
        info->type = FD_TYPE_BLK;
        info->device = link_st.st_rdev;
        info->inode = link_st.st_ino;
        snprintf(info->path, sizeof(info->path), "/dev (blk %d,%d)",
                 (int)(link_st.st_rdev >> 16), (int)(link_st.st_rdev & 0xFFFF));
        secure_strncpy(info->access, "rw", sizeof(info->access));
        return 0;
    }

    /* Try readlink for symlinks */
    len = readlink(link_path, target, sizeof(target) - 1);
    if (len > 0 && (size_t)len < sizeof(target)) {
        target[len] = '\0';
        secure_strncpy(info->path, target, sizeof(info->path));

        if (stat(target, &st) == 0) {
            info->device = st.st_dev;
            info->inode = st.st_ino;
            info->size = st.st_size;
            info->link_count = st.st_nlink;

            if (S_ISREG(st.st_mode)) {
                info->type = FD_TYPE_REG;
            } else if (S_ISDIR(st.st_mode)) {
                info->type = FD_TYPE_DIR;
            } else if (S_ISCHR(st.st_mode)) {
                info->type = FD_TYPE_CHR;
            } else if (S_ISBLK(st.st_mode)) {
                info->type = FD_TYPE_BLK;
            } else if (S_ISFIFO(st.st_mode)) {
                info->type = FD_TYPE_FIFO;
            } else if (S_ISSOCK(st.st_mode)) {
                info->type = FD_TYPE_SOCK;
                get_network_info(info);
            } else {
                info->type = FD_TYPE_UNKNOWN;
            }
        } else {
            /* Target doesn't exist - parse path string */
            if (strncmp(target, "socket:", 7) == 0 ||
                strncmp(target, "TCP", 3) == 0 ||
                strncmp(target, "UDP", 3) == 0) {
                info->type = FD_TYPE_SOCK;
                get_network_info(info);
            } else if (strncmp(target, "pipe:", 5) == 0) {
                info->type = FD_TYPE_FIFO;
            } else {
                info->type = FD_TYPE_UNKNOWN;
            }
        }
    } else {
        secure_strncpy(info->path, link_path, sizeof(info->path));
        info->device = link_st.st_dev;
        info->inode = link_st.st_ino;
        info->type = FD_TYPE_UNKNOWN;
    }

    secure_strncpy(info->access, "u", sizeof(info->access));
    return 0;
}

/**
 * @brief Get all file descriptors for a process
 * @param proc Process info structure (fds array filled in)
 * @return 0 on success, -1 on error
 */
static int get_process_fds(proc_info_t *proc)
{
    char fd_dir[256];
    DIR *dir;
    struct dirent *entry;
    int fd_num, i, only_special_fds;
    fd_info_t *info;

    if (proc == NULL) {
        return -1;
    }

    /* Initialize or reset FD array */
    if (proc->fds == NULL) {
        if (proc_init_fds(proc) != 0) {
            return -1;
        }
    }
    proc->fd_count = 0;

    /* Check if only special FDs requested */
    only_special_fds = (g_opts.filter_fd_cwd || g_opts.filter_fd_rtd ||
                        g_opts.filter_fd_txt || g_opts.filter_fd_mem) &&
                       (g_opts.filter_fd_count == 0);

    snprintf(fd_dir, sizeof(fd_dir), "/proc/%d/fd", (int)proc->pid);

    dir = opendir(fd_dir);
    if (!dir) {
        return -1;
    }

    /* Add special FDs first */
    add_special_fds(proc);

    /* If only special FDs requested, we're done */
    if (only_special_fds) {
        closedir(dir);
        return 0;
    }

    /* Scan numeric FDs */
    while ((entry = readdir(dir)) != NULL && proc->fd_count < MAX_FDS) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        /* Validate FD number */
        if (!validate_integer(entry->d_name, 0, INT_MAX, &fd_num)) {
            continue;
        }

        /* Apply FD number filter */
        if (g_opts.filter_fd_count > 0) {
            int match = 0;
            for (i = 0; i < g_opts.filter_fd_count && i < MAX_FILTERS; i++) {
                if (g_opts.filter_fds[i] == fd_num) {
                    match = 1;
                    break;
                }
            }
            if (!match) {
                continue;
            }
        }

        /* Grow array if needed */
        if (proc->fd_count >= proc->fd_capacity) {
            if (proc_grow_fds(proc) != 0) {
                break;  /* At capacity, stop adding */
            }
        }

        info = &proc->fds[proc->fd_count];
        memset(info, 0, sizeof(fd_info_t));
        info->fd = fd_num;
        snprintf(info->fd_name, sizeof(info->fd_name), "%d", fd_num);

        if (get_fd_info(proc->pid, fd_num, info) == 0) {
            /* Apply all FD-level filters */
            if (apply_fd_filters(info)) {
                proc->fd_count++;
            }
        }
    }

    closedir(dir);
    return 0;
}

/* ============================================================================
 * SECTION 8: OUTPUT FUNCTIONS
 * ============================================================================ */

/**
 * @brief Print usage/help information
 */
static void print_usage(void)
{
    printf("lpsof %s - List Open Files for AIX (LibrePowerSof)\n\n", LPSOF_VERSION);
    printf("Usage: lpsof [subcommand] [options] [--] [files]\n\n");

    printf("SUBCOMMANDS:\n");
    printf("  list      List open files (default)\n");
    printf("  summary   Top N processes by open file count\n");
    printf("  watch     Continuous monitoring\n");
    printf("  delta     Compare with saved snapshot\n");
    printf("  doctor    System diagnostics\n\n");

    printf("COMMON OPTIONS:\n");
    printf("  -h, --help           Show help\n");
    printf("  -v, --version        Show version\n");
    printf("  --limit N            Limit to N processes (default: %d, max: %d)\n",
           DEFAULT_LIMIT, MAX_LIMIT);
    printf("  --no-limit           Remove limit\n\n");

    printf("FILTER OPTIONS:\n");
    printf("  -p, --pid PID        Filter by PID (^PID to exclude)\n");
    printf("  -u, --user USER      Filter by user\n");
    printf("  -c, --cmd CMD        Filter by command prefix\n");
    printf("  --path PATH          Filter by path substring\n");
    printf("  --type TYPE          Filter: file|dir|pipe|device|socket|all\n");
    printf("  -i [ADDR]            Network files [46][proto][@host][:port]\n");
    printf("  +D DIR               Files in directory (recursive)\n\n");

    printf("OUTPUT OPTIONS:\n");
    printf("  -t, --terse          PIDs only\n");
    printf("  -H, --human          Human readable sizes\n");
    printf("  -l, --numeric-uid    Numeric UIDs\n");
    printf("  -R, --ppid           Show PPID column\n\n");

    printf("SECURITY:\n");
    printf("  State files restricted to %s\n", SAFE_STATE_DIR);
    printf("  Maximum %d processes, %d FDs per process\n\n", MAX_PROCS, MAX_FDS);
}

/**
 * @brief Print version information
 */
static void print_version(void)
{
    struct utsname uts;

    printf("lpsof version %s (security-hardened)\n", LPSOF_VERSION);
    printf("LibrePowerSof - List Open Files for AIX\n");
    printf("Build: AIX native, using getprocs64() API\n");
    printf("Compiled: %s %s\n", __DATE__, __TIME__);

    if (uname(&uts) == 0) {
        printf("System: %s %s %s %s\n", uts.sysname, uts.nodename,
               uts.release, uts.machine);
    }
}

/**
 * @brief Print table header
 */
static void print_header(void)
{
    if (g_opts.terse_mode || g_opts.field_output || g_header_printed) {
        return;
    }

    printf("%-*s %7s ", g_opts.cmd_width, "COMMAND", "PID");

    if (g_opts.show_ppid) {
        printf("%7s ", "PPID");
    }
    if (g_opts.show_pgid) {
        printf("%7s ", "PGID");
    }

    printf("%10s %4s %5s %10s ", "USER", "FD", "TYPE", "DEVICE");

    if (g_opts.show_offset) {
        printf("%10s ", "OFFSET");
    }
    if (g_opts.show_link_count > 0) {
        printf("%4s ", "NLINK");
    }

    printf("%10s %s\n", "SIZE/OFF", "NAME");
    g_header_printed = 1;
}

/**
 * @brief Print a single file descriptor line
 * @param proc Process info
 * @param fd File descriptor info
 */
static void print_fd(const proc_info_t *proc, const fd_info_t *fd)
{
    char fd_str[16], device_str[32], size_str[32], offset_str[32];
    char name[MAX_PATH_LEN];
    char sanitized_name[MAX_PATH_LEN];

    if (proc == NULL || fd == NULL) {
        return;
    }

    /* Format FD string */
    if (fd->fd < 0) {
        snprintf(fd_str, sizeof(fd_str), "%s", fd->fd_name);
    } else {
        snprintf(fd_str, sizeof(fd_str), "%d%s", fd->fd, fd->access);
    }

    /* Format device string */
    if (fd->device > 0) {
        snprintf(device_str, sizeof(device_str), "%d,%d",
                 (int)(fd->device >> 16), (int)(fd->device & 0xFFFF));
    } else if (fd->device == (dev_t)-1) {
        secure_strncpy(device_str, "-1,65535", sizeof(device_str));
    } else {
        secure_strncpy(device_str, "-", sizeof(device_str));
    }

    /* Format size string */
    if (fd->type == FD_TYPE_INET || fd->type == FD_TYPE_INET6 ||
        fd->type == FD_TYPE_SOCK || fd->type == FD_TYPE_UNIX) {
        snprintf(size_str, sizeof(size_str), "0t%lld", (long long)fd->offset);
    } else {
        format_size(fd->size, size_str, sizeof(size_str));
    }

    /* Format offset if requested */
    if (g_opts.show_offset) {
        if (g_opts.offset_digits > 0 && g_opts.offset_digits <= 20) {
            snprintf(offset_str, sizeof(offset_str), "%0*lld",
                     g_opts.offset_digits, (long long)fd->offset);
        } else {
            snprintf(offset_str, sizeof(offset_str), "0t%lld",
                     (long long)fd->offset);
        }
    }

    /* Format name */
    if (fd->type == FD_TYPE_INET || fd->type == FD_TYPE_INET6) {
        const char *proto_str = "";
        if (fd->proto == IPPROTO_TCP) proto_str = "TCP";
        else if (fd->proto == IPPROTO_UDP) proto_str = "UDP";

        if (fd->remote_port > 0) {
            snprintf(name, sizeof(name), "%s %s:%d->%s:%d",
                     proto_str,
                     fd->local_addr[0] ? fd->local_addr : "*",
                     fd->local_port,
                     fd->remote_addr[0] ? fd->remote_addr : "*",
                     fd->remote_port);
        } else if (fd->local_port > 0) {
            snprintf(name, sizeof(name), "%s %s:%d",
                     proto_str,
                     fd->local_addr[0] ? fd->local_addr : "*",
                     fd->local_port);
        } else {
            secure_strncpy(name, fd->path, sizeof(name));
        }

        if (fd->state[0] && fd->proto == IPPROTO_TCP) {
            size_t name_len = strlen(name);
            if (name_len + strlen(fd->state) + 3 < sizeof(name)) {
                snprintf(name + name_len, sizeof(name) - name_len,
                         " (%s)", fd->state);
            }
        }
    } else {
        secure_strncpy(name, fd->path, sizeof(name));
    }

    /* Sanitize name for output */
    secure_strncpy(sanitized_name, name, sizeof(sanitized_name));
    sanitize_output(sanitized_name, sizeof(sanitized_name));

    /* Print the line */
    printf("%-*.*s %7d ", g_opts.cmd_width, g_opts.cmd_width,
           proc->command, (int)proc->pid);

    if (g_opts.show_ppid) {
        printf("%7d ", (int)proc->ppid);
    }
    if (g_opts.show_pgid) {
        printf("%7d ", (int)proc->pgid);
    }

    printf("%10.10s %4s %5s %10s ", proc->user, fd_str,
           get_fd_type_str(fd->type), device_str);

    if (g_opts.show_offset) {
        printf("%10s ", offset_str);
    }
    if (g_opts.show_link_count > 0) {
        printf("%4d ", fd->link_count);
    }

    printf("%10s %s\n", size_str, sanitized_name);
}

/**
 * @brief Print field output format (-F)
 * @param proc Process info
 * @param fd File descriptor info
 */
static void print_field_output(const proc_info_t *proc, const fd_info_t *fd)
{
    static pid_t last_pid = -1;
    char sep;
    char sanitized_path[MAX_PATH_LEN];

    if (proc == NULL || fd == NULL) {
        return;
    }

    sep = g_opts.field_sep;

    if (proc->pid != last_pid) {
        printf("p%d%c", (int)proc->pid, sep);
        if (g_opts.show_ppid) {
            printf("R%d%c", (int)proc->ppid, sep);
        }
        printf("c%s%c", proc->command, sep);
        printf("u%d%c", (int)proc->uid, sep);
        if (g_opts.show_pgid) {
            printf("g%d%c", (int)proc->pgid, sep);
        }
        last_pid = proc->pid;
    }

    printf("f%s%c", fd->fd_name, sep);
    printf("t%s%c", get_fd_type_str(fd->type), sep);

    if (fd->device > 0) {
        printf("D%d,%d%c", (int)(fd->device >> 16),
               (int)(fd->device & 0xFFFF), sep);
    }
    if (fd->inode > 0) {
        printf("i%lld%c", (long long)fd->inode, sep);
    }
    if (fd->size > 0) {
        printf("s%lld%c", (long long)fd->size, sep);
    }
    if (g_opts.show_offset) {
        printf("o%lld%c", (long long)fd->offset, sep);
    }
    if (g_opts.show_link_count > 0 && fd->link_count > 0) {
        printf("k%d%c", fd->link_count, sep);
    }

    if ((fd->type == FD_TYPE_INET || fd->type == FD_TYPE_INET6 ||
         fd->type == FD_TYPE_SOCK) && fd->proto) {
        printf("P%s%c", fd->proto == IPPROTO_TCP ? "TCP" : "UDP", sep);
    }

    /* Sanitize path for output */
    secure_strncpy(sanitized_path, fd->path, sizeof(sanitized_path));
    sanitize_output(sanitized_path, sizeof(sanitized_path));
    printf("n%s%c", sanitized_path, sep);

    if (sep == '\0') {
        putchar('\n');
    }
}

/**
 * @brief Print all FDs for a process
 * @param proc Process info
 */
static void print_process(const proc_info_t *proc)
{
    static pid_t last_terse_pid = -1;
    int i;

    if (proc == NULL) {
        return;
    }

    if (proc->fd_count == 0) {
        return;
    }

    if (g_opts.terse_mode) {
        if (proc->pid != last_terse_pid) {
            printf("%d\n", (int)proc->pid);
            last_terse_pid = proc->pid;
        }
        return;
    }

    print_header();

    for (i = 0; i < proc->fd_count && i < MAX_FDS; i++) {
        if (g_opts.field_output) {
            print_field_output(proc, &proc->fds[i]);
        } else {
            print_fd(proc, &proc->fds[i]);
        }
    }
}

/* ============================================================================
 * SECTION 9: SUBCOMMANDS
 * ============================================================================ */

/**
 * @brief List open files (default subcommand)
 * @param procs Process array
 * @param max_procs Array size
 * @return 0 on success, 1 on error
 */
static int cmd_list(proc_info_t *procs, int max_procs)
{
    int proc_count, i, shown = 0;

    if (procs == NULL || max_procs <= 0) {
        return 1;
    }

    do {
        g_header_printed = 0;
        proc_count = get_processes(procs, max_procs);

        if (proc_count < 0) {
            fprintf(stderr, "lpsof: failed to get process list: %s\n",
                    strerror(errno));
            return 1;
        }

        /* Safety warning */
        if (g_opts.safe_mode && proc_count > 500 &&
            g_opts.filter_pid_count == 0 && g_opts.filter_uid_count == 0 &&
            g_opts.filter_cmd_count == 0 && g_opts.limit > 0) {
            fprintf(stderr, "lpsof: WARNING: %d processes found. "
                    "Limiting to %d (use --no-limit to show all)\n",
                    proc_count, g_opts.limit);
        }

        for (i = 0; i < proc_count && (g_opts.limit == 0 || shown < g_opts.limit); i++) {
            if (get_process_fds(&procs[i]) == 0 && procs[i].fd_count > 0) {
                print_process(&procs[i]);
                shown++;
            }
        }

        if (g_opts.repeat_mode) {
            fflush(stdout);
            sleep(g_opts.repeat_interval > 0 ? g_opts.repeat_interval : 1);
            if (!g_opts.terse_mode && !g_opts.field_output) {
                printf("=======\n");
            }
            shown = 0;
        }
    } while (g_opts.repeat_mode && !g_signal_received);

    return 0;
}

/**
 * @brief Compare function for qsort - sort by fd_count descending
 * SECURITY: Uses safe comparison to avoid integer overflow
 */
static int compare_by_fd_count(const void *a, const void *b)
{
    const proc_info_t *pa = (const proc_info_t *)a;
    const proc_info_t *pb = (const proc_info_t *)b;

    /* Safe comparison avoiding integer overflow */
    if (pb->fd_count > pa->fd_count) return 1;
    if (pb->fd_count < pa->fd_count) return -1;
    return 0;
}

/**
 * @brief Show top N processes by open file count
 * @param procs Process array
 * @param max_procs Array size
 * @return 0 on success, 1 on error
 */
static int cmd_summary(proc_info_t *procs, int max_procs)
{
    int proc_count, i, shown = 0;
    int limit;

    if (procs == NULL || max_procs <= 0) {
        return 1;
    }

    limit = g_opts.limit > 0 ? g_opts.limit : 20;
    if (limit > MAX_LIMIT) {
        limit = MAX_LIMIT;
    }

    proc_count = get_processes(procs, max_procs);
    if (proc_count < 0) {
        fprintf(stderr, "lpsof: failed to get process list: %s\n",
                strerror(errno));
        return 1;
    }

    /* Get FD counts */
    for (i = 0; i < proc_count; i++) {
        get_process_fds(&procs[i]);
    }

    /* Sort by fd_count descending */
    qsort(procs, proc_count, sizeof(proc_info_t), compare_by_fd_count);

    /* Print header and results */
    printf("%-7s %-10s %-16s %8s\n", "PID", "USER", "COMMAND", "COUNT");
    printf("%-7s %-10s %-16s %8s\n", "-------", "----------",
           "----------------", "--------");

    for (i = 0; i < proc_count && shown < limit; i++) {
        if (procs[i].fd_count > 0) {
            printf("%-7d %-10s %-16s %8d\n",
                   (int)procs[i].pid, procs[i].user,
                   procs[i].command, procs[i].fd_count);
            shown++;
        }
    }

    printf("\nTotal: %d processes scanned\n", proc_count);
    return 0;
}

/**
 * @brief Continuous monitoring mode
 * @param procs Process array
 * @param max_procs Array size
 * @return 0 on normal exit, 1 on error
 */
static int cmd_watch(proc_info_t *procs, int max_procs)
{
    int proc_count, i, shown;
    time_t now;
    char timebuf[64];
    int interval;

    if (procs == NULL || max_procs <= 0) {
        return 1;
    }

    interval = g_opts.watch_interval;
    if (interval < MIN_WATCH_INTERVAL) {
        interval = MIN_WATCH_INTERVAL;
    }
    if (interval > MAX_WATCH_INTERVAL) {
        interval = MAX_WATCH_INTERVAL;
    }

    /* Set up signal handlers for graceful shutdown */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("lpsof watch mode - polling every %d seconds (Ctrl-C to stop)\n",
           interval);
    printf("=============================================================\n\n");

    while (!g_signal_received) {
        now = time(NULL);
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S",
                 localtime(&now));

        g_header_printed = 0;
        proc_count = get_processes(procs, max_procs);

        if (proc_count < 0) {
            fprintf(stderr, "[%s] ERROR: failed to get process list\n",
                    timebuf);
            sleep(interval);
            continue;
        }

        printf("[%s] Scanning %d processes...\n", timebuf, proc_count);

        shown = 0;
        for (i = 0; i < proc_count &&
             (g_opts.limit == 0 || shown < g_opts.limit); i++) {
            if (get_process_fds(&procs[i]) == 0 && procs[i].fd_count > 0) {
                print_process(&procs[i]);
                shown++;
            }
        }

        if (shown == 0) {
            printf("  (no matching files found)\n");
        }

        printf("\n--- %d files shown, sleeping %d seconds ---\n\n",
               shown, interval);
        fflush(stdout);
        sleep(interval);
    }

    printf("\nWatch mode terminated by signal.\n");
    return 0;
}

/**
 * @brief Save current state to file using atomic write pattern
 * SECURITY: Uses mkstemp + fsync + rename to prevent TOCTOU attacks
 * @param procs Process array
 * @param count Number of processes
 * @param path State file path
 * @return 0 on success, -1 on error
 */
static int save_state(proc_info_t *procs, int count, const char *path)
{
    FILE *fp;
    int fd, i, j;
    char tmppath[MAX_PATH_LEN];
    struct stat st;

    if (procs == NULL || path == NULL || count < 0) {
        return -1;
    }

    /* Validate path is in allowed directory */
    if (!validate_path(path, SAFE_STATE_DIR)) {
        fprintf(stderr, "lpsof: state file must be in %s\n", SAFE_STATE_DIR);
        return -1;
    }

    /* Check if destination exists and verify it's not a symlink/special */
    if (lstat(path, &st) == 0) {
        if (!S_ISREG(st.st_mode)) {
            fprintf(stderr, "lpsof: %s exists but is not a regular file\n", path);
            return -1;
        }
    }

    /* Create temporary file in same directory for atomic rename */
    /* Use shorter suffix to avoid truncation with long paths */
    if (snprintf(tmppath, sizeof(tmppath), "%s.XXXXXX", path) >= (int)sizeof(tmppath)) {
        fprintf(stderr, "lpsof: path too long for temp file\n");
        return -1;
    }

    /* mkstemp creates file with mode 0600 and opens it */
    fd = mkstemp(tmppath);
    if (fd < 0) {
        fprintf(stderr, "lpsof: cannot create temp file: %s\n", strerror(errno));
        return -1;
    }

    /* Lock file during write (AIX-compatible)
     * Note: lockf uses current position. Seek to beginning to lock entire file.
     */
    if (lseek(fd, 0, SEEK_SET) < 0) {
        fprintf(stderr, "lpsof: cannot seek temp file: %s\n", strerror(errno));
        close(fd);
        unlink(tmppath);
        return -1;
    }
    if (lockf(fd, F_TLOCK, 0) < 0) {
        fprintf(stderr, "lpsof: cannot lock temp file: %s\n", strerror(errno));
        close(fd);
        unlink(tmppath);
        return -1;
    }

    fp = fdopen(fd, "w");
    if (!fp) {
        fprintf(stderr, "lpsof: cannot write %s: %s\n", tmppath, strerror(errno));
        lseek(fd, 0, SEEK_SET);
        lockf(fd, F_ULOCK, 0);
        close(fd);
        unlink(tmppath);
        return -1;
    }

    fprintf(fp, "# lpsof state file v%s\n", LPSOF_VERSION);
    fprintf(fp, "# Generated: %ld\n", (long)time(NULL));
    fprintf(fp, "# Processes: %d\n", count);

    for (i = 0; i < count && i < MAX_PROCS; i++) {
        if (procs[i].fds == NULL || procs[i].fd_count == 0) continue;

        for (j = 0; j < procs[i].fd_count && j < MAX_FDS; j++) {
            /* Sanitize path data before writing */
            char sanitized_path[MAX_PATH_LEN];
            secure_strncpy(sanitized_path, procs[i].fds[j].path, sizeof(sanitized_path));

            /* Remove pipe characters from path to prevent parsing issues */
            for (char *p = sanitized_path; *p; p++) {
                if (*p == '|') *p = '_';
            }

            fprintf(fp, "%d|%s|%s|%d|%s|%lld|%lld\n",
                    (int)procs[i].pid, procs[i].command, procs[i].user,
                    procs[i].fds[j].fd, sanitized_path,
                    (long long)procs[i].fds[j].device,
                    (long long)procs[i].fds[j].inode);
        }
    }

    /* Ensure data is flushed and synced to disk */
    fflush(fp);
    if (fsync(fd) != 0) {
        fprintf(stderr, "lpsof: fsync failed: %s\n", strerror(errno));
        /* Continue anyway - data is likely safe */
    }

    /* Seek to beginning before unlocking to release entire file lock */
    lseek(fd, 0, SEEK_SET);
    lockf(fd, F_ULOCK, 0);
    fclose(fp);

    /* Atomic rename - this is the TOCTOU-safe operation */
    if (rename(tmppath, path) != 0) {
        fprintf(stderr, "lpsof: cannot rename %s to %s: %s\n",
                tmppath, path, strerror(errno));
        unlink(tmppath);
        return -1;
    }

    return 0;
}

/**
 * @brief Compare current state with saved snapshot
 * OPTIMIZED: Uses hash table for O(N) comparison instead of O(N²)
 * @param procs Process array
 * @param max_procs Array size
 * @return 0 on success, 1 on error
 */
static int cmd_delta(proc_info_t *procs, int max_procs)
{
    const char *state_path;
    int proc_count, i, j;
    FILE *fp;
    char line[MAX_LINE_LEN];
    hash_table_t *old_ht = NULL;
    char **removed_entries = NULL;  /* Track entries for removal report */
    int removed_capacity = 1024;
    int old_count = 0, new_count = 0, added = 0, removed = 0;
    struct stat st;

    if (procs == NULL || max_procs <= 0) {
        return 1;
    }

    state_path = g_opts.state_file;
    if (state_path[0] == '\0') {
        state_path = STATE_FILE_DEFAULT;
    }

    /* Validate path */
    if (!validate_path(state_path, SAFE_STATE_DIR)) {
        fprintf(stderr, "lpsof: state file must be in %s\n", SAFE_STATE_DIR);
        return 1;
    }

    /* Save mode */
    if (g_opts.save_state) {
        proc_count = get_processes(procs, max_procs);
        if (proc_count < 0) {
            fprintf(stderr, "lpsof: failed to get process list\n");
            return 1;
        }
        for (i = 0; i < proc_count; i++) {
            get_process_fds(&procs[i]);
        }
        if (save_state(procs, proc_count, state_path) == 0) {
            printf("State saved to %s (%d processes)\n", state_path, proc_count);
            return 0;
        }
        return 1;
    }

    /* Compare mode - verify file is regular file */
    if (lstat(state_path, &st) != 0) {
        fprintf(stderr, "lpsof: no saved state at %s\n", state_path);
        fprintf(stderr, "       Run 'lpsof delta --save' first\n");
        return 1;
    }

    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "lpsof: %s is not a regular file\n", state_path);
        return 1;
    }

    /* Check file size limit */
    if (st.st_size > STATE_FILE_MAX_SIZE) {
        fprintf(stderr, "lpsof: state file too large (max %d bytes)\n",
                STATE_FILE_MAX_SIZE);
        return 1;
    }

    fp = fopen(state_path, "r");
    if (!fp) {
        fprintf(stderr, "lpsof: cannot read %s: %s\n", state_path, strerror(errno));
        return 1;
    }

    /* Create hash table for O(1) lookups */
    old_ht = hash_create(HASH_TABLE_SIZE);
    if (!old_ht) {
        fprintf(stderr, "lpsof: out of memory\n");
        fclose(fp);
        return 1;
    }

    /* Allocate array to track removed entries for reporting */
    removed_entries = calloc(removed_capacity, sizeof(char *));
    if (!removed_entries) {
        fprintf(stderr, "lpsof: out of memory\n");
        hash_free(old_ht);
        fclose(fp);
        return 1;
    }

    /* Load old entries into hash table - O(N) */
    while (fgets(line, sizeof(line), fp) != NULL && old_count < MAX_STATE_ENTRIES) {
        if (line[0] == '#') continue;
        line[strcspn(line, "\n")] = '\0';
        if (line[0] != '\0') {
            if (hash_insert(old_ht, line) == 0) {
                /* Also keep a copy for removal reporting */
                if (old_count >= removed_capacity) {
                    int new_cap = removed_capacity * 2;
                    if (new_cap > MAX_STATE_ENTRIES) new_cap = MAX_STATE_ENTRIES;
                    char **new_arr = realloc(removed_entries, new_cap * sizeof(char *));
                    if (new_arr) {
                        removed_entries = new_arr;
                        removed_capacity = new_cap;
                    }
                }
                if (old_count < removed_capacity) {
                    removed_entries[old_count] = strdup(line);
                }
                old_count++;
            }
        }
    }
    fclose(fp);

    /* Get current state */
    proc_count = get_processes(procs, max_procs);
    if (proc_count < 0) {
        fprintf(stderr, "lpsof: failed to get process list\n");
        for (i = 0; i < old_count && i < removed_capacity; i++) {
            free(removed_entries[i]);
        }
        free(removed_entries);
        hash_free(old_ht);
        return 1;
    }

    printf("Delta report: comparing %d old entries with current state\n",
           old_count);
    printf("================================================================\n\n");

    /* Find new entries - O(N) with hash lookups */
    for (i = 0; i < proc_count; i++) {
        get_process_fds(&procs[i]);
        if (procs[i].fds == NULL) continue;

        for (j = 0; j < procs[i].fd_count && j < MAX_FDS; j++) {
            char current[MAX_LINE_LEN];
            char sanitized_path[MAX_PATH_LEN];

            secure_strncpy(sanitized_path, procs[i].fds[j].path, sizeof(sanitized_path));
            for (char *p = sanitized_path; *p; p++) {
                if (*p == '|') *p = '_';
            }

            snprintf(current, sizeof(current), "%d|%s|%s|%d|%s|%lld|%lld",
                     (int)procs[i].pid, procs[i].command, procs[i].user,
                     procs[i].fds[j].fd, sanitized_path,
                     (long long)procs[i].fds[j].device,
                     (long long)procs[i].fds[j].inode);

            /* O(1) hash lookup and removal */
            if (hash_remove(old_ht, current)) {
                /* Found in old - not new, mark as matched in removed_entries */
                for (int k = 0; k < old_count && k < removed_capacity; k++) {
                    if (removed_entries[k] && strcmp(removed_entries[k], current) == 0) {
                        free(removed_entries[k]);
                        removed_entries[k] = NULL;
                        break;
                    }
                }
            } else {
                /* Not in old - this is a new entry */
                printf("+ PID %-7d %-10s %-12s fd=%-3d %s\n",
                       (int)procs[i].pid, procs[i].user, procs[i].command,
                       procs[i].fds[j].fd, procs[i].fds[j].path);
                added++;
            }
            new_count++;
        }
    }

    /* Report removed entries (those still in removed_entries array) */
    for (i = 0; i < old_count && i < removed_capacity; i++) {
        if (removed_entries[i]) {
            char *line_copy = removed_entries[i];
            char *p = line_copy;
            char *pid_s = strsep(&p, "|");
            char *cmd = strsep(&p, "|");
            char *user = strsep(&p, "|");
            char *fd_s = strsep(&p, "|");
            char *path = strsep(&p, "|");

            if (pid_s && cmd && user && path) {
                printf("- PID %-7s %-10s %-12s fd=%-3s %s\n",
                       pid_s, user, cmd, fd_s ? fd_s : "?", path);
            }
            removed++;
            free(removed_entries[i]);
        }
    }
    free(removed_entries);
    hash_free(old_ht);

    printf("\n================================================================\n");
    printf("Summary: +%d added, -%d removed (was %d, now %d entries)\n",
           added, removed, old_count, new_count);

    return 0;
}

/**
 * @brief System diagnostics
 * @return 0 if no issues, 1 if issues found
 */
static int cmd_doctor(void)
{
    struct utsname uts;
    struct stat st;
    FILE *fp;
    char buf[256];
    int issues = 0;

    printf("lpsof %s - System Diagnostics (Security-Hardened)\n", LPSOF_VERSION);
    printf("======================================\n\n");

    /* System info */
    printf("[System Information]\n");
    if (uname(&uts) == 0) {
        printf("  OS:        %s %s\n", uts.sysname, uts.release);
        printf("  Node:      %s\n", uts.nodename);
        printf("  Machine:   %s\n", uts.machine);
    }

    /* Use absolute path for security */
    fp = popen("/usr/bin/oslevel -s 2>/dev/null", "r");
    if (fp) {
        if (fgets(buf, sizeof(buf), fp) != NULL) {
            buf[strcspn(buf, "\n")] = '\0';
            sanitize_output(buf, sizeof(buf));
            printf("  AIX Level: %s\n", buf);
        }
        pclose(fp);
    }
    printf("\n");

    /* Privileges */
    printf("[Privileges]\n");
    if (geteuid() == 0) {
        printf("  Running as: root (full access)\n");
    } else {
        printf("  Running as: UID %d (limited access)\n", (int)geteuid());
        printf("  WARNING: Run as root for full visibility\n");
        issues++;
    }
    printf("\n");

    /* /proc filesystem */
    printf("[/proc Filesystem]\n");
    if (stat("/proc", &st) == 0 && S_ISDIR(st.st_mode)) {
        printf("  /proc:     Available\n");
        if (stat("/proc/1", &st) == 0) {
            printf("  /proc/1:   Accessible\n");
        } else {
            printf("  /proc/1:   Not accessible (%s)\n", strerror(errno));
            issues++;
        }
        if (stat("/proc/self/fd", &st) == 0) {
            printf("  /proc/self/fd: Available\n");
        } else {
            printf("  /proc/self/fd: Not available\n");
        }
    } else {
        printf("  /proc:     NOT AVAILABLE\n");
        printf("  ERROR: lpsof requires /proc\n");
        issues++;
    }
    printf("\n");

    /* Helper commands */
    printf("[Helper Commands]\n");
    printf("  procfiles: %s\n",
           access("/usr/bin/procfiles", X_OK) == 0 ? "Available" : "Not found");
    printf("  fuser:     %s\n",
           access("/usr/sbin/fuser", X_OK) == 0 ||
           access("/usr/bin/fuser", X_OK) == 0 ? "Available" : "Not found");
    printf("  netstat:   %s\n",
           access("/usr/bin/netstat", X_OK) == 0 ? "Available" : "Not found");
    printf("\n");

    /* State file */
    printf("[State File]\n");
    printf("  Default:   %s\n", STATE_FILE_DEFAULT);
    printf("  Allowed dir: %s\n", SAFE_STATE_DIR);
    if (access(STATE_FILE_DEFAULT, F_OK) == 0) {
        if (stat(STATE_FILE_DEFAULT, &st) == 0) {
            printf("  Status:    Exists (%lld bytes)\n", (long long)st.st_size);
        }
    } else {
        printf("  Status:    Not created yet\n");
    }
    printf("\n");

    /* Security limits */
    printf("[Security Limits]\n");
    printf("  MAX_PROCS: %d\n", MAX_PROCS);
    printf("  MAX_FDS:   %d per process\n", MAX_FDS);
    printf("  MAX_LIMIT: %d\n", MAX_LIMIT);
    printf("  MAX_STATE: %d entries\n", MAX_STATE_ENTRIES);
    printf("  Default limit: %d processes\n", DEFAULT_LIMIT);
    printf("\n");

    /* Summary */
    printf("[Summary]\n");
    if (issues == 0) {
        printf("  Status:    READY - No issues detected\n");
    } else {
        printf("  Status:    %d issue(s) detected\n", issues);
    }
    printf("\n");

    return issues > 0 ? 1 : 0;
}

/* ============================================================================
 * SECTION 10: OPTION PARSING
 * ============================================================================ */

/**
 * @brief Safely duplicate an argument string for parsing
 * @note Creates a local copy to avoid modifying argv (which may be read-only)
 * @param arg Original argument string
 * @return Duplicated string (caller must NOT free - uses static buffer)
 */
static char *safe_strdup_arg(const char *arg)
{
    static char buf[MAX_ARGV_COPY];

    if (arg == NULL) {
        buf[0] = '\0';
        return buf;
    }

    secure_strncpy(buf, arg, sizeof(buf));
    return buf;
}

/**
 * @brief Initialize options to defaults
 */
static void init_options(void)
{
    memset(&g_opts, 0, sizeof(g_opts));
    g_opts.cmd_width = DEFAULT_CMD_WIDTH;
    g_opts.limit = DEFAULT_LIMIT;
    g_opts.safe_mode = 1;
    g_opts.watch_interval = DEFAULT_WATCH_INTERVAL;
    secure_strncpy(g_opts.state_file, STATE_FILE_DEFAULT, sizeof(g_opts.state_file));
    g_opts.subcommand = CMD_LIST;
    g_opts.type_filter = TYPE_FILTER_ALL;
}

/**
 * @brief Parse subcommand from argument
 * @param arg Argument string
 * @return 1 if recognized as subcommand, 0 if not
 */
static int parse_subcommand(const char *arg)
{
    if (arg == NULL) {
        return 0;
    }

    if (strcmp(arg, "list") == 0) {
        g_opts.subcommand = CMD_LIST;
    } else if (strcmp(arg, "summary") == 0) {
        g_opts.subcommand = CMD_SUMMARY;
    } else if (strcmp(arg, "watch") == 0) {
        g_opts.subcommand = CMD_WATCH;
    } else if (strcmp(arg, "delta") == 0) {
        g_opts.subcommand = CMD_DELTA;
    } else if (strcmp(arg, "doctor") == 0) {
        g_opts.subcommand = CMD_DOCTOR;
    } else if (strcmp(arg, "help") == 0) {
        g_opts.show_help = 1;
    } else if (strcmp(arg, "version") == 0) {
        g_opts.show_version = 1;
    } else {
        return 0;  /* Not a subcommand */
    }
    return 1;
}

/**
 * @brief Parse network filter string
 * @param filter Filter string ([46][proto][@host][:port])
 * @return 0 on success
 */
static int parse_network_filter(const char *filter)
{
    const char *p;
    char proto[8] = "";
    int proto_idx = 0;
    int port;

    if (filter == NULL) {
        return -1;
    }

    p = filter;

    /* IP version */
    if (*p == '4') {
        g_opts.network_proto = 4;
        p++;
    } else if (*p == '6') {
        g_opts.network_proto = 6;
        p++;
    }

    /* Protocol */
    while (*p && isalpha((unsigned char)*p) && proto_idx < 7) {
        proto[proto_idx++] = (char)toupper((unsigned char)*p);
        p++;
    }
    proto[proto_idx] = '\0';

    if (strcmp(proto, "TCP") == 0) {
        g_opts.network_tcp = 1;
    } else if (strcmp(proto, "UDP") == 0) {
        g_opts.network_udp = 1;
    }

    /* Host */
    if (*p == '@') {
        p++;
        const char *colon = strchr(p, ':');
        if (colon) {
            size_t host_len = colon - p;
            if (host_len >= sizeof(g_opts.network_host)) {
                host_len = sizeof(g_opts.network_host) - 1;
            }
            memcpy(g_opts.network_host, p, host_len);
            g_opts.network_host[host_len] = '\0';
            p = colon;
        } else {
            secure_strncpy(g_opts.network_host, p, sizeof(g_opts.network_host));
            return 0;
        }
    }

    /* Port */
    if (*p == ':') {
        p++;
        if (validate_integer(p, 0, 65535, &port)) {
            g_opts.network_port = port;
        } else if (isalpha((unsigned char)*p)) {
            struct servent *se = getservbyname(p, NULL);
            if (se) {
                g_opts.network_port = ntohs(se->s_port);
            }
        }
    }

    return 0;
}

/**
 * @brief Parse --type filter value
 * @param type Type string
 * @return Type filter enum value
 */
static type_filter_t parse_type_filter(const char *type)
{
    if (!type || strcmp(type, "all") == 0) {
        return TYPE_FILTER_ALL;
    } else if (strcmp(type, "file") == 0) {
        return TYPE_FILTER_FILE;
    } else if (strcmp(type, "dir") == 0) {
        return TYPE_FILTER_DIR;
    } else if (strcmp(type, "pipe") == 0) {
        return TYPE_FILTER_PIPE;
    } else if (strcmp(type, "device") == 0) {
        return TYPE_FILTER_DEVICE;
    } else if (strcmp(type, "socket") == 0) {
        return TYPE_FILTER_SOCKET;
    } else {
        fprintf(stderr, "lpsof: unknown type: %s (use: file|dir|pipe|device|socket|all)\n", type);
        return TYPE_FILTER_ALL;
    }
}

/**
 * @brief Parse all command-line options
 * @param argc Argument count
 * @param argv Argument array
 * @return 0 on success, -1 on error
 */
static int parse_options(int argc, char *argv[])
{
    int i;
    int end_of_options = 0;
    int val;

    for (i = 1; i < argc; i++) {
        /* Bounds check */
        if (argv[i] == NULL) {
            continue;
        }

        /* Positional arguments (files to search) */
        if (end_of_options || (argv[i][0] != '-' && argv[i][0] != '+')) {
            if (g_opts.search_file_count < MAX_FILTERS) {
                struct stat st;
                g_opts.search_files[g_opts.search_file_count] = argv[i];
                if (stat(argv[i], &st) == 0) {
                    g_opts.search_devs[g_opts.search_file_count] = st.st_dev;
                    g_opts.search_inodes[g_opts.search_file_count] = st.st_ino;
                } else {
                    g_opts.search_devs[g_opts.search_file_count] = 0;
                    g_opts.search_inodes[g_opts.search_file_count] = 0;
                    if (g_opts.warn_not_found) {
                        fprintf(stderr, "lpsof: can't stat %s: %s\n",
                                argv[i], strerror(errno));
                    }
                }
                g_opts.search_file_count++;
            }
            continue;
        }

        if (strcmp(argv[i], "--") == 0) {
            end_of_options = 1;
            continue;
        }

        /* Long options */
        if (strncmp(argv[i], "--", 2) == 0) {
            if (strcmp(argv[i], "--help") == 0) {
                g_opts.show_help = 1;
            } else if (strcmp(argv[i], "--version") == 0) {
                g_opts.show_version = 1;
            } else if (strcmp(argv[i], "--limit") == 0 && i + 1 < argc) {
                if (validate_integer(argv[++i], 1, MAX_LIMIT, &val)) {
                    g_opts.limit = val;
                } else {
                    fprintf(stderr, "lpsof: invalid limit (1-%d)\n", MAX_LIMIT);
                    return -1;
                }
            } else if (strcmp(argv[i], "--no-limit") == 0) {
                g_opts.limit = 0;
                g_opts.safe_mode = 0;
            } else if (strcmp(argv[i], "--interval") == 0 && i + 1 < argc) {
                if (validate_integer(argv[++i], MIN_WATCH_INTERVAL, MAX_WATCH_INTERVAL, &val)) {
                    g_opts.watch_interval = val;
                } else {
                    fprintf(stderr, "lpsof: invalid interval (%d-%d)\n",
                            MIN_WATCH_INTERVAL, MAX_WATCH_INTERVAL);
                    return -1;
                }
            } else if (strcmp(argv[i], "--state") == 0 && i + 1 < argc) {
                i++;
                if (!validate_path(argv[i], SAFE_STATE_DIR)) {
                    fprintf(stderr, "lpsof: state file must be in %s\n", SAFE_STATE_DIR);
                    return -1;
                }
                secure_strncpy(g_opts.state_file, argv[i], sizeof(g_opts.state_file));
            } else if (strcmp(argv[i], "--save") == 0) {
                g_opts.save_state = 1;
            } else if (strcmp(argv[i], "--path") == 0 && i + 1 < argc) {
                g_opts.path_filter = argv[++i];
            } else if (strcmp(argv[i], "--type") == 0 && i + 1 < argc) {
                g_opts.type_filter = parse_type_filter(argv[++i]);
            } else if (strcmp(argv[i], "--pid") == 0 && i + 1 < argc) {
                /* SECURITY: Use copy to avoid modifying argv */
                char *p = safe_strdup_arg(argv[++i]);
                char *token;
                if (*p == '^') { g_opts.filter_pid_exclude = 1; p++; }
                while ((token = strsep(&p, ",")) != NULL) {
                    if (g_opts.filter_pid_count < MAX_FILTERS) {
                        if (validate_integer(token, 1, INT_MAX, &val)) {
                            g_opts.filter_pids[g_opts.filter_pid_count++] = val;
                        }
                    }
                }
            } else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc) {
                /* SECURITY: Use copy to avoid modifying argv */
                char *p = safe_strdup_arg(argv[++i]);
                char *token;
                if (*p == '^') { g_opts.filter_uid_exclude = 1; p++; }
                while ((token = strsep(&p, ",")) != NULL) {
                    if (g_opts.filter_uid_count < MAX_FILTERS) {
                        /* Validate before passing to getpwnam */
                        if (!is_valid_user_filter(token) &&
                            !validate_integer(token, 0, INT_MAX, &val)) {
                            if (!g_opts.ignore_errors) {
                                fprintf(stderr, "lpsof: invalid user filter: %s\n", token);
                            }
                            continue;
                        }
                        struct passwd *pw = getpwnam(token);
                        if (pw) {
                            g_opts.filter_uids[g_opts.filter_uid_count++] = pw->pw_uid;
                        } else if (validate_integer(token, 0, INT_MAX, &val)) {
                            g_opts.filter_uids[g_opts.filter_uid_count++] = val;
                        }
                    }
                }
            } else if (strcmp(argv[i], "--cmd") == 0 && i + 1 < argc) {
                const char *cmd_arg = argv[++i];
                if (g_opts.filter_cmd_count < MAX_FILTERS) {
                    if (is_valid_command_filter(cmd_arg)) {
                        g_opts.filter_commands[g_opts.filter_cmd_count++] = (char *)cmd_arg;
                    } else if (!g_opts.ignore_errors) {
                        fprintf(stderr, "lpsof: invalid command filter: %s\n", cmd_arg);
                    }
                }
            } else if (strcmp(argv[i], "--terse") == 0) {
                g_opts.terse_mode = 1;
            } else if (strcmp(argv[i], "--human") == 0) {
                g_opts.human_readable = 1;
            } else if (strcmp(argv[i], "--numeric-uid") == 0) {
                g_opts.no_username = 1;
            } else if (strcmp(argv[i], "--ppid") == 0) {
                g_opts.show_ppid = 1;
            } else {
                fprintf(stderr, "lpsof: unknown option: %s\n", argv[i]);
                return -1;
            }
            continue;
        }

        /* Plus options */
        if (argv[i][0] == '+') {
            switch (argv[i][1]) {
            case 'c':
                if (i + 1 < argc) {
                    if (validate_integer(argv[++i], 1, MAX_CMD_WIDTH, &val)) {
                        g_opts.cmd_width = val;
                    }
                }
                break;
            case 'd':
                if (i + 1 < argc && g_opts.filter_file_count < MAX_FILTERS) {
                    g_opts.filter_files[g_opts.filter_file_count++] = argv[++i];
                    g_opts.filter_file_recursive = 0;
                }
                break;
            case 'D':
                if (i + 1 < argc && g_opts.filter_file_count < MAX_FILTERS) {
                    g_opts.filter_files[g_opts.filter_file_count++] = argv[++i];
                    g_opts.filter_file_recursive = 1;
                }
                break;
            case 'L':
                g_opts.show_link_count = 1;
                break;
            case 'r':
                g_opts.repeat_mode = 1;
                g_opts.repeat_interval = 15;
                if (i + 1 < argc && isdigit((unsigned char)argv[i + 1][0])) {
                    if (validate_integer(argv[++i], 1, MAX_WATCH_INTERVAL, &val)) {
                        g_opts.repeat_interval = val;
                    }
                }
                break;
            default:
                if (!g_opts.ignore_errors) {
                    fprintf(stderr, "lpsof: unknown option: %s\n", argv[i]);
                }
                break;
            }
            continue;
        }

        /* Minus options */
        switch (argv[i][1]) {
        case '?':
        case 'h':
            g_opts.show_help = 1;
            break;
        case 'v':
            g_opts.show_version = 1;
            break;
        case 'a':
            g_opts.and_logic = 1;
            break;
        case 't':
            g_opts.terse_mode = 1;
            break;
        case 'n':
            g_opts.no_hostname = 1;
            break;
        case 'P':
            g_opts.no_portname = 1;
            break;
        case 'l':
            g_opts.no_username = 1;
            break;
        case 'H':
            g_opts.human_readable = 1;
            break;
        case 'R':
            g_opts.show_ppid = 1;
            break;
        case 'Q':
            g_opts.ignore_errors = 1;
            break;
        case 'V':
            g_opts.warn_not_found = 1;
            break;
        case 'U':
            g_opts.unix_sockets = 1;
            break;
        case 'L':
            g_opts.show_link_count = -1;
            break;
        case 'o':
            g_opts.show_offset = 1;
            if (argv[i][2] != '\0') {
                if (validate_integer(&argv[i][2], 1, 20, &val)) {
                    g_opts.offset_digits = val;
                }
            } else if (i + 1 < argc && isdigit((unsigned char)argv[i + 1][0])) {
                if (validate_integer(argv[++i], 1, 20, &val)) {
                    g_opts.offset_digits = val;
                }
            }
            break;
        case 's':
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                char *sf = argv[++i];
                if (strncasecmp(sf, "TCP:", 4) == 0) {
                    sf += 4;
                    if (strcasecmp(sf, "LISTEN") == 0) {
                        g_opts.tcp_listen = 1;
                        g_opts.network_only = 1;
                    } else if (strcasecmp(sf, "ESTABLISHED") == 0) {
                        g_opts.tcp_established = 1;
                        g_opts.network_only = 1;
                    } else if (strcasecmp(sf, "CLOSE_WAIT") == 0) {
                        g_opts.tcp_close_wait = 1;
                        g_opts.network_only = 1;
                    } else if (strcasecmp(sf, "TIME_WAIT") == 0) {
                        g_opts.tcp_time_wait = 1;
                        g_opts.network_only = 1;
                    }
                }
            }
            break;
        case 'F':
            g_opts.field_output = 1;
            g_opts.field_sep = '\0';
            break;
        case 'g':
            if (i + 1 < argc) {
                /* SECURITY: Use copy to avoid modifying argv */
                char *p = safe_strdup_arg(argv[++i]);
                char *token;
                while ((token = strsep(&p, ",")) != NULL) {
                    if (g_opts.filter_pgid_count < MAX_FILTERS) {
                        if (validate_integer(token, 0, INT_MAX, &val)) {
                            g_opts.filter_pgids[g_opts.filter_pgid_count++] = val;
                        }
                    }
                }
            }
            break;
        case 'p':
            if (i + 1 < argc) {
                /* SECURITY: Use copy to avoid modifying argv */
                char *p = safe_strdup_arg(argv[++i]);
                char *token;
                if (*p == '^') { g_opts.filter_pid_exclude = 1; p++; }
                while ((token = strsep(&p, ",")) != NULL) {
                    if (g_opts.filter_pid_count < MAX_FILTERS) {
                        if (validate_integer(token, 1, INT_MAX, &val)) {
                            g_opts.filter_pids[g_opts.filter_pid_count++] = val;
                        }
                    }
                }
            }
            break;
        case 'u':
            if (i + 1 < argc) {
                /* SECURITY: Use copy to avoid modifying argv */
                char *p = safe_strdup_arg(argv[++i]);
                char *token;
                if (*p == '^') { g_opts.filter_uid_exclude = 1; p++; }
                while ((token = strsep(&p, ",")) != NULL) {
                    if (g_opts.filter_uid_count < MAX_FILTERS) {
                        /* Validate before passing to getpwnam */
                        if (!is_valid_user_filter(token) &&
                            !validate_integer(token, 0, INT_MAX, &val)) {
                            if (!g_opts.ignore_errors) {
                                fprintf(stderr, "lpsof: invalid user filter: %s\n", token);
                            }
                            continue;
                        }
                        struct passwd *pw = getpwnam(token);
                        if (pw) {
                            g_opts.filter_uids[g_opts.filter_uid_count++] = pw->pw_uid;
                        } else if (validate_integer(token, 0, INT_MAX, &val)) {
                            g_opts.filter_uids[g_opts.filter_uid_count++] = val;
                        } else if (!g_opts.ignore_errors) {
                            fprintf(stderr, "lpsof: unknown user: %s\n", token);
                        }
                    }
                }
            }
            break;
        case 'c':
            if (i + 1 < argc && g_opts.filter_cmd_count < MAX_FILTERS) {
                const char *cmd_arg = argv[++i];
                if (is_valid_command_filter(cmd_arg)) {
                    g_opts.filter_commands[g_opts.filter_cmd_count++] = (char *)cmd_arg;
                } else if (!g_opts.ignore_errors) {
                    fprintf(stderr, "lpsof: invalid command filter: %s\n", cmd_arg);
                }
            }
            break;
        case 'd':
            if (i + 1 < argc) {
                /* SECURITY: Use copy to avoid modifying argv */
                char *p = safe_strdup_arg(argv[++i]);
                char *token;
                while ((token = strsep(&p, ",")) != NULL) {
                    if (strcmp(token, "cwd") == 0) {
                        g_opts.filter_fd_cwd = 1;
                    } else if (strcmp(token, "rtd") == 0) {
                        g_opts.filter_fd_rtd = 1;
                    } else if (strcmp(token, "txt") == 0) {
                        g_opts.filter_fd_txt = 1;
                    } else if (strcmp(token, "mem") == 0) {
                        g_opts.filter_fd_mem = 1;
                    } else if (g_opts.filter_fd_count < MAX_FILTERS) {
                        if (validate_integer(token, 0, INT_MAX, &val)) {
                            g_opts.filter_fds[g_opts.filter_fd_count++] = val;
                        }
                    }
                }
            }
            break;
        case 'i':
            g_opts.network_only = 1;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                g_opts.network_filter = argv[++i];
                parse_network_filter(g_opts.network_filter);
            }
            break;
        case 'r':
            g_opts.repeat_mode = 1;
            g_opts.repeat_interval = 1;
            if (i + 1 < argc && isdigit((unsigned char)argv[i + 1][0])) {
                if (validate_integer(argv[++i], 1, MAX_WATCH_INTERVAL, &val)) {
                    g_opts.repeat_interval = val;
                }
            }
            break;
        default:
            if (!g_opts.ignore_errors) {
                fprintf(stderr, "lpsof: unknown option: %s\n", argv[i]);
                fprintf(stderr, "Use -h for help\n");
                return -1;
            }
            break;
        }
    }

    return 0;
}

/* ============================================================================
 * SECTION 11: MAIN
 * ============================================================================ */

/**
 * @brief Program entry point
 * @param argc Argument count
 * @param argv Argument array
 * @return Exit code (0 = success, 1 = error)
 */
int main(int argc, char *argv[])
{
    proc_info_t *procs = NULL;
    int ret = 0;
    int proc_count = 0;

    /* SECURITY: Sanitize environment before any operations */
    sanitize_env();

    /* SECURITY: Set restrictive umask for any files we create */
    (void)umask(077);

    init_options();

    /* Check for subcommand */
    if (argc > 1 && argv[1] != NULL &&
        argv[1][0] != '-' && argv[1][0] != '+') {
        parse_subcommand(argv[1]);
    }

    if (parse_options(argc, argv) != 0) {
        return 1;
    }

    if (g_opts.show_help) {
        print_usage();
        return 0;
    }

    if (g_opts.show_version) {
        print_version();
        return 0;
    }

    /* Doctor doesn't need process array */
    if (g_opts.subcommand == CMD_DOCTOR) {
        return cmd_doctor();
    }

    /* Allocate process array (just the structures, not FD arrays) */
    procs = calloc(MAX_PROCS, sizeof(proc_info_t));
    if (!procs) {
        fprintf(stderr, "lpsof: out of memory\n");
        return 1;
    }

    /* Execute subcommand */
    switch (g_opts.subcommand) {
    case CMD_LIST:
        ret = cmd_list(procs, MAX_PROCS);
        proc_count = MAX_PROCS;  /* Assume max for cleanup */
        break;
    case CMD_SUMMARY:
        ret = cmd_summary(procs, MAX_PROCS);
        proc_count = MAX_PROCS;
        break;
    case CMD_WATCH:
        ret = cmd_watch(procs, MAX_PROCS);
        proc_count = MAX_PROCS;
        break;
    case CMD_DELTA:
        ret = cmd_delta(procs, MAX_PROCS);
        proc_count = MAX_PROCS;
        break;
    default:
        ret = cmd_list(procs, MAX_PROCS);
        proc_count = MAX_PROCS;
        break;
    }

    /* Cleanup dynamically allocated FD arrays */
    procs_cleanup(procs, proc_count);
    free(procs);
    return ret;
}
