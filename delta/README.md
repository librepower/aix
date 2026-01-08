# Delta for AIX

A syntax-highlighting pager for git, diff, and grep output.

## Features

- Language syntax highlighting with color themes
- Within-line highlights based on Levenshtein edit inference
- Side-by-side view with line-wrapping
- Line numbering
- Improved merge conflict display
- Improved git blame display

## Installation

```bash
# Add LibrePower repo (if not already added)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install delta
dnf install delta
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

## Build Notes

- Compiled with IBM Open SDK for Rust 1.90
- Required patching `nix` crate for AIX `timespec` compatibility (tv_nsec is i32 on AIX)
- Uses system OpenSSL and zlib

## Links

- [Delta GitHub](https://github.com/dandavison/delta)
- [Delta Documentation](https://dandavison.github.io/delta/)
- [LibrePower](https://librepower.org)
