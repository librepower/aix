# LibrePower AIX

**LibrePower - Unlocking Power Systems through open source. Unmatched RAS and TCO. Minimal footprint 🌍**

Open-source tools and packages for AIX/VIOS on IBM Power systems.

> **Tested on AIX 7.1, 7.2, and 7.3** — All packages install without `--ignoreos` on any supported version.
> While we use them in production, please report issues via GitLab.

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Beta live now!

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

### System Requirements

| AIX Version | Status | Tested |
|-------------|--------|--------|
| AIX 7.1 TL5+ | Supported | 7100-05-09-2148 |
| AIX 7.2 TL4+ | Supported | 7200-05-10-2520 |
| AIX 7.3 TL2+ | Supported | 7300-04-00-2546 |

All packages are built with `OS:aix7.1` for maximum compatibility across AIX versions.


### AIX 7.1/7.2 Compatibility (NEW)

We've improved compatibility across all AIX versions:

| Feature | AIX 7.1 | AIX 7.2 | AIX 7.3 |
|---------|---------|---------|---------|
| **Go packages** (fzf, duf, rclone, yq, age, gron, stgtui) | ✅ | ✅ | ✅ |
| **Rust packages** (ripgrep, fd, delta, eza, gping, starship) | ✅ | ✅ | ✅ |
| **nano** | ✅* | ✅* | ✅ |
| **mariadb11** | ❌ | ✅* | ✅ |
| **php83** | ❌ | ✅ | ✅ |
| **caddy** | ✅ | ✅ | ✅ |
| **gridgain** + **semeru-jdk25** | ❓ | ❓ | ✅ |

**Legend:** ✅ = Works | ✅* = Requires `librepower-compat` shim | ❌ = Not compatible

**New packages for compatibility:**
- **libunwind** — Enables all Rust packages on AIX 7.1/7.2 (auto-installed as dependency)
- **librepower-compat** — Provides missing `single_locale` symbol for nano/mariadb on older AIX

```bash
# For nano on AIX 7.1/7.2:
dnf install librepower-compat nano
export LDR_PRELOAD64="/opt/freeware/lib/librepower/libcompat.a(shr_64.o)"
nano file.txt
```

### DNF Installation on AIX 7.1/7.2

AIX 7.3 ships with DNF ready to use. For AIX 7.1 and 7.2, you need to install DNF manually:

