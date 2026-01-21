# fd - Fast Find Alternative for AIX

**LibrePower - Unlocking Power Systems through open source**

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![fd](https://img.shields.io/badge/fd-10.3.0-green)
![License](https://img.shields.io/badge/license-MIT%2FApache--2.0-blue)

A simple, fast and user-friendly alternative to `find`.

## Quick Start

```bash
# Install
dnf install fd

# Find files by pattern
fd pattern

# Find specific file types
fd -e rs       # Find .rs files
fd -e py -e js # Find .py and .js files

# Include hidden files
fd -H pattern
```

## Features

- Intuitive syntax: `fd PATTERN` instead of `find -iname '*PATTERN*'`
- Regular expressions and glob-based patterns
- Very fast due to parallelized directory traversal
- Smart case: search is case-insensitive by default
- Colorized terminal output (similar to ls)

## Installation

```bash
# Via dnf (recommended)
dnf install fd

# Direct RPM
rpm -ivh fd-10.3.0-1.librepower.aix7.3.ppc.rpm
```

## Examples

```bash
# Find all .c files
fd -e c

# Find files in specific directory
fd pattern /path/to/dir

# Exclude directories
fd -E node_modules pattern

# Execute command on results
fd -e txt -x wc -l
```

## Package Contents

```
fd/
├── README.md
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

## License

MIT/Apache-2.0 - [sharkdp/fd](https://github.com/sharkdp/fd)

## Credits

- fd by [David Peter](https://github.com/sharkdp)
- AIX port by [LibrePower](https://librepower.org)
