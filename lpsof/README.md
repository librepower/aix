# lpsof - List Open Files for AIX

**Native AIX implementation of lsof (list open files)**

A production-ready tool for AIX 7.x sysadmins to list open files, track changes, and diagnose system issues.

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Launching February 2026

---

## Features

- **Safety limits** - Output limited to 100 processes by default to prevent system overload
- **Delta tracking** - Compare current state with saved snapshots for incident response
- **Watch mode** - Continuous monitoring with configurable polling interval
- **Summary mode** - Quick overview of top processes by open file count
- **Doctor mode** - System diagnostics and capability checks
- **Full TCP/UDP socket information** - Ports, addresses, and connection states via netstat + rmsock

## Installation

### Via DNF (Recommended)

```bash
dnf install lpsof
```

### Manual RPM Install

```bash
rpm -ivh lpsof-0.3.0-1.aix7.3.ppc.rpm
```

## Quick Start

```bash
# List open files (limited to 100 procs by default)
lpsof

# Files for specific PID
lpsof -p 1234

# Files by user
lpsof -u oracle

# Network connections on port 22
lpsof -i :22

# Top processes by FD count
lpsof summary

# System diagnostics
lpsof doctor
```

## Subcommands

| Subcommand | Purpose |
|------------|---------|
| `list`     | List open files (default) |
| `summary`  | Top N processes by open file count |
| `watch`    | Continuous monitoring |
| `delta`    | Compare with saved snapshot |
| `doctor`   | System diagnostics |

## Common Use Cases

### Finding What's Blocking an Unmount

```bash
lpsof +D /mnt/nfs
```

### Finding Network Connections

```bash
lpsof -i                # All network files
lpsof -i :22            # Port 22
lpsof -i TCP            # All TCP
lpsof -s TCP:LISTEN     # Listening sockets
lpsof -s TCP:ESTABLISHED # Active connections
```

### Incident Response

```bash
# Save baseline
lpsof delta --save

# Later, check what changed
lpsof delta
```

### Getting PIDs for Kill

```bash
kill $(lpsof -t -c httpd)
```

## Filter Options

| Option | Description |
|--------|-------------|
| `-p, --pid PID` | Filter by PID (^PID to exclude) |
| `-u, --user USER` | Filter by user |
| `-c, --cmd CMD` | Filter by command prefix |
| `-g PGID` | Filter by process group |
| `-d FD` | Filter by file descriptor (cwd, rtd, txt, mem, or number) |
| `--path PATH` | Filter by path substring |
| `--type TYPE` | Filter: file\|dir\|pipe\|device\|socket\|all |
| `-i [ADDR]` | Network files (optional: [46][TCP\|UDP][@host][:port]) |
| `-s STATE` | TCP state filter (e.g., TCP:LISTEN) |
| `--limit N` | Limit to N processes (default: 100) |
| `--no-limit` | Remove limit (use with caution) |
| `-a` | AND logic for combining filters |

## Output Options

| Option | Description |
|--------|-------------|
| `-t, --terse` | Output PIDs only (for scripting) |
| `-F` | Field output mode for machine parsing |
| `-H, --human` | Human readable sizes |
| `-l, --numeric-uid` | Show numeric UIDs |
| `-R, --ppid` | Show parent PID column |
| `-n` | No hostname resolution |
| `-P` | No port name resolution |

## Known Limitations

**AIX /proc filesystem difference**: Unlike Linux, AIX `/proc/PID/fd/N` entries are not symlinks to actual files. This means:

- `--path` and `+d/+D` filters only work for cwd/rtd, not regular FDs
- `--type file` cannot identify regular files in most cases
- File paths shown as `/proc/PID/fd/N` instead of actual paths

This is an AIX kernel limitation, not a bug in lpsof.

## Requirements

- AIX 7.1, 7.2, or 7.3
- Root access recommended for full process visibility
- `bos.perf.perfstat` fileset (libperfstat.a)

## Files

| File | Purpose |
|------|---------|
| `/opt/freeware/bin/lpsof` | Installed binary |
| `/opt/freeware/share/man/man1/lpsof.1` | Man page |
| `/var/tmp/lpsof.state` | Default state file for delta mode |

## Man Page Access

AIX doesn't search `/opt/freeware/share/man` by default. To access the man page:

```bash
# Option 1: Direct path
man -M /opt/freeware/share/man lpsof

# Option 2: Add to your profile (~/.profile or ~/.bashrc)
export MANPATH=/opt/freeware/share/man:${MANPATH:-/usr/share/man}

# Option 3: Install linux-compat package (sets MANPATH automatically)
dnf install linux-compat
```

## Version

Current version: **0.3.0** (Security-Hardened)

## License

MIT License - Copyright 2025-2026 LibrePower Project

## Support

Report bugs at: https://gitlab.com/librepower/aix/-/issues
