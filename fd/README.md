# fd - Fast Find Alternative for AIX

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![fd](https://img.shields.io/badge/fd-10.3.0-green)
![License](https://img.shields.io/badge/license-MIT%2FApache--2.0-blue)

A simple, fast and user-friendly alternative to `find`.

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Launching February 2026

---

## Demo

![fd demo on AIX](fd-demo.gif)

## Why fd?

fd is a program to find entries in your filesystem. It's a simple, fast, and user-friendly alternative to the classic `find` command.

**Instead of:**
```bash
find /opt -iname '*config*' -type f
```

**Just do:**
```bash
fd config /opt
```

Much simpler, colorized output, and 10x faster due to parallel search.

### Performance on AIX

| Files | Search Time |
|-------|-------------|
| 100,000 files | ~0.5 seconds |
| Recursive search | Parallel by default |

## Installation

### Option 1: dnf (Recommended)

```bash
# Add repository (one-time setup)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install
dnf install fd
```

📦 Repository details: https://aix.librepower.org/

### Option 2: Direct RPM

```bash
rpm -ivh fd-10.3.0-1.librepower.aix7.3.ppc.rpm
```

## Quick Start

```bash
# Find files by pattern
fd pattern

# Find specific file types
fd -e rs          # Find .rs files
fd -e py -e js    # Find .py and .js files

# Include hidden files
fd -H pattern

# Case-sensitive search
fd -s Pattern
```

## Features

- **Intuitive syntax**: `fd PATTERN` instead of `find -iname '*PATTERN*'`
- **Regular expressions**: Full regex support by default
- **Parallel search**: Very fast due to parallelized directory traversal
- **Smart case**: Case-insensitive by default, smart switch when pattern has uppercase
- **Colorized output**: Easy to read results (similar to ls)
- **.gitignore aware**: Respects .gitignore by default

## Real-World Examples

### Find all shell scripts
```bash
fd -e sh
```

### Find files modified in last 24 hours
```bash
fd --changed-within 24h
```

### Find and delete .tmp files
```bash
fd -e tmp -x rm {}
```

### Search only in specific directory
```bash
fd config /etc
```

### Exclude directories
```bash
fd -E node_modules -E .git pattern
```

### Execute command on results
```bash
fd -e c -x wc -l   # Count lines in all .c files
```

## Package Contents

```
fd/
├── README.md
├── fd-demo.gif
└── RPMS/
    └── fd-10.3.0-1.librepower.aix7.3.ppc.rpm
```

## Technical Details

| Component | Details |
|-----------|---------|
| **Version** | 10.3.0 |
| **Platform** | AIX 7.3+ (ppc64) |
| **Compiler** | IBM Rust SDK 1.90 |
| **Binary Size** | ~10 MB |
| **Dependencies** | None (statically linked) |

## Requirements

- AIX 7.2+ or VIOS 3.x
- No additional dependencies

## License

MIT/Apache-2.0 - [sharkdp/fd](https://github.com/sharkdp/fd)

## Credits

- fd by [David Peter](https://github.com/sharkdp)
- AIX port and packaging by [LibrePower](https://librepower.org)
- Part of [LibrePower](https://librepower.org) - Unlocking Power Systems through open source 🌍
