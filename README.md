# LibrePower AIX

**LibrePower - Unlocking Power Systems through open source. Unmatched RAS and TCO. Minimal footprint 🌍**

Open-source tools and packages for AIX/VIOS on IBM Power systems.

> ⚠️ **Early Release**: These packages are provided as-is for testing and evaluation. 
> While we use them in production, bugs may exist. 

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

Lightweight system prober that captures "system fingerprints" for AI-assisted analysis. Features advanced PID attribution, baseline learning, and web dashboard.

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
