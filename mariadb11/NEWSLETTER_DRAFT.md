# MariaDB 11.8.5 LTS for AIX - Newsletter Draft

## Subject Line Options
1. "MariaDB comes to AIX with native thread pool - 83% faster"
2. "First MariaDB for AIX with pool-of-threads support"
3. "MariaDB 11.8.5 LTS now available for POWER9/10/11"

---

## Newsletter Content

### MariaDB 11.8.5 LTS for AIX - Native Thread Pool, Finally

We're excited to announce MariaDB 11.8.5 LTS for AIX - the first MariaDB build with native thread pool support on AIX.

**Why it matters:**

AIX has been stuck with `one-thread-per-connection` mode - a model that collapses under concurrent load. Our port adds native `pollset(2)` support to MariaDB's thread pool, bringing AIX to full parity with Linux.

**The numbers:**
- 83% faster at 100 concurrent clients
- 65% faster at 50 concurrent clients
- Ready for POWER9, POWER10, and POWER11

**Two builds available:**
- **Open XL Build** - Compiled with IBM's Clang/LLVM-based compiler for maximum performance
- **GCC Build** - No external dependencies, works out of the box

**Installation:**
```bash
curl -fsSL https://aix.librepower.org/install.sh | sh
dnf install mariadb11
```

**Technical highlights:**
- Native AIX `pollset(2)` backend for thread pool
- 64K large page support via `MAP_ANON_64K`
- SRC integration for AIX service management
- Vector index (MHNSW) ready with optimized cache settings

The port required just 4 patches - 2 CMake fixes and 2 new features. All patches are open source and ready for upstream contribution.

**Links:**
- Download: https://aix.librepower.org
- Source: https://gitlab.com/librepower/aix/-/tree/main/mariadb11
- Documentation: README in GitLab

---

*LibrePower - Unlocking Power Systems through open source*

---

## Social Media Snippets

**Twitter/X:**
MariaDB 11.8.5 LTS now available for AIX with native thread pool. 83% faster concurrent workloads. Ready for POWER9/10/11. Two builds: Open XL and GCC.

https://aix.librepower.org

#AIX #MariaDB #POWER #IBM #OpenSource

**LinkedIn:**
Excited to announce MariaDB 11.8.5 LTS for IBM AIX - the first build with native thread pool support.

We added pollset(2) backend to MariaDB's thread pool, bringing AIX to full parity with Linux for concurrent database workloads. Result: up to 83% faster performance under load.

Available now at aix.librepower.org with two builds:
- Open XL (IBM's Clang/LLVM compiler) for maximum performance
- GCC for environments without IBM compiler dependencies

Ready for POWER9, POWER10, and POWER11.

#AIX #MariaDB #IBM #Power #Database #OpenSource
