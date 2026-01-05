#============================================================================
# Linux Compatibility Profile for AIX
# Part of LibrePower - https://github.com/librepower/aix
# Version: 2.1
#============================================================================
# This profile makes IBM AIX Toolbox GNU tools the default for interactive
# shells, providing a familiar Linux/macOS experience on AIX.
#
# Credit: IBM did the hard work of porting GNU tools. We just configure them.
#============================================================================

# Safety: Only run for interactive shells
case $- in
    *i*) ;;
    *)   return 0 ;;
esac

# Allow disable
[ -n "$LINUX_COMPAT_DISABLE" ] && return 0

# Prevent double-source
[ -n "$_LINUX_COMPAT_LOADED" ] && return 0
export _LINUX_COMPAT_LOADED=1

#============================================================================
# PATH Configuration - Put GNU tools first
#============================================================================
case ":$PATH:" in
    *:/opt/freeware/bin:*) ;;
    *) export PATH="/opt/freeware/bin:$PATH" ;;
esac

case ":$LIBPATH:" in
    *:/opt/freeware/lib:*) ;;
    *) export LIBPATH="/opt/freeware/lib:${LIBPATH:-/usr/lib}" ;;
esac

#============================================================================
# NATIVE GNU TOOLS - Aliases for GNU options (from IBM AIX Toolbox)
#============================================================================

# Directory listing (GNU ls)
alias ls='/opt/freeware/bin/ls --color=auto'
alias ll='/opt/freeware/bin/ls -lh --color=auto'
alias la='/opt/freeware/bin/ls -lha --color=auto'
alias l='/opt/freeware/bin/ls -CF --color=auto'
alias lt='/opt/freeware/bin/ls -lht --color=auto'    # Sort by time
alias lS='/opt/freeware/bin/ls -lhS --color=auto'    # Sort by size

# Grep with colors (GNU grep)
alias grep='/opt/freeware/bin/grep --color=auto'
alias egrep='/opt/freeware/bin/egrep --color=auto'
alias fgrep='/opt/freeware/bin/fgrep --color=auto'

# Safe file operations
alias cp='cp -i'
alias mv='mv -i'
alias rm='rm -i'

# Directory navigation
alias ..='cd ..'
alias ...='cd ../..'
alias ....='cd ../../..'
alias -- -='cd -'

# Disk usage - human readable (GNU coreutils)
alias df='/opt/freeware/bin/df -h'
alias du='/opt/freeware/bin/du -h'
alias dus='/opt/freeware/bin/du -sh'     # Summary
alias dust='/opt/freeware/bin/du -sh *'  # Summary of current dir items

# Process
alias psg='ps -ef | /opt/freeware/bin/grep -v grep | /opt/freeware/bin/grep'
alias psu='ps -ef | head -1; ps -ef | /opt/freeware/bin/grep'

# History
alias h='history'
alias hg='history | /opt/freeware/bin/grep'

# Editor (GNU vim)
alias vi='/opt/freeware/bin/vim'
alias vim='/opt/freeware/bin/vim'

# Tree
alias tree='/opt/freeware/bin/tree --charset=ASCII'

# Misc
alias cls='clear'
alias path='echo $PATH | tr ":" "\n" | nl'
alias now='date "+%Y-%m-%d %H:%M:%S"'
alias week='date +%V'

#============================================================================
# AIX TRANSLATIONS - Native AIX commands with familiar names
#============================================================================
alias top='topas'                            # Process monitor
alias lsblk='lsdev -Cc disk'                 # List block devices
alias lscpu='prtconf | /opt/freeware/bin/grep -E "Processor|CPU|MHz"'
alias lsmem='prtconf | /opt/freeware/bin/grep -E "Memory|Good"'

#============================================================================
# EMULATED COMMANDS - Linux commands reimplemented for AIX
#============================================================================

# free - Display memory usage (like free -m)
free() {
    echo "Unit: MB"
    svmon -G -O unit=MB
}

# watch - Execute command periodically (like watch)
watch() {
    local interval=2
    if [ "$1" = "-n" ]; then
        interval=$2
        shift 2
    fi
    while true; do
        clear
        echo "Every ${interval}s: $@    $(date)"
        echo "---"
        eval "$@"
        sleep $interval
    done
}

# pgrep - Find PIDs by process name
pgrep() {
    if [ -z "$1" ]; then
        echo "Usage: pgrep <pattern>"
        return 1
    fi
    ps -ef | /opt/freeware/bin/grep -v grep | /opt/freeware/bin/grep "$1" | /opt/freeware/bin/awk '{print $2}'
}

# pkill - Kill processes by name
pkill() {
    if [ -z "$1" ]; then
        echo "Usage: pkill <pattern>"
        return 1
    fi
    local pids=$(ps -ef | /opt/freeware/bin/grep -v grep | /opt/freeware/bin/grep "$1" | /opt/freeware/bin/awk '{print $2}')
    if [ -n "$pids" ]; then
        echo "Killing PIDs: $pids"
        echo $pids | xargs kill
    else
        echo "No matching processes found"
    fi
}

