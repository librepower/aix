# nano - GNU Text Editor for AIX

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![Version](https://img.shields.io/badge/nano-8.3-orange)
![License](https://img.shields.io/badge/license-GPL--3.0-green)

The simple, friendly GNU text editor, now available for AIX. If you're tired of vi's modal editing, nano gives you a familiar, intuitive experience.

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Beta live now!

---

## Why nano?

**If you know how to type, you know how to use nano.**

nano is the default editor on many Linux distributions for good reason: it's simple, fast, and gets out of your way. Perfect for quick edits, config files, and anyone who doesn't want to learn vi keybindings.

| Feature | vi/vim | nano |
|---------|--------|------|
| Learning curve | Steep | None |
| Mode switching | Yes (insert/command) | No |
| Keyboard shortcuts | On-screen | Hidden |
| Save and quit | `:wq` | Ctrl+O, Ctrl+X |

### Features

- ✅ **Syntax highlighting** - 40+ languages enabled by default
- ✅ **UTF-8 support** - Full Unicode text editing
- ✅ **Search & replace** - With regex support
- ✅ **Multiple buffers** - Edit multiple files simultaneously
- ✅ **Auto-indentation** - Enabled by default (great for YAML, Python, etc.)
- ✅ **Line numbers** - Easy code navigation
- ✅ **Minimal dependencies** - Only requires ncurses (included in AIX Toolbox)

## Installation

### Option 1: dnf (Recommended)

Add the LibrePower repository and install with one command:

```bash
# Add repository (one-time setup)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install (automatically resolves ncurses dependency)
dnf install nano
```

📦 Repository details: https://aix.librepower.org/

### Option 2: curl (if dnf/yum not available)

```bash
cd /tmp

curl -L -o nano-8.3-3.librepower.aix7.3.ppc.rpm \
  https://github.com/librepower/aix/releases/download/nano-v8.3/nano-8.3-3.librepower.aix7.3.ppc.rpm

# Requires ncurses from AIX Toolbox
rpm -ivh nano-8.3-3.librepower.aix7.3.ppc.rpm
```

> ⚠️ **Note**: Use `-L` flag with curl to follow redirects.

### Option 3: GitHub Releases Page

Download from [Releases](https://github.com/librepower/aix/releases/tag/nano-v8.3)

## Quick Start

```bash
# Edit a file
nano /etc/hosts

# Edit with line numbers
nano -l /path/to/script.sh

# Open multiple files
nano file1.txt file2.txt

# Restricted mode (read-only, no save)
rnano sensitive-file.conf
```

## Syntax Highlighting

**Syntax highlighting is enabled by default** for 40+ file types via `/opt/freeware/etc/nanorc`.

Supported languages include: C, C++, Python, Shell, Perl, Ruby, Go, Rust, Java, JavaScript, JSON, YAML, XML, HTML, CSS, SQL, Makefile, and many more.

### Customizing

To add custom syntax or override defaults, create `~/.nanorc`:

```bash
# Load all default syntax files
include "/opt/freeware/share/nano/*.nanorc"

# Add your customizations
set linenumbers
set mouse
```

## Key Bindings

nano displays shortcuts at the bottom of the screen. Here are the essentials:

| Key | Action |
|-----|--------|
| Ctrl+O | Save file (WriteOut) |
| Ctrl+X | Exit nano |
| Ctrl+K | Cut current line |
| Ctrl+U | Paste cut text |
| Ctrl+W | Search |
| Ctrl+\ | Search and replace |
| Ctrl+G | Display help |
| Alt+U | Undo |
| Alt+E | Redo |
| Ctrl+C | Show cursor position |
| Alt+# | Toggle line numbers |

> **Note**: `^` means Ctrl key, `M-` means Alt key

## Configuration

nano is configured via:
- `/opt/freeware/etc/nanorc` - Global defaults (syntax highlighting + autoindent)
- `~/.nanorc` - User overrides

### Default Configuration

```nanorc
include "/opt/freeware/share/nano/*.nanorc"
set autoindent
```

### Common Customizations

Add to `~/.nanorc`:

```nanorc
set linenumbers
set mouse
set tabstospaces
set tabsize 4
```

## Building from Source

📖 **[BUILD.md](BUILD.md)** - Complete build instructions including:
- Prerequisites and dependencies
- Configure options for AIX
- OBJECT_MODE=64 requirement
- Creating the RPM package

## Package Contents

```
nano-editor/
├── RPMS/
│   └── nano-8.3-3.librepower.aix7.3.ppc.rpm
├── SPECS/
│   └── nano.spec
├── BUILD.md          # How to compile from source
├── INSTALL.txt       # English installation guide
├── INSTALL_ES.txt    # Spanish installation guide
└── README.md
```

## Requirements

- AIX 7.1+ or VIOS 3.x
- ncurses (from AIX Toolbox - installed automatically with dnf)

## Technical Notes

| Property | Value |
|----------|-------|
| Version | 8.3 |
| Release | 3.librepower |
| Architecture | 64-bit XCOFF (ppc) |
| Size | ~391 KB |
| UTF-8 | Enabled |
| Syntax files | 40+ (enabled by default) |

## License

- GNU nano: GPL-3.0 License
- AIX packaging: GPL-3.0 License (LibrePower)

## Credits

- GNU nano by the [Free Software Foundation](https://www.nano-editor.org/)
- AIX port and packaging by [LibrePower](https://librepower.org)
- Part of [LibrePower](https://librepower.org) - Unlocking Power Systems through open source 🌍
