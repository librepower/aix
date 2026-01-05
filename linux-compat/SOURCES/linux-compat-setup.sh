#!/usr/bin/ksh
#============================================================================
# linux-compat-setup.sh - GNU/Linux CLI Experience for AIX
#============================================================================
# Part of LibrePower - Open Source for IBM Power
# https://github.com/librepower/aix
#
# Copyright (C) 2026 SIXE - IBM Business Partner
# https://sixe.eu | hugo.blanco@sixe.eu
#
# License: GPL-3.0
#============================================================================

SCRIPT_VERSION="2.0"
PROFILE_SNIPPET="$HOME/.linux-compat-profile"
BACKUP_SUFFIX=".pre-linux-compat"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info()    { printf "${BLUE}[INFO]${NC} %s\n" "$1"; }
print_success() { printf "${GREEN}[OK]${NC} %s\n" "$1"; }
print_warning() { printf "${YELLOW}[WARN]${NC} %s\n" "$1"; }
print_error()   { printf "${RED}[ERROR]${NC} %s\n" "$1"; }

#----------------------------------------------------------------------------
# Check prerequisites
#----------------------------------------------------------------------------
check_prerequisites() {
    print_info "Checking prerequisites..."
    
    if [ "$(uname -s)" != "AIX" ]; then
        print_error "This script is designed for AIX only."
        exit 1
    fi
    
    if [ ! -d "/opt/freeware/bin" ]; then
        print_error "AIX Toolbox not found at /opt/freeware"
        print_error "Install AIX Toolbox first. See: https://www.ibm.com/support/pages/aix-toolbox-open-source-software"
        exit 1
    fi
    
    print_success "Prerequisites check passed"
}

#----------------------------------------------------------------------------
# Check required packages
#----------------------------------------------------------------------------
check_packages() {
    print_info "Checking required GNU packages..."
    echo ""
    
    local missing=""
    local installed=""
    
    # Core packages
    for pkg in ls grep sed awk find tar gzip xz vim diff jq tree tmux; do
        if [ -x "/opt/freeware/bin/$pkg" ]; then
            installed="$installed $pkg"
        else
            missing="$missing $pkg"
        fi
    done
    
    if [ -n "$missing" ]; then
        print_warning "Missing packages:$missing"
        echo ""
        echo "Install with DNF (recommended):"
        echo "  dnf install -y coreutils findutils grep sed gawk diffutils \\"
        echo "                 tar xz zip unzip vim-enhanced tmux jq tree"
        echo ""
        echo "Or with YUM:"
        echo "  yum install -y coreutils findutils grep sed gawk diffutils \\"
        echo "                 tar xz zip unzip vim-enhanced tmux jq tree"
        echo ""
        echo "Or download individually from:"
        echo "  https://public.dhe.ibm.com/aix/freeSoftware/aixtoolbox/RPMS/ppc/"
        echo ""
        return 1
    else
        print_success "All required packages installed"
        return 0
    fi
}

#----------------------------------------------------------------------------
# Generate the profile snippet
#----------------------------------------------------------------------------
generate_profile_snippet() {
    cat > "$PROFILE_SNIPPET" << 'PROFILE_EOF'
#============================================================================
# Linux Compatibility Profile for AIX
# Part of LibrePower - https://github.com/librepower/aix
# Version: 2.1
#============================================================================

# Safety: Only run for interactive shells
case $- in
    *i*) ;;
    *)   return 0 ;;
esac

# Allow disable via environment variable
[ -n "$LINUX_COMPAT_DISABLE" ] && return 0

# Prevent double-source
[ -n "$_LINUX_COMPAT_LOADED" ] && return 0
export _LINUX_COMPAT_LOADED=1

#----------------------------------------------------------------------------
# PATH Configuration
#----------------------------------------------------------------------------
case ":$PATH:" in
    *:/opt/freeware/bin:*) ;;
    *) export PATH="/opt/freeware/bin:$PATH" ;;
esac

case ":$LIBPATH:" in
    *:/opt/freeware/lib:*) ;;
    *) export LIBPATH="/opt/freeware/lib:${LIBPATH:-/usr/lib}" ;;
