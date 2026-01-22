/*
 * C-Sentinel - Semantic Observability for UNIX Systems
 * Copyright (c) 2025 William Murray
 *
 * Licensed under the MIT License.
 * See LICENSE file for details.
 *
 * aix_files.c - Critical AIX files to monitor for security compliance
 *
 * This list is based on:
 * - IBM PowerSC RTC (Real Time Compliance) file list
 * - CIS IBM AIX Benchmark
 * - DoD STIG for AIX
 * - AIX Security Expert recommendations
 */

#ifdef _AIX

#include <stddef.h>  /* For NULL */

/* ============================================================
 * CATEGORY 1: Authentication & User Management
 * Critical for: User access control, privilege escalation detection
 * ============================================================ */
static const char *aix_auth_files[] = {
    "/etc/passwd",
    "/etc/group",
    "/etc/pam.conf",
    "/etc/security/passwd",
    "/etc/security/user",
    "/etc/security/group",
    "/etc/security/limits",
    "/etc/security/login.cfg",
    "/etc/security/environ",
    "/etc/security/lastlog",
    "/etc/security/failedlogin",
    "/etc/security/mkuser.default",
    "/etc/security/mkuser.sys",
    "/etc/security/roles",
    "/etc/security/authorizations",
    "/etc/security/uattr",
    "/etc/security/user.roles",
    "/etc/security/priv",
    "/etc/security/privcmds",
    "/etc/security/privdevs",
    "/etc/security/privfiles",
    "/etc/security/pwdalg.cfg",
    "/etc/security/domains",
    "/etc/security/domobjs",
    "/etc/security/acl",
    "/etc/security/portlog",
    "/etc/security/services",
    "/etc/security/smitacl.user",
    "/etc/security/smitacl.group",
    NULL
};

/* ============================================================
 * CATEGORY 2: Audit Configuration
 * Critical for: Security event logging, compliance
 * ============================================================ */
static const char *aix_audit_files[] = {
    "/etc/security/audit/config",
    "/etc/security/audit/events",
    "/etc/security/audit/objects",
    "/etc/security/audit/bincmds",
    "/etc/security/audit/streamcmds",
    NULL
};

/* ============================================================
 * CATEGORY 3: Network Configuration
 * Critical for: Network security, service exposure
 * ============================================================ */
static const char *aix_network_files[] = {
    "/etc/hosts",
    "/etc/hosts.equiv",
    "/etc/hosts.allow",
    "/etc/hosts.deny",
    "/etc/resolv.conf",
    "/etc/netsvc.conf",
    "/etc/services",
    "/etc/protocols",
    "/etc/rpc",
    "/etc/inetd.conf",
    "/etc/netgroup",
    "/etc/networks",
    "/etc/rc.tcpip",
    "/etc/rc.net",
    "/etc/rc.bsdnet",
    "/etc/ftpusers",
    NULL
};

/* ============================================================
 * CATEGORY 4: SSH Configuration
 * Critical for: Remote access security
 * ============================================================ */
static const char *aix_ssh_files[] = {
    "/etc/ssh/sshd_config",
    "/etc/ssh/ssh_config",
    "/etc/ssh/moduli",
    "/etc/ssh/ssh_host_rsa_key.pub",
    "/etc/ssh/ssh_host_ecdsa_key.pub",
    "/etc/ssh/ssh_host_ed25519_key.pub",
    NULL
};

/* ============================================================
 * CATEGORY 5: System Boot & Initialization
 * Critical for: Startup integrity, persistent access detection
 * ============================================================ */
static const char *aix_boot_files[] = {
    "/etc/inittab",
    "/etc/rc",
    "/etc/rc.bootc",
    "/etc/rc.nfs",
    "/etc/rc.security.boot",
    "/etc/rc.shutdown",
    "/etc/rc.powerfail",
    "/etc/environment",
    "/etc/profile",
    "/etc/motd",
    "/etc/issue",
    "/etc/tsh_profile",
    "/etc/shells",
    NULL
};

/* ============================================================
 * CATEGORY 6: Scheduled Tasks (Cron)
 * Critical for: Unauthorized scheduled task detection
 * ============================================================ */
static const char *aix_cron_files[] = {
    "/etc/cronlog.conf",
    "/var/spool/cron/crontabs/root",
    "/var/spool/cron/crontabs/adm",
    "/var/spool/cron/crontabs/sys",
    "/var/spool/cron/crontabs/uucp",
    "/var/adm/cron/at.allow",
    "/var/adm/cron/at.deny",
    "/var/adm/cron/cron.allow",
    "/var/adm/cron/cron.deny",
    NULL
};

