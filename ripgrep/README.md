# ripgrep - Blazingly Fast Search for AIX

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![Rust](https://img.shields.io/badge/Rust-1.90-orange)
![License](https://img.shields.io/badge/license-MIT-green)

ripgrep is a line-oriented search tool that recursively searches the current directory for a regex pattern. **4x faster than grep** with smart defaults and beautiful output.

![ripgrep Demo](demo/ripgrep-demo.gif)

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Launching February 2026

---

## Why ripgrep?

| Feature | grep | ripgrep |
|---------|------|---------|
| Speed | Baseline | **4x faster** |
| Colors | Manual flag | **Auto-detected** |
| .gitignore | No | **Respected by default** |
| Binary files | Searched | **Skipped automatically** |
| Unicode | Limited | **Full support** |
| Regex | Basic/Extended | **Rust regex (fast)** |

## Features

- **Blazingly fast** - Written in Rust, uses parallelism and SIMD
- **Smart defaults** - Respects .gitignore, skips binary files, hidden dirs
- **Beautiful output** - Colors, line numbers, context lines
- **Powerful regex** - Full regex support with lookahead/lookbehind
- **Cross-platform** - Same syntax on AIX, Linux, macOS, Windows

## Installation

### Option 1: dnf (Recommended)

```bash
# Add LibrePower repo (if not already added)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install ripgrep
dnf install ripgrep
```

### Option 2: Manual RPM

```bash
curl -L -o ripgrep-14.1.1-1.librepower.aix7.3.ppc.rpm \
  https://aix.librepower.org/packages/ripgrep-14.1.1-1.librepower.aix7.3.ppc.rpm

rpm -ivh ripgrep-14.1.1-1.librepower.aix7.3.ppc.rpm
```

## Quick Start

```bash
# Basic search
rg 'pattern' /path/to/search

# Case insensitive
rg -i 'error' /var/log

# Show line numbers (default)
rg -n 'function' src/

# Show context lines
rg -C 3 'TODO' .

# Count matches per file
rg -c 'import' *.py

# List files with matches only
rg -l 'password' /etc

# Search specific file types
rg -t py 'def main'

# Invert match (lines NOT matching)
rg -v '^#' config.txt

# Fixed string (no regex)
rg -F '192.168.1.1' /var/log
```

## Common Options

| Option | Description |
|--------|-------------|
| `-i` | Case insensitive search |
| `-v` | Invert match (exclude pattern) |
| `-w` | Match whole words only |
| `-n` | Show line numbers (default on) |
| `-l` | List matching files only |
| `-c` | Count matches per file |
| `-C NUM` | Show NUM context lines |
| `-A NUM` | Show NUM lines after match |
| `-B NUM` | Show NUM lines before match |
| `-t TYPE` | Search only files of TYPE |
| `-g GLOB` | Include/exclude files by glob |
| `-F` | Treat pattern as literal string |
| `--hidden` | Search hidden files too |
| `--no-ignore` | Don't respect .gitignore |

## Regex Examples

```bash
# IP addresses
rg '\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}' /var/log

# Email addresses
rg '[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}' .

# Function definitions (Python)
rg 'def \w+\(' *.py

# Error lines with timestamps
rg '^\d{4}-\d{2}-\d{2}.*ERROR' /var/log

# Lines starting with # (comments)
rg '^#' config.txt

# Lines ending with semicolon
rg ';$' *.c
```

## Configuration

Create `~/.ripgreprc` for persistent options:

```bash
# ~/.ripgreprc
--smart-case
--hidden
--glob=!.git
```

Then set the environment variable:

```bash
export RIPGREP_CONFIG_PATH=~/.ripgreprc
```

## Build Notes

- Compiled with IBM Open SDK for Rust 1.90
- 64-bit XCOFF binary
- No PCRE2 (uses Rust regex engine)
- Binary size: ~6.4 MB (stripped)

## Performance

Tested on AIX 7.3 TL4 searching `/var/adm`:

| Tool | Time |
|------|------|
| grep -r | 0.068s |
| **rg** | **0.018s** |

**Result: ripgrep is ~4x faster**

## Links

- [ripgrep GitHub](https://github.com/BurntSushi/ripgrep)
- [ripgrep User Guide](https://github.com/BurntSushi/ripgrep/blob/master/GUIDE.md)
- [LibrePower](https://librepower.org)

## License

- ripgrep: MIT / Unlicense (dual-licensed)
- AIX port: MIT (LibrePower)

## Credits

- ripgrep by [Andrew Gallant (BurntSushi)](https://github.com/BurntSushi)
- AIX port by [LibrePower](https://librepower.org)