esac

#----------------------------------------------------------------------------
# Aliases - Native GNU tools with Linux-style options
#----------------------------------------------------------------------------
# Directory listing (NATIVE GNU)
alias ls='/opt/freeware/bin/ls --color=auto'
alias ll='/opt/freeware/bin/ls -lh --color=auto'
alias la='/opt/freeware/bin/ls -lha --color=auto'
alias l='/opt/freeware/bin/ls -CF --color=auto'
alias lt='/opt/freeware/bin/ls -lht --color=auto'
alias lS='/opt/freeware/bin/ls -lhS --color=auto'

# Text processing (NATIVE GNU)
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

# Disk usage (NATIVE GNU)
alias df='/opt/freeware/bin/df -h'
alias du='/opt/freeware/bin/du -h'
alias dus='/opt/freeware/bin/du -sh'
alias dust='/opt/freeware/bin/du -sh *'

# Process (using AIX ps with GNU grep)
alias psg='ps -ef | /opt/freeware/bin/grep -v grep | /opt/freeware/bin/grep'
alias psu='ps -ef | head -1; ps -ef | /opt/freeware/bin/grep'

# History
alias h='history'
alias hg='history | /opt/freeware/bin/grep'

# Editor (NATIVE GNU)
alias vi='/opt/freeware/bin/vim'
alias vim='/opt/freeware/bin/vim'

# Tree (NATIVE GNU)
alias tree='/opt/freeware/bin/tree --charset=ASCII'

# Misc
alias cls='clear'
alias path='echo $PATH | tr ":" "\n" | nl'
alias now='date "+%Y-%m-%d %H:%M:%S"'

#----------------------------------------------------------------------------
# AIX-specific aliases (translations for Linux sysadmins)
#----------------------------------------------------------------------------
alias services='lssrc -a'                    # Like: systemctl list-units
alias svc='lssrc -s'                         # Like: systemctl status
alias top='topas'                            # AIX process monitor
alias free='svmon -G -O unit=MB'             # Like: free -m (EMULATED)
alias lsblk='lsdev -Cc disk'                 # List disks (EMULATED)
alias lscpu='prtconf | /opt/freeware/bin/grep -E "Processor|CPU|MHz"'
alias lsmem='prtconf | /opt/freeware/bin/grep -E "Memory|Good"'

#----------------------------------------------------------------------------
# Functions - EMULATED Linux commands
#----------------------------------------------------------------------------

# Extract any archive (uses NATIVE GNU tar/gzip/xz)
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

# Find file by name (uses NATIVE GNU find)
ff() {
    /opt/freeware/bin/find . -iname "*$1*" 2>/dev/null
}

# Find in files (uses NATIVE GNU grep)
fif() {
    /opt/freeware/bin/grep -rn --color=auto "$1" . 2>/dev/null
}

# watch replacement (EMULATED - runs command every N seconds)
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

# pgrep replacement (EMULATED)
pgrep() {
    ps -ef | /opt/freeware/bin/grep -v grep | /opt/freeware/bin/grep "$1" | /opt/freeware/bin/awk '{print $2}'
}

