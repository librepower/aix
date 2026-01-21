# LibrePower AIX

**LibrePower - Unlocking Power Systems through open source. Unmatched RAS and TCO. Minimal footprint 🌍**

Open-source tools and packages for AIX/VIOS on IBM Power systems.

> ⚠️ **Early Release**: These packages are provided as-is for testing and evaluation.
> While we use them in production, bugs may exist.

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Launching February 2026

---

## Quick Install via DNF Repository

The easiest way to install our packages. One-time setup, then use `dnf install` like on Linux.

```bash
# Add LibrePower repository (one time)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install packages
dnf install fzf nano etc
```

📦 **Repository:** [aix.librepower.org](https://aix.librepower.org)

### Man Pages

AIX doesn't search `/opt/freeware/share/man` by default. To access man pages for LibrePower packages:

```bash
# Add to your ~/.profile or ~/.bashrc
export MANPATH=/opt/freeware/share/man:${MANPATH:-/usr/share/man}
```

Or install `linux-compat` which configures this automatically.

---

## Available Packages

### 🔐 2FA for AIX Made Simple

Google Authenticator two-factor authentication for AIX/VIOS, done right.

- ✅ QR codes work (libqrencode included)
- ✅ Bilingual setup wizards (EN/ES)
- ✅ NTP verification before setup
- ✅ Secure defaults
- ✅ Full rollback and emergency access instructions

📁 **[Documentation & Downloads](2fa-made-simple/)**

---

### ✏️ nano - GNU Text Editor
*Simple, friendly editor for everyone*

The default editor on many Linux distributions, now on AIX. 

- ✅ Simple, intuitive interface (no modal editing)
- ✅ Syntax highlighting for 40+ languages (enabled by default!)
- ✅ UTF-8 support
- ✅ Auto-indentation enabled
- ✅ On-screen keyboard shortcuts

📁 **[Documentation & Downloads](nano-editor/)**

---

### 🔍 fzf - Fuzzy Finder
*First? Go-based tool compiled for AIX*

The incredibly popular command-line fuzzy finder, now on AIX. Search through anything: files, processes, packages, command history.

- ✅ Blazing fast (500,000 items in < 1 second)
- ✅ Zero dependencies (single static binary)
- ✅ AIX-specific helper scripts (fzf-rpm, fzf-proc, fzf-svc)
- ✅ Shell integration (Ctrl-R history search, Ctrl-T file picker)
- ✅ The proof that modern Go tools can run on AIX

📁 **[Documentation & Downloads](fzf-fuzzy-finder/)**

---

### ☁️ rclone - Cloud Sync
*Rsync for cloud storage - 70+ providers supported*

Sync files to and from cloud storage providers like Amazon S3, Google Drive, Dropbox, Azure, and 70+ more. Full-featured cloud management from the command line.

- ✅ **70+ cloud providers** - S3, Google Drive, Dropbox, Azure, Backblaze, etc.
- ✅ **Bidirectional sync** - bisync for two-way synchronization
- ✅ **Built-in servers** - Serve files via HTTP, WebDAV, FTP, or NFS
- ✅ **Encryption** - Client-side encryption with crypt backend
- ✅ **Zero dependencies** - Single static binary
- ✅ **serve nfs** - Mount alternative for AIX (no FUSE needed)

📁 **[Documentation & Downloads](rclone/)**

---

### 🐧 Linux Compatibility
*Used to Linux? Feel at home on AIX*

A configuration layer for IBM's GNU tools. Makes the excellent IBM AIX Toolbox packages the default in your interactive shell.

**What IBM provides:** GNU coreutils, grep, sed, awk, find, tar, vim, tmux, jq, and more (via AIX Toolbox)  
**What we add:** Shell configuration, aliases, emulated commands, and `systemctl`/`service` wrappers for AIX SRC

- ✅ Puts IBM's GNU tools first in your interactive PATH
- ✅ `systemctl` and `service` wrappers for familiar service management
- ✅ Emulates missing commands (`watch`, `pgrep`, `pkill`, `free`)
- ✅ Safe by design—only interactive shells affected
- ✅ Scripts using `#!/bin/sh` remain untouched

📁 **[Documentation & Downloads](linux-compat/)**

---

### 🔍 C-Sentinel - Semantic Observability for AIX
*AI-assisted system monitoring and anomaly detection*

AIX port of [C-Sentinel](https://github.com/williamofai/c-sentinel) by William. Lightweight system prober that captures "system fingerprints" for AI-assisted analysis. Features advanced PID attribution, baseline learning, and web dashboard.

- ✅ **95% feature parity** with Linux version on AIX 7.1/7.2/7.3
- ✅ **PID attribution for 70+ ports** (SSH, PostgreSQL, MySQL, Oracle, DB2, Informix, WebSphere, SAP, etc.)
- ✅ **System monitoring** via libperfstat (uptime, load, memory)
- ✅ **Process monitoring** - 408+ processes tracked via /proc/psinfo
- ✅ **Network monitoring** - TCP/UDP listeners with intelligent PID detection
- ✅ **Config drift detection** - SHA256 checksums with baseline comparison
- ✅ **Process chain analysis** - Detect suspicious parent-child relationships
- ✅ **Baseline learning** - Automatic learning and anomaly detection
- ✅ **Web dashboard** - Multi-user authentication, PostgreSQL backend, real-time alerts
- ✅ **RPM package** - Easy installation via DNF

📁 **[Documentation & Downloads](csentinel4aix/)**

---

### 📂 lpsof - List Open Files for AIX
*Native lsof implementation for AIX sysadmins*

Production-ready tool to list open files, track changes, and diagnose system issues. Uses AIX-native APIs (getprocs64, libperfstat) for full TCP/UDP socket information.

- ✅ **Safety limits** - Default 100 process limit prevents system overload
- ✅ **Delta tracking** - Compare snapshots for incident response
- ✅ **Watch mode** - Continuous monitoring with configurable interval
- ✅ **Summary mode** - Top processes by open file count
- ✅ **Full socket info** - TCP/UDP ports, addresses, connection states
- ✅ **Security hardened** - Input validation, no command injection

📁 **[Documentation & Downloads](lpsof/)**

---

### 🐘 PHP 8.3 - Modern PHP for AIX
*The latest PHP LTS release with 53 extensions*

Full port of PHP 8.3.16 to AIX 7.3. Modern web development with Fibers, opcache, PHP-FPM, and complete database support. Perfect for WordPress, Drupal, Zabbix frontend.

- ✅ **53 extensions** - bcmath, curl, gd, intl, mbstring, mysqli, pgsql, soap, sodium, zip...
- ✅ **Fibers support** - Native ppc64 assembly for coroutines
- ✅ **PHP-FPM** - Production-ready process manager
- ✅ **Database ready** - MySQL, PostgreSQL, SQLite
- ✅ **Zabbix compatible** - All required extensions included
- ✅ **Benchmark** - 2.436s (Zend bench.php on POWER9)

📁 **[Documentation & Downloads](php83/)**

---

### 🗄️ MariaDB - Database Server
*Enterprise SQL database on POWER architecture*

Full port of MariaDB 11.8.0 to AIX 7.3. Community-developed fork of MySQL with Performance Schema support and minimal AIX-specific patches ready for upstream submission.

- ✅ **Full MariaDB 11.8.0** - Complete SQL database functionality
- ✅ **Performance Schema** - Working correctly with AIX-specific patches
- ✅ **AIX SRC integration** - Managed with startsrc/stopsrc commands
- ✅ **Minimal patches** - Only 2 CMake configuration changes (28 lines)
- ✅ **Upstream ready** - Patches documented for MariaDB project submission
- ✅ **C++11 threading** - Uses pthread-enabled libstdc++ automatically

📁 **[Documentation & Downloads](https://gitlab.com/librepower/mariadb)**

---

### 🌐 Complete Web Stack for AIX

**Run WordPress, Nextcloud, Flarum, Lychee, Kanboard and more on AIX!**

Between LibrePower and IBM AIX Toolbox, you now have a complete LAMP/LEMP stack:

| Component | Version | Source | Install |
|-----------|---------|--------|---------|
| **Apache httpd** | 2.4.66 | IBM AIX Toolbox | `dnf install httpd` |
| **nginx** | 1.27.4 | IBM AIX Toolbox | `dnf install nginx` |
| **PHP** | 8.3.16 | LibrePower | `dnf install php83 php83-fpm` |
| **MariaDB** | 11.8.0 | LibrePower | `dnf install mariadb11` |
| **PostgreSQL** | 16.x | IBM AIX Toolbox | `dnf install postgresql16-server` |

**Quick LAMP setup:**
```bash
# Add LibrePower repository
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install web stack
dnf install httpd php83 mariadb11

# Start services
startsrc -s httpd
startsrc -s mariadb11
```

---

### 👾 DOOM ASCII
*Text-based DOOM running in your terminal*

Port of [doom-ascii](https://github.com/wojciech-graj/doom-ascii) to AIX/POWER. Play the classic DOOM game rendered entirely in ASCII or Unicode block characters directly in your terminal.

- ✅ **Big-endian support** - Proper byte-swapping for POWER architecture
- ✅ **64-bit XCOFF binary** - Native AIX executable
- ✅ **Unicode block/braille characters** - Higher resolution with UTF-8 locale
- ✅ **Multiple scaling options** - Adjust resolution to your terminal size
- ✅ **Color support** - 256-color terminal rendering

📁 **[Documentation & Downloads](doom-ascii/)**

---

### 🎨 delta - Syntax-Highlighting Pager for Git
*First Rust application ported to AIX*

Port of [delta](https://github.com/dandavison/delta) to AIX/POWER. A syntax-highlighting pager for git, diff, and grep output that makes code reviews beautiful.

- ✅ **188 languages** - Syntax highlighting for virtually any language
- ✅ **Git integration** - Works with git diff, log, show, blame
- ✅ **Side-by-side view** - Compare changes in parallel columns
- ✅ **Word-level diffs** - Highlights exactly what changed within lines
- ✅ **20+ themes** - Dracula, Monokai, Nord, Solarized, and more
- ✅ **Compiled with Rust 1.90** - IBM Open SDK for Rust on AIX

📁 **[Documentation & Downloads](delta/)**

---


### 🔐 age - Modern File Encryption
*Simple, secure encryption with no config needed*

Port of [age](https://github.com/FiloSottile/age) to AIX. A simple, modern, and secure file encryption tool with small explicit keys, no config options, and UNIX-style composability.

- ✅ **Simple keys** - No complex key management
- ✅ **Passphrase mode** - Encrypt with just a password
- ✅ **SSH key support** - Use existing SSH keys for encryption
- ✅ **Composable** - Works well with pipes and scripts
- ✅ **Zero dependencies** - Single static binary

📁 **[Documentation & Downloads](age/)**

---

### 💾 duf - Disk Usage/Free Utility
*A better df alternative with colors and graphs*

Port of [duf](https://github.com/muesli/duf) to AIX. Modern disk usage utility with colorful output, sorting, filtering, and JSON support.

- ✅ **Beautiful output** - Color-coded tables with usage bars
- ✅ **Smart grouping** - Local, network, fuse, special devices
- ✅ **Multiple formats** - Table, JSON, CSV output
- ✅ **Filtering** - Show only specific filesystem types
- ✅ **Zero dependencies** - Single static binary

📁 **[Documentation & Downloads](duf/)**

---

### 🔧 gron - Make JSON Greppable
*Transform JSON for easy grep and sed processing*

Port of [gron](https://github.com/tomnomnom/gron) to AIX. Transforms JSON into discrete assignments to make it easier to grep and see the absolute path to each value.

- ✅ **Greppable JSON** - Find paths to values easily
- ✅ **Reversible** - Convert back to JSON with `gron -u`
- ✅ **Stream processing** - Works with pipes and large files
- ✅ **Colorized output** - Easy to read paths
- ✅ **Zero dependencies** - Single static binary

📁 **[Documentation & Downloads](gron/)**

---

### 🔍 ripgrep - Blazingly Fast Search
*The fastest grep alternative, now on AIX*

Port of [ripgrep](https://github.com/BurntSushi/ripgrep) to AIX. A line-oriented search tool that recursively searches directories for a regex pattern. **First Rust application compiled for AIX with IBM Open SDK for Rust.**

- ✅ **4x faster than grep** - Written in Rust with SIMD optimization
- ✅ **Smart defaults** - Respects .gitignore, skips binary files
- ✅ **Beautiful output** - Colors, line numbers, context
- ✅ **Full regex** - Powerful Rust regex engine
- ✅ **Unicode support** - Full UTF-8 handling
- ✅ **Compiled with Rust 1.90** - IBM Open SDK for Rust on AIX

📁 **[Documentation & Downloads](ripgrep/)**

---

### 💾 stgtui - AIX Storage Explorer
*Professional TUI for LVM/SAN Management*

A LibrePower original tool, created exclusively for AIX. Terminal User Interface for exploring and monitoring AIX storage: Volume Groups, Logical Volumes, Filesystems, Physical Volumes, and LUNs.

- ✅ **8 interactive views** - Dashboard, VGs, Health, LVs, I/O, Mirrors, bidirectional mapping
- ✅ **Health monitoring** - Stale PPs, multipath, quorum, errpt disk errors
- ✅ **Unused disk detection** - Shows clean vs VGDA remnants
- ✅ **Bidirectional navigation** - FS ↔ LV ↔ VG ↔ PV ↔ LUN
- ✅ **Search & Export** - Find anything, save reports
- ✅ **vim keys** - j/k scroll, g/G navigation
- ✅ **Configurable thresholds** - ~/.stgtuirc for custom alerts

📁 **[Documentation & Downloads](stgtui/)**

---

### 📝 yq - YAML/JSON/XML Processor
*Like jq, but for YAML and more*

Port of [yq](https://github.com/mikefarah/yq) to AIX. A lightweight and portable command-line processor for YAML, JSON, XML, CSV, and properties files.

- ✅ **Multi-format** - YAML, JSON, XML, CSV, Properties
- ✅ **Convert between formats** - YAML to JSON, JSON to YAML, etc.
- ✅ **jq-like syntax** - Familiar expression language
- ✅ **In-place editing** - Modify files directly
- ✅ **Merge files** - Combine multiple YAML/JSON files
- ✅ **Zero dependencies** - Single static binary

📁 **[Documentation & Downloads](yq/)**

---

## Contribute

**We welcome contributions from the community!**

Have you compiled open source software for AIX? Built something useful for Power Systems? Share it here.

### What we're looking for

- 📦 RPM packages compiled for AIX/VIOS
- 🛠️ Tools and utilities for Power Systems
- 📚 Documentation and guides
- 🔧 Patches for AIX compatibility

### How to contribute

1. **Fork** this repository
2. **Add** your package:
   ```
   your-package/
   ├── RPMS/           # Compiled RPMs
   ├── SPECS/          # Spec files (for reproducibility)
   ├── SOURCES/        # Scripts, patches, configs
   ├── README.md       # Documentation
   └── BUILD.md        # Building instructions / DIY
   
   ```
3. **Submit** a Pull Request

### Guidelines

- Include license information
- Document tested AIX/VIOS versions
- Provide build instructions when possible
- Documentation in any language welcome. We maintain English and Spanish; other languages supported if volunteers step up

### Ideas welcome too

No code yet? Open an [Issue](https://gitlab.com/librepower/aix/issues) with your ideas, requests, or questions.

---

Let's build something great for Power Systems together.

---

*Maintained by [LibrePower](https://librepower.org)*
