# fzf - Command-Line Fuzzy Finder for AIX

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![Go](https://img.shields.io/badge/Go-1.21.6-00ADD8)
![License](https://img.shields.io/badge/license-MIT-green)

The popular fuzzy finder, now available for AIX. Fast, portable, and incredibly useful for any sysadmin workflow.

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Launching February 2026

---

## Demo

![fzf demo on AIX](fzf.gif)

## Why fzf?

fzf transforms how you work with lists in the terminal. Search through files, command history, processes, packages - anything.

**Instead of:**
```bash
ps -ef | grep -i oracle | grep -v grep
```

**Just do:**
```bash
ps -ef | fzf
```

Type "oracle", watch results filter in real-time, press Enter. Done.

### Performance on AIX

| Items | Search Time |
|-------|-------------|
| 22,000 files | 0.28 seconds |
| 100,000 lines | 0.15 seconds |
| 500,000 lines | 0.71 seconds |

## Installation

### Option 1: dnf (Recommended)

Add the LibrePower repository and install with one command:

```bash
# Add repository (one-time setup)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install
dnf install fzf
```

📦 Repository details: https://aix.librepower.org/

### Option 2: curl (if dnf/yum not available)

```bash
cd /tmp

curl -L -o fzf-0.46.1-1.librepower.aix7.3.ppc.rpm \
  https://github.com/librepower/aix/releases/download/fzf-v0.46.1/fzf-0.46.1-1.librepower.aix7.3.ppc.rpm

rpm -ivh fzf-0.46.1-1.librepower.aix7.3.ppc.rpm
```

> ⚠️ **Important**: Use `-L` flag to follow redirects.

### Option 3: GitHub Releases Page

Download from [Releases](https://github.com/librepower/aix/releases/tag/fzf-v0.46.1)

## Quick Start

```bash
# Basic usage - pipe any list
ls | fzf

# Search installed packages
rpm -qa | fzf

# Find files
find /opt -type f | fzf

# Filter mode (non-interactive)
ps -ef | fzf --filter="sshd"
```

## ⚠️ AIX-Specific Configuration

When running `fzf` without piped input, it uses a default find command with GNU/Linux options that AIX doesn't support.

**Add to your `~/.bashrc`:**

```bash
# fzf default command for AIX
export FZF_DEFAULT_COMMAND='find . -type f -print 2>/dev/null'
```

After this, `fzf` works standalone:

```bash
cd /some/directory
fzf                    # Now works!
```

**Without this configuration**, always use fzf with piped input (which always works):

```bash
ls | fzf               # ✅ Works
rpm -qa | fzf          # ✅ Works
find /etc | fzf        # ✅ Works
```

## AIX Helper Scripts

This package includes ready-to-use scripts for common AIX tasks:

| Script | Description | Example |
|--------|-------------|---------|
| `fzf-rpm` | Browse installed RPM packages | `fzf-rpm python` |
| `fzf-proc` | Browse running processes | `fzf-proc oracle` |
| `fzf-svc` | Browse AIX services | `fzf-svc active` |
| `fzf-hist` | Search command history | `fzf-hist rpm` |

All scripts work interactively (no arguments) or as filters (with argument).

## Search Syntax

| Pattern | Match Type | Example |
|---------|-----------|---------|
| `term` | Fuzzy match | `oracl` matches `oracle` |
| `'term` | Exact match | `'oracle` matches only `oracle` |
| `^term` | Prefix match | `^ora` matches `oracle`, not `dboracle` |
| `term$` | Suffix match | `cle$` matches `oracle` |
| `!term` | Negation | `!test` excludes lines with `test` |
| `term1 term2` | AND | `ora db` matches lines with both |
| `term1 \| term2` | OR | `ora \| mysql` matches either |

## Shell Integration

Add to your `~/.bashrc` for advanced features:

```bash
# fzf default command for AIX (required!)
export FZF_DEFAULT_COMMAND='find . -type f -print 2>/dev/null'

# Enable completion and key bindings
source /opt/freeware/share/fzf/shell/completion.bash
source /opt/freeware/share/fzf/shell/key-bindings.bash
```

After sourcing:
- `Ctrl-T` - Paste selected file path
- `Ctrl-R` - Search command history
- `Alt-C` - cd into selected directory
- `**<Tab>` - Fuzzy completion (e.g., `vim **<Tab>`)

## Real-World Examples

### Find and edit a config file
```bash
vim $(find /etc -name "*.conf" 2>/dev/null | fzf)
```

### Kill a process interactively
```bash
kill $(ps -ef | fzf | awk '{print $2}')
```

### SSH to a host from known_hosts
```bash
ssh $(grep "^Host " ~/.ssh/config | cut -d' ' -f2 | fzf)
```

### Search error reports
```bash
errpt -a | fzf
```

### Browse filesystem usage
```bash
df -g | fzf --header-lines=1
```

## Building from Source

Want to compile fzf yourself or understand how Go works on AIX?

📖 **[BUILD.md](BUILD.md)** - Complete build instructions including:
- Why gccgo doesn't work (and what does)
- Installing Go toolchain on AIX
- Step-by-step compilation
- Creating the RPM package

## Package Contents

```
fzf-fuzzy-finder/
├── RPMS/
│   └── fzf-0.46.1-1.librepower.aix7.3.ppc.rpm
├── SPECS/
│   └── fzf.spec
├── SOURCES/
│   ├── fzf-rpm
│   ├── fzf-proc
│   ├── fzf-svc
│   └── fzf-hist
├── BUILD.md          # How to compile from source
├── INSTALL.txt       # English installation guide
├── INSTALL_ES.txt    # Spanish installation guide
├── fzf.gif           # Demo animation
└── README.md
```

## Requirements

- AIX 7.1+ or VIOS 3.x
- No additional dependencies

## Technical Notes

This is the **first Go-based tool** compiled for AIX as part of LibrePower.

**Why this matters:** The official Go toolchain (not gccgo) works on AIX, opening the door to hundreds of modern CLI tools written in Go.

Built with:
- Go 1.21.6 official toolchain from go.dev
- Static binary with no external dependencies
- 64-bit XCOFF executable

## License

- fzf: MIT License (by Junegunn Choi)
- AIX packaging and scripts: MIT License (LibrePower)

## Credits

- fzf by [Junegunn Choi](https://github.com/junegunn/fzf)
- AIX port and packaging by [SIXE](https://sixe.eu)
- Part of [LibrePower](https://librepower.org) - Unlocking Power Systems through open source 🌍