1. **Prerequisites** (from [IBM AIX Toolbox](https://www.ibm.com/support/pages/aix-toolbox-open-source-software)):
   - **OpenSSL 3.0+** - Required for secure connections
   - **ncurses** - Required by rpmlibs32

2. **Install DNF Bundle**:
   ```bash
   # Download DNF bundle from IBM
   curl -O https://public.dhe.ibm.com/aix/freeSoftware/aixtoolbox/ezinstall/ppc/dnf_aixtoolbox.sh
   chmod +x dnf_aixtoolbox.sh
   ./dnf_aixtoolbox.sh
   ```

3. **Add LibrePower Repository**:
   ```bash
   curl -fsSL https://aix.librepower.org/install.sh | sh
   ```

For detailed instructions, see [AIX Toolbox DNF documentation](https://www.ibm.com/support/pages/aix-toolbox-open-source-software-downloads-alpha#D).

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

### 🗄️ MariaDB 11.8.5 LTS - Database Server
*Enterprise SQL database on POWER architecture*

Full port of MariaDB 11.8.5 LTS to AIX 7.3 with **native thread pool** (`pool-of-threads`). Up to 83% faster than one-thread-per-connection for concurrent workloads. Built with `-O3 -mcpu=power9`.

- ✅ **MariaDB 11.8.5 LTS** - Latest long-term support release
- ✅ **Thread Pool** - Native AIX pollset support (pool-of-threads), up to 83% faster
- ✅ **Optimized** - Built with `-O3 -mcpu=power9` for POWER9+ systems
- ✅ **QA Validated** - 1,000 clients, 30 min sustained, 0 errors
- ✅ **Performance Schema** - Working with AIX-specific patches
- ✅ **AIX SRC integration** - Managed with startsrc/stopsrc commands
- ✅ **3 patches** - 2 CMake fixes + thread pool, submitted upstream

📁 **[Documentation & Downloads](mariadb11/)**

---

### 🌐 Complete Web Stack for AIX

**Run WordPress, Nextcloud, Flarum, Lychee, Kanboard and more on AIX!**

Between LibrePower and IBM AIX Toolbox, you now have a complete LAMP/LEMP stack with AI/vector search capabilities:

| Component | Version | Source | Install |
|-----------|---------|--------|---------|
| **Apache httpd** | 2.4.66 | IBM AIX Toolbox | `dnf install httpd` |
| **nginx** | 1.27.4 | IBM AIX Toolbox | `dnf install nginx` |
| **PHP** | 8.3.16 | LibrePower | `dnf install php83 php83-fpm` |
| **MariaDB** | 11.8.5 | LibrePower | `dnf install mariadb11` |
| **PostgreSQL** | 18.0 | IBM AIX Toolbox | `dnf install postgresql18-server` |
| **pgvector** | 0.8.1 | IBM AIX Toolbox | `dnf install postgresql18-pgvector` |

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

📁 **[Full Documentation: LAMP/LEMP, PHP-FPM, pgvector for AI/RAG](webstack/)**

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

### 🔎 fd - Fast Find Alternative
*A simple, fast and user-friendly alternative to find*

Port of [fd](https://github.com/sharkdp/fd) to AIX. An intuitive alternative to `find` with colorized output, smart case, and parallel search.

- ✅ **Intuitive syntax** - `fd pattern` instead of `find -iname '*pattern*'`
- ✅ **Blazing fast** - Parallelized directory traversal
- ✅ **Smart case** - Case-insensitive by default, smart switch
- ✅ **Colorized output** - Easy to read results
- ✅ **Regex support** - Full regular expression patterns
- ✅ **Compiled with Rust 1.90** - IBM Open SDK for Rust on AIX

📁 **[Documentation & Downloads](fd/)**

---

### 📂 eza - Modern ls Replacement
*A modern, maintained replacement for ls*

Port of [eza](https://github.com/eza-community/eza) to AIX. Beautiful file listings with colors, icons, Git integration, and tree view.

- ✅ **Colors & Icons** - Beautiful terminal output (icons require Nerd Font)
- ✅ **Git integration** - Shows file status in repositories
- ✅ **Tree view** - `eza --tree` for directory trees
- ✅ **Extended attributes** - Shows permissions, sizes, dates
- ✅ **Human-readable sizes** - Easy to understand file sizes
- ✅ **Compiled with Rust 1.90** - IBM Open SDK for Rust on AIX

📁 **[Documentation & Downloads](eza/)**

---

### 📊 gping - Ping with Graph
*Ping, but with a real-time graph*

Port of [gping](https://github.com/orf/gping) to AIX. Visualize ping latency in real-time with a beautiful terminal graph.

- ✅ **Real-time graph** - See latency trends instantly
- ✅ **Multiple hosts** - Ping several hosts simultaneously
- ✅ **Color-coded** - Different colors per host
- ✅ **Custom AIX pinger** - Native AIX ping integration
- ✅ **TUI interface** - Beautiful terminal UI (requires PTY)
- ✅ **Compiled with Rust 1.90** - IBM Open SDK for Rust on AIX

📁 **[Documentation & Downloads](gping/)**

---

### ✨ starship - Cross-Shell Prompt
*The minimal, blazing-fast, and infinitely customizable prompt*

Port of [starship](https://github.com/starship/starship) to AIX. A beautiful, fast prompt that works with any shell and shows git status, directory, and more.

- ✅ **Any shell** - Works with bash, ksh, zsh
- ✅ **Blazing fast** - Written in Rust for speed
- ✅ **Git integration** - Branch, status, ahead/behind
- ✅ **Highly customizable** - TOML configuration
- ✅ **Battery/time/hostname** - All the info you need
- ✅ **Compiled with Rust 1.90** - IBM Open SDK for Rust on AIX

📁 **[Documentation & Downloads](starship/)**

---

### 🌐 Caddy - Modern Web Server
*Automatic HTTPS, reverse proxy, and file server*

Port of [Caddy](https://github.com/caddyserver/caddy) to AIX. The most popular modern web server with zero-config HTTPS, reverse proxy, load balancing, and static file serving.

- ✅ **Automatic HTTPS** — Let's Encrypt and ZeroSSL, zero configuration
- ✅ **Reverse proxy** — Load balancing, health checks, circuit breakers
- ✅ **5,300+ req/s** — On par with Linux ppc64le performance
- ✅ **Zero dependencies** — Single static binary (Go, CGO_ENABLED=0)
- ✅ **All AIX versions** — Works on AIX 7.1, 7.2, and 7.3
- ✅ **Patched badger** — `syscall.FcntlFlock` replaces `unix.Flock` for AIX

📁 **[Documentation & Downloads](caddy/)**

---

### ⚡ GridGain - In-Memory Computing Platform
*Distributed caching, SQL, and compute grid on POWER*

Port of [GridGain Community Edition](https://www.gridgain.com) (based on Apache Ignite) to AIX. In-memory data grid with distributed caching, SQL engine, and compute capabilities. **Does NOT touch your system Java.**

- ✅ **5,000+ ops/s** — In-memory cache on POWER9
- ✅ **SQL engine** — ANSI SQL queries over in-memory data via SQLLine
- ✅ **REST API** — HTTP interface for cache operations
- ✅ **Java safe** — Private JDK (Semeru 25), does NOT modify system Java
- ✅ **One command** — `dnf install gridgain` installs everything including JDK
- ✅ **Patched for OpenJ9** — Graceful fallback for HotSpot-specific APIs

> Tested on AIX 7.3 only. Requires JDK 25 (installed automatically).

📁 **[Documentation & Downloads](gridgain/)**

---

### ☕ IBM Semeru JDK 25
*Modern JDK for AIX — isolated, safe, no conflicts*

IBM Semeru Certified Edition JDK 25 (Eclipse OpenJ9) for AIX ppc64. Installed as an **additional** JDK that does NOT modify your system Java, PATH, or JAVA_HOME.

- ✅ **JDK 25.0.2** — Latest LTS with OpenJ9 0.57.0
- ✅ **Safe install** — Does NOT touch `/usr/bin/java` or `/usr/java8_64`
- ✅ **Auto-installed** — Pulled automatically as `gridgain` dependency
- ✅ **IBM Certified** — Official IBM Semeru Runtime Certified Edition

> Tested on AIX 7.3 only. Does NOT interfere with existing Java installations.

📁 **[Documentation & Downloads](semeru-jdk25/)**

---

### 📚 libunwind - Stack Unwinding Library
*Required dependency for Rust packages on AIX 7.1/7.2*

Repackaged from IBM Open XL C++ 17.1.3 Runtime. Provides the stack unwinding library that Rust-compiled binaries require on AIX.

- ✅ **Automatic installation** - DNF installs as dependency of Rust packages
- ✅ **Works on all AIX versions** - 7.1, 7.2, and 7.3
- ✅ **Original source** - IBM Open XL C++ Utilities (freely available)
- ✅ **Enables** - ripgrep, fd, delta, eza, gping, starship

📁 **[Documentation & Downloads](libunwind/)**

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