#============================================================================
# SERVICE MANAGEMENT - systemctl/service wrappers for AIX SRC
#============================================================================
# AIX uses SRC (System Resource Controller) instead of systemd
# - Subsystems: individual services (-s)
# - Groups: collections of related services (-g)
# - No enable/disable (boot config in /etc/rc.tcpip, inittab, etc.)
#============================================================================

# systemctl - Primary wrapper for systemd users
# Usage: systemctl <command> <service|@group>
# Use @ prefix for groups: systemctl start @tcpip
systemctl() {
    local cmd="$1"
    local target="$2"
    local flag="-s"
    local name="$target"
    
    if [ -z "$cmd" ]; then
        cat << 'SYSHELP'
Usage: systemctl <command> <service|@group>

Commands:
  start <name>      Start a service or @group
  stop <name>       Stop a service or @group  
  restart <name>    Restart a service or @group
  reload <name>     Reload config (uses 'refresh')
  status [name]     Show status (all if no name)
  kill <name>       Force stop (stopsrc -f)

  list-units        List all services
  list-groups       List service groups
  is-active <name>  Check if active

Groups: Use @ prefix, e.g., systemctl start @tcpip

Note: enable/disable not available in AIX SRC
      Boot config: /etc/rc.tcpip, /etc/inittab
      Native SRC: startsrc, stopsrc, lssrc, refresh
SYSHELP
        return 0
    fi
    
    # Detect group (prefix with @)
    if [ -n "$target" ]; then
        case "$target" in
            @*)
                flag="-g"
                name="${target#@}"
                ;;
        esac
    fi
    
    case "$cmd" in
        start)
            [ -z "$target" ] && echo "Usage: systemctl start <service|@group>" && return 1
            startsrc $flag "$name"
            ;;
        stop)
            [ -z "$target" ] && echo "Usage: systemctl stop <service|@group>" && return 1
            stopsrc $flag "$name"
            ;;
        restart)
            [ -z "$target" ] && echo "Usage: systemctl restart <service|@group>" && return 1
            stopsrc $flag "$name" 2>/dev/null
            sleep 1
            startsrc $flag "$name"
            ;;
        reload|refresh)
            [ -z "$target" ] && echo "Usage: systemctl reload <service>" && return 1
            if [ "$flag" = "-g" ]; then
                echo "Error: reload only works with individual services, not groups"
                return 1
            fi
            refresh -s "$name"
            ;;
        status)
            if [ -z "$target" ]; then
                lssrc -a
            else
                lssrc $flag "$name"
            fi
            ;;
        kill)
            [ -z "$target" ] && echo "Usage: systemctl kill <service|@group>" && return 1
            stopsrc -f $flag "$name"
            ;;
        is-active)
            [ -z "$target" ] && echo "Usage: systemctl is-active <service>" && return 1
            lssrc -s "$name" 2>/dev/null | /opt/freeware/bin/grep -q "active" && echo "active" || echo "inactive"
            ;;
        list-units|list)
            lssrc -a
            ;;
        list-groups)
            echo "Service groups:"
            lssrc -a | /opt/freeware/bin/awk 'NR>1 && $2!="" {print $2}' | sort -u | /opt/freeware/bin/grep -v "^[0-9]"
            ;;
        enable|disable)
            echo "Note: enable/disable not available in AIX SRC"
            echo "Boot configuration:"
            echo "  /etc/rc.tcpip     - TCP/IP services"  
            echo "  /etc/inittab      - Init services"
            echo "  /etc/rc.d/        - RC scripts"
            return 1
            ;;
        *)
            echo "Unknown command: $cmd (run 'systemctl' for help)"
            return 1
            ;;
    esac
}

# service - SysV init style wrapper (simpler syntax)
# Usage: service <name> <start|stop|restart|reload|status>
service() {
    local name="$1"
    local cmd="$2"
    
    if [ -z "$name" ] || [ -z "$cmd" ]; then
        echo "Usage: service <name> <start|stop|restart|reload|status>"
        echo "For groups: systemctl start @groupname"
        return 1
    fi
    
    case "$cmd" in
        start)   startsrc -s "$name" ;;
        stop)    stopsrc -s "$name" ;;
        restart) stopsrc -s "$name" 2>/dev/null; sleep 1; startsrc -s "$name" ;;
        reload)  refresh -s "$name" ;;
        status)  lssrc -s "$name" ;;
        *)
            echo "Unknown: $cmd. Use: start|stop|restart|reload|status"
            return 1
            ;;
    esac
}

# Quick service aliases
alias services='lssrc -a'
alias services-active='lssrc -a | /opt/freeware/bin/grep active'
alias services-down='lssrc -a | /opt/freeware/bin/grep inoperative'

#============================================================================
# UTILITY FUNCTIONS
#============================================================================

