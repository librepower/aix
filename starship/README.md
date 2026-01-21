# Starship - Cross-Shell Prompt for AIX

**LibrePower - Unlocking Power Systems through open source**

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![Starship](https://img.shields.io/badge/starship-1.24.2-DD0B78)
![License](https://img.shields.io/badge/license-ISC-blue)

The minimal, blazing-fast, and infinitely customizable prompt for any shell!

## Quick Start

```bash
# Install
dnf install starship

# Add to ~/.bashrc or ~/.profile
eval "$(starship init bash)"

# Reload shell
source ~/.bashrc
```

## Features

- Works with any shell (bash, zsh, ksh, etc.)
- Git repository status
- Current directory
- Package version detection
- Command execution time
- Highly customizable via TOML config

## Installation

```bash
# Via dnf (recommended)
dnf install starship

# Direct RPM
rpm -ivh starship-1.24.2-1.librepower.aix7.3.ppc.rpm
```

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

## Shell Setup

### Bash
```bash
# Add to ~/.bashrc
eval "$(starship init bash)"
```

### Ksh
```ksh
# Add to ~/.kshrc
eval "$(starship init bash)"
```

## Package Contents

```
starship/
├── README.md
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

## License

ISC - [starship/starship](https://github.com/starship/starship)

## Credits

- Starship by [Starship Contributors](https://github.com/starship/starship)
- AIX port by [LibrePower](https://librepower.org)
