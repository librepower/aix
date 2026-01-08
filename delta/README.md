# Delta - Syntax-Highlighting Pager for Git on AIX

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![Rust](https://img.shields.io/badge/Rust-1.90-orange)
![License](https://img.shields.io/badge/license-MIT-green)

A syntax-highlighting pager for git, diff, and grep output. First Rust application ported to AIX for LibrePower.

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Launching February 2026

---

## Features

- Language syntax highlighting with color themes
- Within-line highlights based on Levenshtein edit inference
- Side-by-side view with line-wrapping
- Line numbering
- Improved merge conflict display
- Improved git blame display
- 188 languages supported

## Installation

### Option 1: dnf (Recommended)

```bash
# Add LibrePower repo (if not already added)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install delta
dnf install delta
```

### Option 2: Manual RPM

Download from [Releases](https://gitlab.com/librepower/aix/-/tree/main/delta/RPMS) and install:

```bash
rpm -ivh delta-0.18.2-1.librepower.aix7.3.ppc.rpm
```

## Usage

### As a standalone diff viewer

```bash
diff -u file1 file2 | delta
delta file1 file2
```

### Configure git to use delta

```bash
git config --global core.pager delta
git config --global interactive.diffFilter "delta --color-only"
```

### View git diffs with syntax highlighting

```bash
git diff
git log -p
git show
git blame
```

### Side-by-side view

```bash
git diff | delta --side-by-side
```

## Build Notes

- Compiled with IBM Open SDK for Rust 1.90
- Required patching `nix` crate for AIX `timespec` compatibility (tv_nsec is i32 on AIX)
- Uses system OpenSSL and zlib
- First Rust application in the LibrePower repository

## Links

- [Delta GitHub](https://github.com/dandavison/delta)
- [Delta Documentation](https://dandavison.github.io/delta/)
- [LibrePower](https://librepower.org)

## Credits

- Delta by [Dan Davison](https://github.com/dandavison)
- AIX port by [LibrePower](https://librepower.org)