# pkill replacement (EMULATED)
pkill() {
    local pids=$(ps -ef | /opt/freeware/bin/grep -v grep | /opt/freeware/bin/grep "$1" | /opt/freeware/bin/awk '{print $2}')
    if [ -n "$pids" ]; then
        echo "Killing PIDs: $pids"
        echo $pids | xargs kill
    else
        echo "No matching processes"
    fi
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

# Service management helpers (WRAPPERS for AIX SRC)
svcstart()   { startsrc -s "$1"; }
svcstop()    { stopsrc -s "$1"; }
svcrestart() { stopsrc -s "$1"; sleep 1; startsrc -s "$1"; }
svcstatus()  { lssrc -s "$1"; }

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

# Quick help
linuxhelp() {
    cat << 'HELPEOF'
╔═══════════════════════════════════════════════════════════════════════════╗
║              Linux Compatibility for AIX - Quick Reference                ║
╠═══════════════════════════════════════════════════════════════════════════╣
║                                                                           ║
║  ✅ NATIVE GNU TOOLS (real GNU binaries from IBM AIX Toolbox):            ║
║     ls, grep, sed, awk, find, tar, diff, vim, jq, tree, tmux              ║
║     → All GNU options work: ls -lh, grep -rP, find -not, sed -i, etc.     ║
║                                                                           ║
║  ⚡ EMULATED FUNCTIONS (shell functions using AIX commands):              ║
║     watch <cmd>     - Repeat command (uses while/sleep loop)              ║
║     pgrep/pkill     - Find/kill by name (uses ps -ef | grep)              ║
║     free            - Memory usage (uses svmon)                           ║
║     svcstart/stop   - Services (uses startsrc/stopsrc)                    ║
║                                                                           ║
║  ALIASES:                                                                 ║
║     ll, la, lt, lS  - ls variants        psg - ps | grep                  ║
║     df, du, dust    - disk usage         services - list services         ║
║                                                                           ║
║  FUNCTIONS:                                                               ║
║     extract <file>  - Auto-extract       mkcd <dir> - mkdir + cd          ║
║     ff <name>       - Find files         fif <text> - Find in files       ║
║     sysinfo         - System summary     showpath - Show PATH             ║
║                                                                           ║
║  NATIVE AIX COMMANDS:  /usr/bin/ls, /usr/bin/ps, etc.                     ║
║  DISABLE TEMPORARILY:  export LINUX_COMPAT_DISABLE=1; exec $SHELL         ║
║                                                                           ║
╚═══════════════════════════════════════════════════════════════════════════╝
HELPEOF
}

#----------------------------------------------------------------------------
# Welcome message
#----------------------------------------------------------------------------
if [ -z "$_LINUX_COMPAT_WELCOMED" ]; then
    export _LINUX_COMPAT_WELCOMED=1
    printf '\033[0;34m[Linux Compat]\033[0m GNU tools active. Type \033[1mlinuxhelp\033[0m for reference.\n'
fi

# End of Linux Compatibility Profile
PROFILE_EOF
}

#----------------------------------------------------------------------------
# Install
#----------------------------------------------------------------------------
do_install() {
    print_info "Installing Linux Compatibility Profile v${SCRIPT_VERSION}..."
    
    # Check packages first
    check_packages
    
    # Generate the profile snippet
    generate_profile_snippet
    print_success "Created $PROFILE_SNIPPET"
    
    # Determine which profile to modify
    local profile_file="$HOME/.profile"
    
    # Check if already installed
    if grep -q "linux-compat-profile" "$profile_file" 2>/dev/null; then
        print_warning "Already installed in $profile_file"
        print_info "Updating profile snippet..."
        generate_profile_snippet
        print_success "Profile updated. Run: source $PROFILE_SNIPPET"
        return 0
    fi
    
    # Backup original
    if [ -f "$profile_file" ]; then
        cp "$profile_file" "${profile_file}${BACKUP_SUFFIX}"
        print_success "Backed up to ${profile_file}${BACKUP_SUFFIX}"
    fi
    
    # Add sourcing line to profile
    cat >> "$profile_file" << EOF

# Linux Compatibility Profile - Added by linux-compat-setup.sh
# Part of LibrePower - https://github.com/librepower/aix
if [ -f "$PROFILE_SNIPPET" ]; then
    . "$PROFILE_SNIPPET"
fi
EOF
    
    print_success "Added to $profile_file"
    
    echo ""
    print_success "Installation complete!"
    echo ""
    print_info "To activate now, run:"
    echo "    source $PROFILE_SNIPPET"
    echo ""
    print_info "Or start a new shell session."
    echo ""
    print_info "Type 'linuxhelp' for quick reference after activation."
}

