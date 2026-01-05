# Linux Compatibility for AIX

![AIX 7.2](https://img.shields.io/badge/AIX-7.2+-blue)
![noarch](https://img.shields.io/badge/arch-noarch-lightgrey)
![License](https://img.shields.io/badge/license-GPL--3.0-green)

**A configuration layer for IBM's GNU tools in AIX Toolbox**

Provides a familiar GNU/Linux command-line experience for AIX administrators coming from Linux environments.

## Why linux-compat?

**Because muscle memory matters.**

You've spent years typing `ls -lh`, `grep -rP`, `systemctl status`. Now you're on AIX and nothing works the same way. The tools exist (IBM ported them!), but they're not in your PATH and AIX services use different commands.

| Linux muscle memory | AIX native | With linux-compat |
|---------------------|------------|-------------------|
| `ls -lh --color` | `ls -l` (no -h, no color) | ✅ Works |
| `grep -rP "regex"` | `/opt/freeware/bin/grep...` | ✅ Works |
| `systemctl status sshd` | `lssrc -s sshd` | ✅ Works |
| `watch -n 5 command` | ❌ Doesn't exist | ✅ Works |

## What This Package Does

Let's be clear about what this is and isn't:

**We don't compile any of these tools.** IBM did that work, and they did it well. The GNU coreutils, grep, sed, awk, and dozens of other tools are already available in [IBM AIX Toolbox](https://www.ibm.com/support/pages/aix-toolbox-open-source-software-overview).

**What IBM provides** (installed as dependencies):
- GNU coreutils (ls, cp, mv, cat, head, tail, wc, sort, etc.)
- GNU grep, sed, awk, find, diff
- GNU tar with gzip/bzip2/xz support
- vim, tmux, jq, tree, and more

**What we add:**
- A shell profile that puts GNU tools first in your PATH
- Aliases for common operations (`ll`, `la`, `lt`, etc.)
- Emulated commands that don't exist on AIX (`watch`, `pgrep`, `pkill`, `free`)
- `systemctl` and `service` wrappers for AIX SRC (System Resource Controller)
- Bilingual documentation

Credit where it's due: **IBM did the hard work of porting these tools. We just make them easier to use by default.**

## Installation

### Option 1: dnf (Recommended)

```bash
# Add LibrePower repository (one time)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install
dnf install linux-compat

# Activate for your user
linux-compat-setup install
source ~/.linux-compat-profile
```

📦 Repository details: https://aix.librepower.org/

### Option 2: curl (if dnf/yum not available)

```bash
# Download RPM
curl -LO https://github.com/librepower/aix/releases/download/linux-compat-v2.1/linux-compat-2.1-1.librepower.aix7.3.noarch.rpm

# Install (dnf resolves dependencies automatically)
dnf install ./linux-compat-2.1-1.librepower.aix7.3.noarch.rpm

# Activate for your user
linux-compat-setup install
source ~/.linux-compat-profile
```

## What You Get

### Native GNU Tools (from IBM AIX Toolbox)

These are real binaries, not emulations:

```bash
ls -lh --color=auto    # GNU ls with human sizes and colors
grep -rP "pattern"     # Recursive grep with Perl regex
find . -name "*.log" -mtime -1   # GNU find with all options
sed -i 's/old/new/g'   # In-place editing
tar -xzf archive.tar.gz   # Direct extraction
```

### Service Management (systemctl/service wrappers)

AIX uses SRC (System Resource Controller) instead of systemd. We provide familiar wrappers:

```bash
# systemctl style (recommended)
systemctl status              # List all services (lssrc -a)
systemctl start sshd          # Start service (startsrc -s sshd)
systemctl stop sshd           # Stop service (stopsrc -s sshd)
systemctl restart sshd        # Restart service
systemctl reload sshd         # Reload config (refresh -s sshd)
systemctl is-active sshd      # Check if running

# Group operations (use @ prefix)
systemctl start @tcpip        # Start all TCP/IP services
systemctl stop @nfs           # Stop all NFS services
systemctl status @tcpip       # Status of TCP/IP group
systemctl list-groups         # Show available groups

# SysV style alternative
service sshd start
service sshd stop
service sshd status

# Quick aliases
services                      # List all (lssrc -a)
services-active               # Only active services
services-down                 # Only stopped services
```

**Note:** `enable`/`disable` are not available—AIX uses `/etc/rc.tcpip`, `/etc/inittab` for boot configuration.

### Emulated Commands

These don't exist natively on AIX, so we provide shell function equivalents:

```bash
watch -n 5 'ps -ef | wc -l'   # Repeat command every 5 seconds
pgrep java                     # Find PIDs by process name
pkill -9 zombie                # Kill processes by name
free                           # Memory usage (uses svmon)
```

### Convenient Aliases

```bash
# Directory listing
ll                  # ls -lh with colors
la                  # ls -lha (show hidden)
lt                  # ls -lht (sort by time)
lS                  # ls -lhS (sort by size)

# Navigation
..                  # cd ..
...                 # cd ../..
mkcd dirname        # mkdir + cd

# Search
ff pattern          # Find files by name
fif text            # Find in files (grep -rn)
psg process         # ps -ef | grep

# System
sysinfo             # Quick system summary
top                 # Opens topas
df                  # Disk free (human readable)
path                # Show PATH entries numbered
```

## Safety Guarantees

1. **Interactive shells only** - Scripts using `#!/bin/sh` or `#!/bin/ksh` are NOT affected
2. **Native commands preserved** - `/usr/bin/ls`, `/usr/bin/ps`, etc. always work
3. **Instantly reversible** - `linux-compat-setup uninstall` removes everything
4. **No system modifications** - Only touches user's home directory

### How It Works

The profile is sourced from `~/.profile` and checks `$-` for interactive mode:

```bash
case $- in
    *i*) ;;      # Interactive - apply settings
    *)   return 0 ;;  # Non-interactive - do nothing
esac
```

## Managing the Installation

```bash
# Check current status
linux-compat-setup status

# Temporarily disable (current session)
export LINUX_COMPAT_DISABLE=1
exec $SHELL

# Completely uninstall
linux-compat-setup uninstall

# Reinstall
linux-compat-setup install
```

## Quick Reference

After activation, type `linuxhelp` for a quick reference card.

## Documentation

- **[INSTALL.txt](INSTALL.txt)** - Full English documentation
- **[INSTALL_ES.txt](INSTALL_ES.txt)** - Documentación completa en español

## Requirements

- AIX 7.2 or later
- DNF/YUM configured with AIX Toolbox repository
- bash (included in dependencies)

## Acknowledgments

- **IBM** for porting GNU tools to AIX and maintaining AIX Toolbox
- **GNU Project** for the excellent command-line tools
- **AIX community** for continued platform support

## License

GPL-3.0 - See [LICENSE](../LICENSE)

---

*Part of [LibrePower](https://librepower.org) - Unlocking Power Systems through open source 🌍*

*Maintained by [SIXE](https://sixe.eu) - IBM Business Partner*
