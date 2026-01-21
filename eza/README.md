# eza - Modern ls Replacement for AIX

**LibrePower - Unlocking Power Systems through open source**

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![eza](https://img.shields.io/badge/eza-0.23.4-green)
![License](https://img.shields.io/badge/license-MIT-blue)

A modern replacement for `ls` with colors, icons, and Git integration.

## Quick Start

```bash
# Install
dnf install eza

# List files with colors
eza

# Long format with icons (requires Nerd Font)
eza -l --icons

# Tree view
eza --tree

# Show git status
eza -l --git
```

## Features

- Colors for file types and permissions
- Icons support (requires Nerd Font in terminal)
- Git integration (shows file status)
- Tree view with `--tree`
- Extended attributes support
- Human-readable file sizes

## Installation

```bash
# Via dnf (recommended)
dnf install eza

# Direct RPM
rpm -ivh eza-0.23.4-1.librepower.aix7.3.ppc.rpm
```

## Useful Aliases

Add to your `~/.bashrc` or `~/.profile`:

```bash
alias ls='eza'
alias ll='eza -l'
alias la='eza -la'
alias lt='eza --tree'
```

## Package Contents

```
eza/
├── README.md
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

## License

MIT - [eza-community/eza](https://github.com/eza-community/eza)

## Credits

- eza by [eza-community](https://github.com/eza-community)
- AIX port by [LibrePower](https://librepower.org)