/* ============================================================
 * CATEGORY 7: Filesystem & Storage
 * Critical for: Mount point integrity, storage security
 * ============================================================ */
static const char *aix_fs_files[] = {
    "/etc/filesystems",
    "/etc/vfs",
    "/etc/swapspaces",
    "/etc/exports",
    NULL
};

/* ============================================================
 * CATEGORY 8: Logging Configuration
 * Critical for: Log integrity, audit trail
 * ============================================================ */
static const char *aix_log_files[] = {
    "/etc/syslog.conf",
    "/var/adm/ras/errlog",
    NULL
};

/* ============================================================
 * CATEGORY 9: System Tunables
 * Critical for: Kernel parameter integrity
 * ============================================================ */
static const char *aix_tunable_files[] = {
    "/etc/tunables/nextboot",
    "/etc/tunables/usermodified",
    NULL
};

/* ============================================================
 * CATEGORY 10: Privilege Escalation
 * Critical for: Sudo/su abuse detection
 * ============================================================ */
static const char *aix_sudo_files[] = {
    "/etc/sudoers",
    NULL
};

/* ============================================================
 * CATEGORY 11: LDAP & Directory Services
 * Critical for: Centralized authentication security
 * ============================================================ */
static const char *aix_ldap_files[] = {
    "/etc/security/ldap/ldap.cfg",
    "/etc/security/ldap/sectoldif.cfg",
    NULL
};

/* ============================================================
 * CATEGORY 12: NTP & Time
 * Critical for: Time synchronization (important for logs)
 * ============================================================ */
static const char *aix_ntp_files[] = {
    "/etc/ntp.conf",
    NULL
};

/* ============================================================
 * CATEGORY 13: Printing & Services
 * Critical for: Service configuration
 * ============================================================ */
static const char *aix_service_files[] = {
    "/etc/qconfig",
    "/etc/hosts.lpd",
    NULL
};

/* ============================================================
 * CATEGORY 14: Critical SUID/SGID Binaries - Authentication
 * Critical for: Privilege escalation attack detection
 * ============================================================ */
static const char *aix_suid_auth_bins[] = {
    "/usr/bin/passwd",
    "/usr/bin/su",
    "/usr/bin/login",
    "/usr/bin/newgrp",
    "/usr/bin/chuser",
    "/usr/bin/chgroup",
    "/usr/bin/mkuser",
    "/usr/bin/rmuser",
    "/usr/bin/chsec",
    "/usr/bin/pwdadm",
    "/usr/bin/pwdck",
    "/usr/bin/lssec",
    "/usr/bin/lsuser",
    "/usr/bin/crontab",
    "/usr/bin/at",
    "/usr/bin/atq",
    "/usr/bin/atrm",
    "/usr/bin/chsh",
    "/usr/bin/chfn",
    "/usr/bin/chrole",
    "/usr/bin/mkrole",
    "/usr/bin/logout",
    NULL
};

/* ============================================================
 * CATEGORY 15: Critical SUID/SGID Binaries - System
 * Critical for: System compromise detection
 * ============================================================ */
static const char *aix_suid_sys_bins[] = {
    "/usr/sbin/mount",
    "/usr/sbin/umount",
    "/usr/sbin/reboot",
    "/usr/sbin/shutdown",
    "/usr/sbin/cron",
    "/usr/sbin/inetd",
    "/usr/sbin/sshd",
    "/usr/sbin/login",
    "/usr/sbin/init",
    "/usr/sbin/srcmstr",
    "/usr/sbin/sendmail",
    "/usr/sbin/audit",
    "/usr/sbin/auditbin",
    "/usr/sbin/auditpr",
    "/usr/sbin/swap",
    "/usr/sbin/swapon",
    "/usr/sbin/swapoff",
    "/usr/sbin/ping",
    "/usr/sbin/traceroute",
    "/usr/sbin/netstat",
    "/usr/sbin/route",
    "/usr/sbin/arp",
    "/usr/sbin/mkdev",
    "/usr/sbin/rmdev",
    "/usr/sbin/lsdev",
    "/usr/sbin/cfgmgr",
    "/usr/sbin/lsattr",
    "/usr/sbin/chdev",
    NULL
};

/* ============================================================
 * CATEGORY 16: Critical SUID/SGID Binaries - LVM/Storage
 * Critical for: Storage manipulation detection
 * ============================================================ */
static const char *aix_suid_lvm_bins[] = {
    "/usr/sbin/lsvg",
    "/usr/sbin/lspv",
    "/usr/sbin/lslv",
    "/usr/sbin/mkvg",
    "/usr/sbin/mklv",
    "/usr/sbin/extendvg",
    "/usr/sbin/reducevg",
    "/usr/sbin/varyonvg",
    "/usr/sbin/lvaryonvg",
    "/usr/sbin/lvaryoffvg",
    NULL
};

