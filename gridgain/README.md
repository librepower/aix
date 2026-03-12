# GridGain Community Edition 8.9.30 for AIX

> In-memory computing platform with **distributed caching**, **SQL**, and **compute grid**. **5,000+ ops/s** on POWER9. Does **NOT** touch your system Java.

[![LibrePower](https://img.shields.io/badge/LibrePower-POWER_Computing-blue)](https://librepower.org)
[![AIX](https://img.shields.io/badge/AIX-7.3+-green)](https://www.ibm.com/power/operating-systems/aix)
[![GridGain](https://img.shields.io/badge/GridGain_CE-8.9.30-orange)](https://www.gridgain.com)
[![Java](https://img.shields.io/badge/JDK-Semeru_25-red)](https://developer.ibm.com/languages/java/semeru-runtimes/)
[![License](https://img.shields.io/badge/License-Apache_2.0-lightgrey)](https://www.apache.org/licenses/LICENSE-2.0)

![GridGain on AIX Demo](demo.gif)

---

## Highlights

- **In-Memory Cache**: Distributed key-value store with sub-millisecond latency
- **SQL Engine**: ANSI SQL queries over in-memory data
- **Compute Grid**: Distribute computation across cluster nodes
- **Java Safe**: Uses its own private JDK — does **NOT** modify your system Java
- **One Command**: `dnf install gridgain` installs everything including JDK

---

## Performance

| Operation | ops/s | Notes |
|-----------|-------|-------|
| **GET** | 5,094 | Thin client, single node |
| **PUT** | 2,758 | Thin client, single node |
| **BULK PUT** | 6,019 | 100-key batches |

> Benchmarked on IBM Power S924 (POWER9), AIX 7.3 TL4, 10K operations

---

## Java Safety

GridGain uses its **own private JDK** (IBM Semeru 25) and does **NOT** modify your system Java.

```
Your system Java:   /usr/java8_64           (UNTOUCHED)
Your app Java:      /usr/java17_64          (UNTOUCHED)
GridGain's JDK:     /opt/freeware/lib/jvm/semeru-25   (private, isolated)
```

The wrapper scripts (`gridgain`, `gridgain-control`, `gridgain-sqlline`) set `JAVA_HOME` internally. No environment variables are modified system-wide.

---

## Installation

### Option 1: DNF (Recommended)

```bash
# Add repository (one-time setup)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install (automatically pulls semeru-jdk25 as dependency)
dnf install gridgain
```

### Option 2: Direct RPM

```bash
# Install JDK first (dependency)
curl -LO https://aix.librepower.org/packages/semeru-jdk25-25.0.2-1.librepower.aix7.1.ppc.rpm
rpm -ivh semeru-jdk25-25.0.2-1.librepower.aix7.1.ppc.rpm

# Install GridGain
curl -LO https://aix.librepower.org/packages/gridgain-8.9.30-2.librepower.aix7.1.ppc.rpm
rpm -ivh gridgain-8.9.30-2.librepower.aix7.1.ppc.rpm
```

---

## Quick Start

```bash
# Start GridGain (background)
gridgain start

# Check status
gridgain status

# SQL client
gridgain-sqlline

# Inside SQLLine:
#   CREATE TABLE city (id INT PRIMARY KEY, name VARCHAR, population INT) WITH "CACHE_NAME=cities";
#   INSERT INTO city VALUES (1, 'New York', 8336817);
#   SELECT * FROM city;
#   !quit

# Stop
gridgain stop
```

### Management Commands

```bash
gridgain start           # Start in background
gridgain stop            # Graceful stop
gridgain status          # Show cluster state
gridgain console         # Start in foreground (debug)
gridgain-control --state # Cluster control utility
gridgain-sqlline         # Interactive SQL client
```

### REST API

```bash
# GridGain exposes a REST API on port 8080
# Create cache and add data
curl "http://localhost:8080/ignite?cmd=getorcreate&cacheName=demo"
curl "http://localhost:8080/ignite?cmd=put&cacheName=demo&key=hello&val=AIX"
curl "http://localhost:8080/ignite?cmd=get&cacheName=demo&key=hello"
# → {"successStatus":0,"response":"AIX"}
```

---

## Features on AIX

| Feature | Status | Notes |
|---------|--------|-------|
| **Key-Value cache** | Working | PUT, GET, REMOVE, BULK operations |
| **SQL engine** | Working | ANSI SQL via SQLLine or JDBC |
| **REST API** | Working | HTTP API on port 8080 |
| **Thin client** | Working | Lightweight TCP client (port 10800) |
| **Compute grid** | Working | Distribute tasks across nodes |
| **Persistence** | Working | Native persistence to disk |
| **Cluster** | Working | Multi-node topology |
| **Control utility** | Working | Cluster state management |

---

## Paths

| Path | Description |
|------|-------------|
| `/opt/freeware/bin/gridgain` | Wrapper script |
| `/opt/freeware/bin/gridgain-control` | Control utility |
| `/opt/freeware/bin/gridgain-sqlline` | SQL client |
| `/opt/freeware/lib/gridgain/` | GridGain home |
| `/opt/freeware/etc/gridgain/gridgain.xml` | Configuration |
| `/opt/freeware/var/lib/gridgain/` | Data directory |
| `/opt/freeware/var/log/gridgain/` | Logs |
| `/opt/freeware/lib/jvm/semeru-25/` | Private JDK |

---

## AIX Port Notes

### Patch: OpenJ9 Compatibility

GridGain uses `HotSpotDiagnosticMXBean.getVMOption()` to query JVM settings. This API is HotSpot-specific and crashes on Eclipse OpenJ9 (the only JVM available for AIX ppc64).

The `ignite-core-8.9.30.jar` includes a patched `IgniteMBeanUtils.class` that:
- Catches all HotSpot-specific exceptions gracefully
- Returns safe defaults (`-1L`) when HotSpot APIs are unavailable
- Uses reflection to avoid `com.sun.management` import conflicts with OpenJ9 modules
- Preserves full functionality when running on HotSpot

Patch source (`IgniteMBeanUtils.java`) is available in `SOURCES/`.

---

## Dependencies

| Package | Version | Purpose |
|---------|---------|---------|
| `semeru-jdk25` | >= 25 | IBM Semeru JDK 25 (OpenJ9) — installed automatically |

---

## Build Information

| | Details |
|--|---------|
| **GridGain** | Community Edition 8.9.30 |
| **Based on** | Apache Ignite |
| **JDK** | IBM Semeru 25.0.2+10 (OpenJ9 0.57.0) |
| **Architecture** | ppc64 (64-bit, big-endian) |
| **RPM Size** | ~221 MB (GridGain) + ~262 MB (JDK) |

### Compatibility

| AIX 7.1 | AIX 7.2 | AIX 7.3 |
|---------|---------|---------|
| ❓      | ❓      | ✅      |

> Tested on AIX 7.3 only. Depends on JDK 25 availability per AIX version.

---

## Package Contents

```
gridgain/
├── RPMS/
│   └── gridgain-8.9.30-2.librepower.aix7.1.ppc.rpm
├── SPECS/
│   └── gridgain.spec
├── SOURCES/
│   └── IgniteMBeanUtils.java      (OpenJ9 compatibility patch)
├── demo.gif                       (terminal demo)
└── README.md
```

### Related Package

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

- GridGain Community Edition: Apache 2.0 (GridGain Systems)
- AIX patches and packaging: Apache 2.0 (LibrePower)
- IBM Semeru JDK: IBM International License Agreement

## Credits

- GridGain by [GridGain Systems](https://www.gridgain.com)
- Apache Ignite by [Apache Software Foundation](https://ignite.apache.org)
- IBM Semeru by [IBM](https://developer.ibm.com/languages/java/semeru-runtimes/)
- AIX port and packaging by [LibrePower](https://librepower.org)

## Links

- **Download**: [aix.librepower.org](https://aix.librepower.org)
- **Source**: [gitlab.com/librepower/aix](https://gitlab.com/librepower/aix)
- **GridGain docs**: [gridgain.com/docs](https://www.gridgain.com/docs/latest/)

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