#----------------------------------------------------------------------------
# Uninstall
#----------------------------------------------------------------------------
do_uninstall() {
    print_info "Uninstalling Linux Compatibility Profile..."
    
    # Remove profile snippet
    if [ -f "$PROFILE_SNIPPET" ]; then
        rm -f "$PROFILE_SNIPPET"
        print_success "Removed $PROFILE_SNIPPET"
    fi
    
    # Clean up .profile
    if [ -f "$HOME/.profile" ]; then
        if grep -q "linux-compat-profile" "$HOME/.profile" 2>/dev/null; then
            grep -v "linux-compat" "$HOME/.profile" > "$HOME/.profile.tmp"
            mv "$HOME/.profile.tmp" "$HOME/.profile"
            print_success "Cleaned $HOME/.profile"
        fi
    fi
    
    echo ""
    print_success "Uninstallation complete!"
    print_info "Start a new shell session for changes to take effect."
    
    if [ -f "$HOME/.profile${BACKUP_SUFFIX}" ]; then
        print_info "Your backup is preserved at: $HOME/.profile${BACKUP_SUFFIX}"
    fi
}

#----------------------------------------------------------------------------
# Status
#----------------------------------------------------------------------------
do_status() {
    echo ""
    echo "═══════════════════════════════════════════════════════════════"
    echo " Linux Compatibility Status (v${SCRIPT_VERSION})"
    echo "═══════════════════════════════════════════════════════════════"
    echo ""
    
    # Profile status
    if [ -f "$PROFILE_SNIPPET" ]; then
        print_success "Profile snippet: Installed"
    else
        print_warning "Profile snippet: Not installed"
    fi
    
    if grep -q "linux-compat" "$HOME/.profile" 2>/dev/null; then
        print_success "Configured in: $HOME/.profile"
    else
        print_warning "Not configured in .profile"
    fi
    
    if [ -n "$_LINUX_COMPAT_LOADED" ]; then
        print_success "Current session: Active"
    else
        print_warning "Current session: Not active (source profile or restart shell)"
    fi
    
    echo ""
    echo "═══════════════════════════════════════════════════════════════"
    echo " GNU Packages Status"
    echo "═══════════════════════════════════════════════════════════════"
    echo ""
    echo " ✅ NATIVE GNU (from IBM AIX Toolbox):"
    for cmd in ls grep sed awk find tar diff vim jq tree tmux xz; do
        if [ -x "/opt/freeware/bin/$cmd" ]; then
            printf "    ✓ %-10s\n" "$cmd"
        else
            printf "    ✗ %-10s (not installed)\n" "$cmd"
        fi
    done
    
    echo ""
    echo " ⚡ EMULATED (shell functions):"
    echo "    • watch, pgrep, pkill, free, svcstart/stop"
    echo ""
}

#----------------------------------------------------------------------------
# Usage
#----------------------------------------------------------------------------
show_usage() {
    cat << EOF
Linux Compatibility Setup for AIX v${SCRIPT_VERSION}
Part of LibrePower - https://github.com/librepower/aix

USAGE:
    $0 [COMMAND]

COMMANDS:
    install     Install Linux compatibility profile (default)
    uninstall   Remove Linux compatibility profile
    status      Show installation status and package info
    help        Show this help message

PREREQUISITES:
    Install required packages with DNF:
    dnf install -y coreutils findutils grep sed gawk diffutils \\
                   tar xz zip unzip vim-enhanced tmux jq tree

SAFETY:
    • Only affects interactive shells
    • Scripts using #!/bin/sh are NOT affected
    • Native AIX commands available via /usr/bin/
    • Easily reversible with uninstall

MORE INFO:
    https://github.com/librepower/aix/tree/main/linux-compat

EOF
}

#----------------------------------------------------------------------------
# Main
#----------------------------------------------------------------------------

# Ensure HOME is set correctly
[ -z "$HOME" ] && export HOME=$(eval echo ~$(whoami))

case "${1:-install}" in
    install|--install|-i)
        check_prerequisites
        do_install
        ;;
    uninstall|--uninstall|-u|remove)
        do_uninstall
        ;;
    status|--status|-s)
        do_status
        ;;
    help|--help|-h)
        show_usage
        ;;
    *)
        print_error "Unknown option: $1"
        show_usage
        exit 1
        ;;
esac
