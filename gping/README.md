# gping - Ping with Graph for AIX

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![gping](https://img.shields.io/badge/gping-1.20.1-green)
![License](https://img.shields.io/badge/license-MIT-blue)

Ping hosts with a real-time graph in your terminal.

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Launching February 2026

---

## Demo

![gping demo on AIX](gping-demo.gif)

## Why gping?

gping is ping, but with a real-time graph. Visualize network latency trends instantly, compare multiple hosts, and identify network issues at a glance.

**Instead of:**
```bash
ping google.com
# Scrolling wall of text...
```

**Just do:**
```bash
gping google.com
```

Beautiful real-time graph showing latency trends.

## Installation

### Option 1: dnf (Recommended)

```bash
# Add repository (one-time setup)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install
dnf install gping
```

📦 Repository details: https://aix.librepower.org/

### Option 2: Direct RPM

```bash
rpm -ivh gping-1.20.1-1.librepower.aix7.3.ppc.rpm
```

## Quick Start

```bash
# Ping a single host
gping google.com

# Ping multiple hosts (compare latency)
gping google.com cloudflare.com 8.8.8.8

# Ping with specific count
gping -c 10 google.com

# Simple mode (no graph, just latency)
gping --simple google.com
```

## Features

- **Real-time graph**: See latency trends instantly
- **Multiple hosts**: Ping several hosts simultaneously
- **Color-coded**: Different colors per host for easy comparison
- **Custom AIX pinger**: Native AIX ping integration
- **TUI interface**: Beautiful terminal UI

## AIX Notes

> ⚠️ **Important**: TUI mode requires a proper terminal (PTY). If running via SSH, ensure you have a proper terminal allocated.

The AIX port includes a custom pinger implementation that uses the native AIX `ping` command, ensuring full compatibility.

## Real-World Examples

### Compare DNS providers
```bash
gping 8.8.8.8 1.1.1.1 9.9.9.9
```

### Monitor network path
```bash
gping router.local gateway.isp.com google.com
```

### Quick latency check
```bash
gping -c 5 --simple production-server
```

## Package Contents

```
gping/
├── README.md
├── gping-demo.gif
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
| **Dependencies** | None (statically linked) |

## Requirements

- AIX 7.2+ or VIOS 3.x
- No additional dependencies
- TUI mode requires terminal with PTY

## License

MIT - [orf/gping](https://github.com/orf/gping)

## Credits

- gping by [Tom Forbes](https://github.com/orf)
- AIX port and packaging by [LibrePower](https://librepower.org)
- Part of [LibrePower](https://librepower.org) - Unlocking Power Systems through open source 🌍
