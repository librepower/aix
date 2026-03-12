# IBM Semeru JDK 25 for AIX

> IBM Semeru Certified Edition JDK 25 (Eclipse OpenJ9) for AIX ppc64. Installs as an **additional** JDK — does **NOT** touch your system Java.

[![LibrePower](https://img.shields.io/badge/LibrePower-POWER_Computing-blue)](https://librepower.org)
[![AIX](https://img.shields.io/badge/AIX-7.3+-green)](https://www.ibm.com/power/operating-systems/aix)
[![JDK](https://img.shields.io/badge/JDK-25.0.2-red)](https://developer.ibm.com/languages/java/semeru-runtimes/)
[![OpenJ9](https://img.shields.io/badge/OpenJ9-0.57.0-orange)](https://eclipse.dev/openj9/)
[![License](https://img.shields.io/badge/License-IBM_ILA-lightgrey)](https://www.ibm.com/support/pages/ibm-international-license-agreement-non-warranted-programs)

---

## Important: Java Safety

This JDK is installed as an **additional** JDK alongside your existing Java.

It does **NOT**:
- Modify `/usr/bin/java` or `/usr/java8_64`
- Change system `JAVA_HOME` or `PATH`
- Create any symlinks in `/usr/bin/`
- Interfere with any existing Java installation

```
Your system Java:   /usr/java8_64                      (UNTOUCHED)
Semeru JDK 25:      /opt/freeware/lib/jvm/semeru-25    (isolated)
```

---

## Installation

### Option 1: DNF (Recommended)

```bash
# Add repository (one-time setup)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install
dnf install semeru-jdk25
```

> **Note**: This package is also installed automatically as a dependency of `gridgain`.

### Option 2: Direct RPM

```bash
curl -LO https://aix.librepower.org/packages/semeru-jdk25-25.0.2-1.librepower.aix7.1.ppc.rpm
rpm -ivh semeru-jdk25-25.0.2-1.librepower.aix7.1.ppc.rpm
```

---

## Usage

```bash
# Use Semeru JDK 25 for a specific session
export JAVA_HOME=/opt/freeware/lib/jvm/semeru-25
export PATH=$JAVA_HOME/bin:$PATH
java -version
# openjdk version "25.0.2" 2025-07-15
# IBM Semeru Runtime Certified Edition 25.0.2.0 (build 25.0.2+10)
# Eclipse OpenJ9 VM 25.0.2.0 (build openj9-0.57.0, ...)
```

Applications that depend on `semeru-jdk25` (like `gridgain`) set `JAVA_HOME` internally in their wrapper scripts — you don't need to configure anything.

---

## Paths

| Path | Description |
|------|-------------|
| `/opt/freeware/lib/jvm/semeru-25/` | JDK installation root |
| `/opt/freeware/lib/jvm/semeru-25/bin/java` | Java runtime |
| `/opt/freeware/lib/jvm/semeru-25/bin/javac` | Java compiler |

---

## Version Details

| | |
|--|--|
| **JDK** | 25.0.2+10 |
| **OpenJ9** | 0.57.0 |
| **Edition** | IBM Semeru Runtime Certified Edition |
| **Architecture** | ppc64 (64-bit, big-endian) |
| **RPM Size** | ~262 MB |
| **Source** | [ibmruntimes/semeru25-certified-binaries](https://github.com/ibmruntimes/semeru25-certified-binaries) |

### Compatibility

| AIX 7.1 | AIX 7.2 | AIX 7.3 |
|---------|---------|---------|
| ❓      | ❓      | ✅      |

> JDK 25 tested on AIX 7.3 only. May work on 7.2. Unlikely on 7.1.

---

## Package Contents

```
semeru-jdk25/
├── RPMS/
│   └── semeru-jdk25-25.0.2-1.librepower.aix7.1.ppc.rpm
├── SPECS/
│   └── semeru-jdk25.spec
└── README.md
```

---

## License

- IBM Semeru Runtime: IBM International License Agreement for Non-Warranted Programs
- AIX packaging: Apache 2.0 (LibrePower)

## Credits

- IBM Semeru Runtime by [IBM](https://developer.ibm.com/languages/java/semeru-runtimes/)
- Eclipse OpenJ9 by [Eclipse Foundation](https://eclipse.dev/openj9/)
- AIX packaging by [LibrePower](https://librepower.org)

## Links

- **Download**: [aix.librepower.org](https://aix.librepower.org)
- **IBM Semeru**: [developer.ibm.com/semeru-runtimes](https://developer.ibm.com/languages/java/semeru-runtimes/)
- **OpenJ9**: [eclipse.dev/openj9](https://eclipse.dev/openj9/)

---

<div align="center">

**Built by [LibrePower](https://librepower.org) -- Unlocking Power Systems**

[![Download](https://img.shields.io/badge/Download-aix.librepower.org-orange?style=for-the-badge)](https://aix.librepower.org)

</div>

---

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Beta live now!

---

*Part of [LibrePower](https://librepower.org) - Unlocking Power Systems through open source. Unmatched RAS and TCO. Minimal footprint 🌍*
