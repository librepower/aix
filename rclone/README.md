# rclone - Cloud Sync for AIX

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![Go](https://img.shields.io/badge/Go-1.24.11-00ADD8)
![License](https://img.shields.io/badge/license-MIT-green)

Rclone is a command-line program to manage files on cloud storage. Sync files to and from 70+ cloud providers including S3, Google Drive, Dropbox, Azure, and more.

## Demo

![rclone on AIX demo](rclone.gif)

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

**[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

**[librepower.org](https://librepower.org)** — Launching February 2026

---

## Features on AIX

| Feature | Status | Notes |
|---------|--------|-------|
| **sync/copy/move** | Working | Full bidirectional sync |
| **70+ cloud providers** | Working | S3, GDrive, Dropbox, Azure, etc. |
| **serve http** | Working | Built-in HTTP server |
| **serve webdav** | Working | WebDAV server |
| **serve ftp** | Working | FTP server |
| **serve nfs** | Working | NFS server (mount alternative!) |
| **bisync** | Working | Bidirectional sync |
| **encryption** | Working | Crypt backend |
| **remote control** | Working | rcd/rc daemon |
| **mount (FUSE)** | Not available | Use `serve nfs` instead |
| **ncdu** | Not available | Clipboard issues |

## Installation

### Option 1: dnf (Recommended)

```bash
# Add repository (one-time setup)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install
dnf install rclone
```

### Option 2: curl (if dnf/yum not available)

```bash
cd /tmp

curl -L -o rclone-1.73.0-1.librepower.aix7.3.ppc.rpm \
  https://aix.librepower.org/packages/rclone-1.73.0-1.librepower.aix7.3.ppc.rpm

rpm -ivh rclone-1.73.0-1.librepower.aix7.3.ppc.rpm
```

## Quick Start

### Configure a remote

```bash
rclone config
```

Follow the interactive prompts to set up your cloud provider.

### Basic operations

```bash
# List configured remotes
rclone listremotes

# List files on remote
rclone ls myremote:path/to/files

# Copy local files to cloud
rclone copy /local/path myremote:cloud/path

# Sync local to cloud (mirror)
rclone sync /local/path myremote:cloud/path

# Bidirectional sync
rclone bisync /local/path myremote:cloud/path --resync
```

### Serve files locally

```bash
# HTTP server (browse files in web browser)
rclone serve http /path/to/files --addr :8080

# WebDAV server (mount on other systems)
rclone serve webdav myremote:path --addr :8081

# FTP server
rclone serve ftp /path/to/files --addr :2121

# NFS server (alternative to FUSE mount!)
rclone serve nfs myremote:path --addr :2049
```

## AIX-Specific Notes

### No FUSE mount - Use NFS instead

AIX doesn't support FUSE, so the `rclone mount` command is not available. However, you can use `rclone serve nfs` to achieve similar functionality:

```bash
# Start NFS server in background
rclone serve nfs myremote:path --addr :2049 &

# Mount on AIX via standard NFS
mount -o vers=3 localhost:/myremote:path /mnt/cloud
```

### Configuration file location

```bash
~/.config/rclone/rclone.conf
```

## Supported Cloud Providers

Rclone supports 70+ cloud storage providers including:

- **Object Storage**: Amazon S3, Google Cloud Storage, Azure Blob, Backblaze B2, Cloudflare R2, MinIO, Wasabi
- **Consumer Cloud**: Google Drive, Dropbox, OneDrive, Box, pCloud, MEGA
- **Enterprise**: SharePoint, Nextcloud, Owncloud, WebDAV
- **Protocols**: SFTP, FTP, HTTP

Full list: https://rclone.org/overview/

## Building from Source

See [BUILD.md](BUILD.md) for instructions on compiling rclone for AIX.

## Package Contents

```
rclone/
├── RPMS/
│   └── rclone-1.73.0-1.librepower.aix7.3.ppc.rpm
├── SPECS/
│   └── rclone.spec
├── SOURCES/           # (empty - pure Go, no patches needed)
├── BUILD.md           # Build instructions
└── README.md          # This file
```

## Requirements

- AIX 7.1+ or VIOS 3.x
- No additional dependencies (static binary)

## Technical Notes

- **Version**: 1.73.0
- **Go version**: 1.24.11
- **Linking**: Static (no external dependencies)
- **Architecture**: ppc64 (64-bit XCOFF)
- **Size**: ~85MB (includes all cloud provider support)

## License

- rclone: MIT License (by Nick Craig-Wood)
- AIX packaging: MIT License (LibrePower)

## Credits

- rclone by [Nick Craig-Wood](https://github.com/rclone/rclone)
- AIX port and packaging by [LibrePower](https://librepower.org)

## Resources

- rclone documentation: https://rclone.org/docs/
- rclone forum: https://forum.rclone.org/
- LibrePower: https://librepower.org