/* ============================================================
 * CATEGORY 17: Critical System Libraries
 * Critical for: Library injection attack detection
 * ============================================================ */
static const char *aix_lib_files[] = {
    "/usr/lib/libc.a",
    "/usr/lib/libpthread.a",
    "/usr/lib/libcrypt.a",
    "/usr/lib/libodm.a",
    "/usr/lib/security/LDAP",
    "/usr/lib/security/KRB5",
    "/usr/lib/security/DCE",
    NULL
};

/* ============================================================
 * CATEGORY 18: Kernel & Boot
 * Critical for: Rootkit detection
 * ============================================================ */
static const char *aix_kernel_files[] = {
    "/unix",
    "/usr/lib/boot/unix_64",
    "/usr/lib/boot/unix_mp",
    NULL
};

/* ============================================================
 * CATEGORY 19: Security Certificates
 * Critical for: PKI/TLS integrity
 * ============================================================ */
static const char *aix_cert_files[] = {
    "/var/ssl/certs",
    "/etc/security/certificates",
    NULL
};

/* ============================================================
 * CATEGORY 20: ODM (Object Data Manager)
 * Critical for: System configuration database
 * ============================================================ */
static const char *aix_odm_files[] = {
    "/etc/objrepos/CuAt",
    "/etc/objrepos/CuDv",
    "/etc/objrepos/CuDep",
    "/etc/objrepos/CuVPD",
    "/etc/objrepos/Config_Rules",
    "/etc/objrepos/SRCsubsys",
    "/etc/objrepos/SRCsubsvr",
    NULL
};

/* ============================================================
 * Master list combining all categories
 * Total: ~150 files (expandable)
 * ============================================================ */

/* Count files in a NULL-terminated array */
static int count_file_list(const char **list) {
    int count = 0;
    while (list && list[count]) count++;
    return count;
}

/* Get total count of all AIX critical files */
int get_aix_critical_file_count(void) {
    int total = 0;
    total += count_file_list(aix_auth_files);
    total += count_file_list(aix_audit_files);
    total += count_file_list(aix_network_files);
    total += count_file_list(aix_ssh_files);
    total += count_file_list(aix_boot_files);
    total += count_file_list(aix_cron_files);
    total += count_file_list(aix_fs_files);
    total += count_file_list(aix_log_files);
    total += count_file_list(aix_tunable_files);
    total += count_file_list(aix_sudo_files);
    total += count_file_list(aix_ldap_files);
    total += count_file_list(aix_ntp_files);
    total += count_file_list(aix_service_files);
    total += count_file_list(aix_suid_auth_bins);
    total += count_file_list(aix_suid_sys_bins);
    total += count_file_list(aix_suid_lvm_bins);
    total += count_file_list(aix_lib_files);
    total += count_file_list(aix_kernel_files);
    total += count_file_list(aix_cert_files);
    total += count_file_list(aix_odm_files);
    return total;
}

/* Get the master list of all AIX critical files */
/* Returns array of category arrays for organized access */
typedef struct {
    const char *category_name;
    const char **files;
} aix_file_category_t;

static const aix_file_category_t aix_all_categories[] = {
    {"authentication", aix_auth_files},
    {"audit", aix_audit_files},
    {"network", aix_network_files},
    {"ssh", aix_ssh_files},
    {"boot", aix_boot_files},
    {"cron", aix_cron_files},
    {"filesystem", aix_fs_files},
    {"logging", aix_log_files},
    {"tunables", aix_tunable_files},
    {"sudo", aix_sudo_files},
    {"ldap", aix_ldap_files},
    {"ntp", aix_ntp_files},
    {"services", aix_service_files},
    {"suid_auth", aix_suid_auth_bins},
    {"suid_system", aix_suid_sys_bins},
    {"suid_lvm", aix_suid_lvm_bins},
    {"libraries", aix_lib_files},
    {"kernel", aix_kernel_files},
    {"certificates", aix_cert_files},
    {"odm", aix_odm_files},
    {NULL, NULL}
};

/* Get all categories */
const aix_file_category_t* get_aix_file_categories(void) {
    return aix_all_categories;
}

/* Build flat list of all files (caller provides buffer) */
int get_aix_critical_files(const char **out_files, int max_files) {
    int idx = 0;

    for (int cat = 0; aix_all_categories[cat].files != NULL; cat++) {
        const char **files = aix_all_categories[cat].files;
        for (int i = 0; files[i] != NULL && idx < max_files; i++) {
            out_files[idx++] = files[i];
        }
    }

    return idx;
}

#endif /* _AIX */
