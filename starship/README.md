# Starship - Cross-Shell Prompt for AIX

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![Starship](https://img.shields.io/badge/starship-1.24.2-DD0B78)
![License](https://img.shields.io/badge/license-ISC-blue)

The minimal, blazing-fast, and infinitely customizable prompt for any shell!

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Launching February 2026

---

## Demo

![starship demo on AIX](starship-demo.gif)

## Why Starship?

Starship is a minimal, blazing-fast prompt that works with any shell. It shows relevant information about your environment—git branch, package version, execution time—without slowing you down.

**Instead of:**
```bash
PS1='[\u@\h \W]\$ '
# Boring, static prompt
```

**Just do:**
```bash
eval "$(starship init bash)"
# Beautiful, informative prompt!
```

## Installation

### Option 1: dnf (Recommended)

```bash
# Add repository (one-time setup)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install
dnf install starship
```

📦 Repository details: https://aix.librepower.org/

### Option 2: Direct RPM

```bash
rpm -ivh starship-1.24.2-1.librepower.aix7.3.ppc.rpm
```

## Quick Start

### Bash

Add to your `~/.bashrc`:

```bash
eval "$(starship init bash)"
```

### Ksh

Add to your `~/.kshrc`:

```ksh
eval "$(starship init bash)"
```

Then reload your shell:

```bash
source ~/.bashrc
```

## Features

- **Works with any shell**: bash, ksh, zsh
- **Blazing fast**: Written in Rust for speed
- **Git integration**: Branch, status, ahead/behind
- **Directory info**: Smart truncation
- **Command execution time**: Know how long commands take
- **Highly customizable**: TOML configuration

## Configuration

Create `~/.config/starship.toml`:

```toml
# Basic configuration
[character]
success_symbol = "[➜](bold green)"
error_symbol = "[✗](bold red)"

[directory]
truncation_length = 3
truncate_to_repo = true

[git_branch]
symbol = "🌱 "

[hostname]
ssh_only = false
format = "[$hostname]($style) "
```

## What It Shows

| Module | Description |
|--------|-------------|
| **Directory** | Current path (smart truncation) |
| **Git branch** | Current branch name |
| **Git status** | Modified, staged, conflicts |
| **Command duration** | Time for slow commands |
| **Hostname** | System name (configurable) |
| **Username** | Current user |
| **Time** | Current time (optional) |

## Real-World Examples

### Minimal prompt
```toml
# ~/.config/starship.toml
format = "$directory$character"
```

### Show hostname always
```toml
[hostname]
ssh_only = false
format = "[$hostname]($style) "
style = "bold blue"
```

### Custom git symbols
```toml
[git_status]
conflicted = "⚔️ "
ahead = "⬆️ "
behind = "⬇️ "
untracked = "❓"
modified = "📝"
staged = "✅"
```

## Package Contents

```
starship/
├── README.md
├── starship-demo.gif
└── RPMS/
    └── starship-1.24.2-1.librepower.aix7.3.ppc.rpm
```

## Technical Details

| Component | Details |
|-----------|---------|
| **Version** | 1.24.2 |
| **Platform** | AIX 7.3+ (ppc64) |
| **Compiler** | IBM Rust SDK 1.90 |
| **Binary Size** | ~30 MB |
| **Dependencies** | None (statically linked) |

## Requirements

- AIX 7.2+ or VIOS 3.x
- No additional dependencies
- UTF-8 locale recommended for symbols

## License

ISC - [starship/starship](https://github.com/starship/starship)

## Credits

- Starship by [Starship Contributors](https://github.com/starship/starship)
- AIX port and packaging by [LibrePower](https://librepower.org)
- Part of [LibrePower](https://librepower.org) - Unlocking Power Systems through open source 🌍
