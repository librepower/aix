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

### 👾 DOOM ASCII
*Text-based DOOM running in your terminal*

Port of [doom-ascii](https://github.com/wojciech-graj/doom-ascii) to AIX/POWER. Play the classic DOOM game rendered entirely in ASCII or Unicode block characters directly in your terminal.

- ✅ **Big-endian support** - Proper byte-swapping for POWER architecture
- ✅ **64-bit XCOFF binary** - Native AIX executable
- ✅ **Unicode block/braille characters** - Higher resolution with UTF-8 locale
- ✅ **Multiple scaling options** - Adjust resolution to your terminal size
- ✅ **Color support** - 256-color terminal rendering

📁 **[Documentation & Downloads](doom-ascii-aix/)**

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

No code yet? Open an [Issue](https://gitlab.com/librepower-tools/aix/issues) with your ideas, requests, or questions.

---

Let's build something great for Power Systems together.

---

*Maintained by [SIXE](https://sixe.eu) - IBM Business Partner*
