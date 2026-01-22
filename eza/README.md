# eza - Modern ls Replacement for AIX

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![eza](https://img.shields.io/badge/eza-0.23.4-green)
![License](https://img.shields.io/badge/license-MIT-blue)

A modern replacement for `ls` with colors, icons, and Git integration.

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Launching February 2026

---

## Demo

![eza demo on AIX](eza-demo.gif)

## Why eza?

eza is a modern, maintained replacement for `ls`. It provides colorized output, icons, Git status, and tree views out of the box.

**Instead of:**
```bash
ls -la
```

**Just do:**
```bash
eza -la
```

Same info, but colorized, with human-readable sizes and proper alignment.

## Installation

### Option 1: dnf (Recommended)

```bash
# Add repository (one-time setup)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install
dnf install eza
```

📦 Repository details: https://aix.librepower.org/

### Option 2: Direct RPM

```bash
rpm -ivh eza-0.23.4-1.librepower.aix7.3.ppc.rpm
```

## Quick Start

```bash
# List files with colors
eza

# Long format with icons (requires Nerd Font)
eza -l --icons

# Tree view
eza --tree

# Show git status
eza -l --git

# All files including hidden
eza -la
```

## Features

- **Colors**: File types, permissions, and sizes are color-coded
- **Icons**: Beautiful icons for file types (requires Nerd Font in terminal)
- **Git integration**: Shows modified/staged/untracked status
- **Tree view**: `--tree` for directory trees
- **Extended attributes**: Shows all file metadata
- **Human-readable sizes**: Easy to understand file sizes

## Useful Aliases

Add to your `~/.bashrc` or `~/.profile`:

```bash
alias ls='eza'
alias ll='eza -l'
alias la='eza -la'
alias lt='eza --tree'
alias lg='eza -la --git'
```

## Real-World Examples

### Tree view with depth limit
```bash
eza --tree --level=2 /opt/freeware
```

### Show only directories
```bash
eza -D
```

### Sort by modification time
```bash
eza -l --sort=modified
```

### Show file sizes in bytes
```bash
eza -l --bytes
```

### Git repository status
```bash
cd /your/git/repo
eza -la --git --git-ignore
```

## Package Contents

```
eza/
├── README.md
├── eza-demo.gif
└── RPMS/
    └── eza-0.23.4-1.librepower.aix7.3.ppc.rpm
```

## Technical Details

| Component | Details |
|-----------|---------|
| **Version** | 0.23.4 |
| **Platform** | AIX 7.3+ (ppc64) |
| **Compiler** | IBM Rust SDK 1.90 |
| **Binary Size** | ~7.5 MB |
| **Dependencies** | None (statically linked) |

## Requirements

- AIX 7.2+ or VIOS 3.x
- No additional dependencies
- For icons: Terminal with Nerd Font installed

## License

MIT - [eza-community/eza](https://github.com/eza-community/eza)

## Credits

- eza by [eza-community](https://github.com/eza-community)
- Originally exa by [Ben S](https://github.com/ogham)
- AIX port and packaging by [LibrePower](https://librepower.org)
- Part of [LibrePower](https://librepower.org) - Unlocking Power Systems through open source 🌍