# Extract any archive format
extract() {
    if [ -z "$1" ]; then
        echo "Usage: extract <archive>"
        return 1
    fi
    if [ ! -f "$1" ]; then
        echo "Error: '$1' is not a file"
        return 1
    fi
    case "$1" in
        *.tar.gz|*.tgz)     /opt/freeware/bin/tar -xzf "$1" ;;
        *.tar.bz2|*.tbz2)   /opt/freeware/bin/tar -xjf "$1" ;;
        *.tar.xz|*.txz)     /opt/freeware/bin/tar -xJf "$1" ;;
        *.tar)              /opt/freeware/bin/tar -xf "$1" ;;
        *.gz)               /opt/freeware/bin/gzip -d "$1" ;;
        *.bz2)              /opt/freeware/bin/bzip2 -d "$1" ;;
        *.xz)               /opt/freeware/bin/xz -d "$1" ;;
        *.zip)              /opt/freeware/bin/unzip "$1" ;;
        *.Z)                uncompress "$1" ;;
        *)                  echo "Unknown archive format: '$1'" ;;
    esac
}

# Make directory and cd into it
mkcd() {
    mkdir -p "$1" && cd "$1"
}

# Find file by name
ff() {
    /opt/freeware/bin/find . -iname "*$1*" 2>/dev/null
}

# Find in files (grep recursively)
fif() {
    /opt/freeware/bin/grep -rn --color=auto "$1" . 2>/dev/null
}

# Quick system info
sysinfo() {
    echo "═══════════════════════════════════════════"
    echo " AIX System Information"
    echo "═══════════════════════════════════════════"
    echo " Hostname:  $(hostname)"
    echo " OS:        AIX $(oslevel -s)"
    echo " Kernel:    $(uname -v).$(uname -r)"
    echo " Arch:      $(uname -p)"
    echo " Uptime:    $(uptime | /opt/freeware/bin/sed 's/.*up //' | /opt/freeware/bin/sed 's/,.*//')"
    echo " CPU:       $(prtconf 2>/dev/null | /opt/freeware/bin/grep 'Number Of Processors' | /opt/freeware/bin/awk '{print $NF}') cores"
    echo " Memory:    $(prtconf 2>/dev/null | /opt/freeware/bin/grep 'Memory Size' | /opt/freeware/bin/awk '{print $3, $4}')"
    echo "═══════════════════════════════════════════"
}

# Show PATH entries
showpath() {
    echo "$PATH" | tr ':' '\n' | nl
}

# Colorize man pages
man() {
    LESS_TERMCAP_md=$'\e[1;36m' \
    LESS_TERMCAP_me=$'\e[0m' \
    LESS_TERMCAP_us=$'\e[1;32m' \
    LESS_TERMCAP_ue=$'\e[0m' \
    command man "$@"
}

#============================================================================
# HELP
#============================================================================
linuxhelp() {
    cat << 'HELPEOF'
╔═══════════════════════════════════════════════════════════════════════════╗
║              Linux Compatibility for AIX - Quick Reference                ║
╠═══════════════════════════════════════════════════════════════════════════╣
║                                                                           ║
║  ✅ NATIVE GNU (from IBM AIX Toolbox - real binaries):                    ║
║     ls -lh, grep -rP, find -not, sed -i, tar -xzf, vim, jq, tmux...       ║
║                                                                           ║
║  ⚡ EMULATED (shell functions):                                           ║
║     watch <cmd>    - repeat command      free    - memory (svmon)         ║
║     pgrep <name>   - find PIDs           pkill   - kill by name           ║
║                                                                           ║
║  🔧 SERVICE MANAGEMENT (wrappers for AIX SRC):                            ║
║     systemctl status              - list all services                     ║
║     systemctl start sshd          - start service                         ║
║     systemctl stop sshd           - stop service                          ║
║     systemctl restart sshd        - restart service                       ║
║     systemctl reload sshd         - reload config                         ║
║     systemctl start @tcpip        - start group (@ prefix)                ║
║     systemctl list-groups         - show available groups                 ║
║                                                                           ║
║     service sshd start            - alternative syntax                    ║
║     services                      - alias for lssrc -a                    ║
║                                                                           ║
║  📁 FILES & SEARCH:                                                       ║
║     ll, la, lt     - ls variants     ff <name>    - find files            ║
║     extract <file> - any archive     fif <text>   - grep in files         ║
║                                                                           ║
║  💻 SYSTEM:                                                               ║
║     sysinfo   - quick summary        top     - topas                      ║
║     df, du    - disk (human)         lsblk   - disk devices               ║
║                                                                           ║
║  NATIVE AIX: /usr/bin/ls, /usr/bin/ps, startsrc, stopsrc, lssrc           ║
║  DISABLE:    export LINUX_COMPAT_DISABLE=1; exec $SHELL                   ║
╚═══════════════════════════════════════════════════════════════════════════╝
HELPEOF
}

#============================================================================
# WELCOME MESSAGE
#============================================================================
if [ -z "$_LINUX_COMPAT_WELCOMED" ]; then
    export _LINUX_COMPAT_WELCOMED=1
    printf '\033[0;34m[Linux Compat]\033[0m GNU tools active. Type \033[1mlinuxhelp\033[0m for reference.\n'
fi

# End of Linux Compatibility Profile
