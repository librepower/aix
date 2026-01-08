# duf - Disk Usage/Free Utility

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![Go](https://img.shields.io/badge/Go-1.24-00ADD8)
![License](https://img.shields.io/badge/license-MIT-green)

duf is a modern replacement for `df`. It shows disk usage in a colorful, easy-to-read table format with sorting, filtering, and JSON output.

![duf Demo](demo/duf-demo.gif)

## Installation

```bash
# Add LibrePower repo (if not already added)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install duf
dnf install duf
```

## Quick Start

```bash
# Show all filesystems
duf

# Show specific paths
duf /home /opt

# JSON output
duf --json

# Only local filesystems
duf --only local
```

## Sample Output

```
╭──────────────────────────────────────────────────────────────────╮
│ 9 local devices                                                  │
├──────────────────┬────────┬────────┬────────┬────────┬──────────┤
│ MOUNTED ON       │   SIZE │   USED │  AVAIL │  USE%  │ TYPE     │
├──────────────────┼────────┼────────┼────────┼────────┼──────────┤
│ /                │  12.1G │   3.0G │   9.1G │  24.5% │ jfs2     │
│ /usr             │   7.8G │   2.7G │   5.1G │  34.4% │ jfs2     │
│ /var             │  15.2G │ 343.4M │  14.9G │   2.2% │ jfs2     │
│ /opt             │  17.1G │   9.7G │   7.4G │  56.9% │ jfs2     │
╰──────────────────┴────────┴────────┴────────┴────────┴──────────╯
```

## Common Options

| Option | Description |
|--------|-------------|
| `--all` | Show all filesystems |
| `--only local` | Only local filesystems |
| `--only network` | Only network filesystems |
| `--json` | Output as JSON |
| `--sort size` | Sort by size |
| `--hide-mp /proc` | Hide mount point |

## Build Notes

- Built with Go 1.24.11 (official)
- AIX support contributed by LibrePower
- PR upstream: https://github.com/muesli/duf/pull/354
- 64-bit XCOFF binary
- Size: ~7.5 MB

## Links

- [duf GitHub](https://github.com/muesli/duf)
- [LibrePower](https://librepower.org)

## License

MIT
