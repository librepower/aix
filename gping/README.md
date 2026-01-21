# gping - Ping with Graph for AIX

**LibrePower - Unlocking Power Systems through open source**

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![gping](https://img.shields.io/badge/gping-1.20.1-green)
![License](https://img.shields.io/badge/license-MIT-blue)

Ping hosts with a real-time graph in your terminal.

## Quick Start

```bash
# Install
dnf install gping

# Ping a single host
gping google.com

# Ping multiple hosts
gping google.com cloudflare.com

# Ping with specific count
gping -c 10 google.com
```

## Features

- Ping multiple hosts simultaneously
- Real-time graph display in terminal
- Color-coded output for each host
- Cross-platform support including AIX

## Installation

```bash
# Via dnf (recommended)
dnf install gping

# Direct RPM
rpm -ivh gping-1.20.1-1.librepower.aix7.3.ppc.rpm
```

## AIX Notes

- TUI mode requires a proper terminal (PTY)
- Custom AIX pinger implementation for compatibility

## Package Contents

```
gping/
├── README.md
└── RPMS/
    └── gping-1.20.1-1.librepower.aix7.3.ppc.rpm
```

## Technical Details

| Component | Details |
|-----------|---------|
| **Version** | 1.20.1 |
| **Platform** | AIX 7.3+ (ppc64) |
| **Compiler** | IBM Rust SDK 1.90 |
| **Binary Size** | ~9.2 MB |

## License

MIT - [orf/gping](https://github.com/orf/gping)

## Credits

- gping by [Tom Forbes](https://github.com/orf)
- AIX port by [LibrePower](https://librepower.org)
