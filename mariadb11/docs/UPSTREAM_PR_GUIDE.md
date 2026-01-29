# MariaDB AIX Support - Upstream Submission Guide

**Target**: MariaDB 11.8.x and later
**Patches**: 3 (1 bug fix + 2 features)
**Status**: Ready for submission

---

## Overview

This guide explains how to submit the AIX patches to upstream MariaDB. Anyone with access to an AIX system can reproduce and validate these patches.

### Patches Summary

| # | File | Type | JIRA Type | Description |
|---|------|------|-----------|-------------|
| 1 | `mariadb-aix-perfschema.patch` | Bug fix | Bug | Fix Performance Schema compilation on AIX |
| 2 | `threadpool_aix_pollset.patch` | Feature | New Feature | Native AIX thread pool using pollset(2) |
| 3 | `0004-AIX-large-pages-MAP_ANON_64K.patch` | Feature | New Feature | AIX large page support (64K pages) |

---

## Prerequisites

### Environment

- AIX 7.3 or later (POWER9/10/11)
- GCC 13+ or IBM Open XL C/C++ 17+
- CMake 3.20+
- Git

### Get MariaDB Source

```bash
git clone --branch mariadb-11.8.5 --depth 1 https://github.com/MariaDB/server.git
cd server
git submodule update --init --recursive
```

### Get Patches

```bash
# From LibrePower repository
git clone https://gitlab.com/librepower/aix.git librepower-aix
cd librepower-aix/mariadb11/SOURCES/

# Patches available:
ls -la *.patch
# mariadb-aix-perfschema.patch      (66 lines)
# threadpool_aix_pollset.patch      (269 lines)
# 0004-AIX-large-pages-MAP_ANON_64K.patch (81 lines)
```

---

## Patch 1: Performance Schema Bug Fix

### Description

MariaDB fails to compile on AIX because CMake incorrectly detects `pthread_threadid_np()` (macOS-only) and `getthrid()` (OpenBSD-only) as available.

### Apply and Test

```bash
cd /path/to/mariadb-server

# Apply patch
patch -p1 < /path/to/mariadb-aix-perfschema.patch

# Verify
grep -A5 "CMAKE_SYSTEM_NAME.*AIX" storage/perfschema/CMakeLists.txt
```

### Build Verification

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
# Should complete without pthread_threadid_np/getthrid errors
```

### JIRA Ticket

**URL**: https://jira.mariadb.org/
**Project**: MDEV
**Type**: Bug
**Priority**: Major
**Summary**: `AIX: Performance Schema fails to compile due to incorrect platform detection`

**Description**:
```
MariaDB 11.8.x fails to compile on AIX due to incorrect platform detection
in the Performance Schema component.

Two functions are incorrectly detected as available on AIX:
1. pthread_threadid_np() - macOS-specific
2. getthrid() - OpenBSD-specific

CMake's CHECK_C_SOURCE_COMPILES test passes (code compiles) but the functions
don't exist at runtime, causing 80+ compilation errors in Performance Schema.

Environment:
- OS: AIX 7.3 TL4
- Architecture: POWER9 (64-bit)
- Compiler: GCC 13.3.0

Error example:
  /storage/perfschema/my_thread.h:50:3: error: 'pthread_threadid_np' was not declared

Solution: Add platform guards to skip these checks on AIX, using POSIX pthread_self() fallback.

Patch: https://gitlab.com/librepower/aix/-/blob/main/mariadb11/SOURCES/mariadb-aix-perfschema.patch
```

### Commit Message

```
MDEV-XXXXX: Fix Performance Schema compilation on AIX

The Performance Schema component incorrectly detects pthread_threadid_np()
and getthrid() as available on AIX, causing compilation failures.

pthread_threadid_np() is macOS-specific and getthrid() is OpenBSD-specific.
CMake's CHECK_C_SOURCE_COMPILES test passes on AIX (the code compiles) but
the functions don't exist at runtime, causing 80+ compilation errors.

This patch adds platform-specific guards to exclude these checks on AIX,
allowing MariaDB to correctly fall back to the POSIX-standard pthread_self().

Tested on: AIX 7.3 TL4, POWER9, GCC 13.3.0

Signed-off-by: Your Name <your@email.com>
```

---

## Patch 2: AIX Thread Pool (pollset)

### Description

Adds native AIX thread pool support using `pollset(2)`, enabling `thread_handling=pool-of-threads` on AIX. Without this patch, AIX can only use `one-thread-per-connection`.

### Technical Details

AIX `pollset(2)` is level-triggered only (no ONESHOT like Linux epoll). This implementation:

1. **ONESHOT simulation**: Removes fds from pollset after events, re-adds when ready
2. **Concurrency protection**: Per-pollset mutex to prevent duplicate fd delivery

### Apply and Test

```bash
cd /path/to/mariadb-server

# Apply patch
patch -p1 < /path/to/threadpool_aix_pollset.patch

# Verify new files
ls sql/threadpool_aix.cc  # Should exist after patch
grep "HAVE_POLLSET" sql/CMakeLists.txt
```

### Build Verification

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Verify thread pool is enabled
./sql/mariadbd --help --verbose | grep thread_handling
# Should show: pool-of-threads
```

### Runtime Test

```bash
# Start with thread pool
./sql/mariadbd --thread_handling=pool-of-threads --thread_pool_size=8 &

# Test with concurrent connections
mariadb -e "SHOW VARIABLES LIKE 'thread_handling';"
# Should show: pool-of-threads
```

### Performance Results

Benchmarked on IBM Power S924 (POWER9), AIX 7.3 TL4:

| Workload | one-thread-per-connection | pool-of-threads | Improvement |
|----------|---------------------------|-----------------|-------------|
| Mixed 50 clients | 5.4s | 1.9s | 65% faster |
| Mixed 100 clients | 11.3s | 2.0s | 83% faster |

### JIRA Ticket

**URL**: https://jira.mariadb.org/
**Project**: MDEV
**Type**: New Feature
**Priority**: Normal
**Component**: Thread pool
**Summary**: `AIX: Add native thread pool support using pollset(2)`

**Description**:
```
This patch adds native AIX thread pool support using pollset(2), enabling
thread_handling=pool-of-threads on IBM AIX systems.

Background:
MariaDB's thread pool currently supports Linux (epoll), Windows (IOCP),
FreeBSD/macOS (kqueue), and Solaris (event ports). AIX was not supported,
forcing AIX users to use one-thread-per-connection.

Implementation:
AIX provides pollset(2) as its scalable I/O multiplexing mechanism. However,
unlike Linux epoll with EPOLLONESHOT, AIX pollset is level-triggered only.

This implementation solves two challenges:

1. ONESHOT simulation:
   - Remove fds from pollset after receiving events (PS_DELETE)
   - Re-add when ready for more data (PS_ADD in io_poll_start_read)

2. Concurrency protection:
   MariaDB's thread pool has concurrent code paths calling io_poll_wait
   on the same pollset. Without protection, the same fd can be delivered
   to multiple threads. Solution: per-pollset mutex serialization.

Testing:
- 1,000 concurrent clients sustained for 30 minutes: PASS
- Mixed read/write workload: 83% faster than one-thread-per-connection
- Tested on AIX 7.3 TL4, POWER9, GCC 13.3.0 and IBM Open XL 17.1.3

Patch: https://gitlab.com/librepower/aix/-/blob/main/mariadb11/SOURCES/threadpool_aix_pollset.patch
```

### Commit Message

```
MDEV-XXXXX: Add AIX thread pool support using pollset(2)

This patch adds native AIX thread pool support, enabling
thread_handling=pool-of-threads on IBM AIX systems.

AIX provides pollset(2) as its scalable I/O multiplexing mechanism.
Unlike Linux epoll with EPOLLONESHOT, AIX pollset is level-triggered
only. This implementation handles two challenges:

1. ONESHOT simulation: Remove fds from pollset after events (PS_DELETE),
   re-add when ready (PS_ADD in io_poll_start_read).

2. Concurrency: Per-pollset mutex to prevent duplicate fd delivery when
   multiple threads call io_poll_wait on the same pollset.

Performance (IBM Power S924, AIX 7.3, 100 clients):
- one-thread-per-connection: 11.3s
- pool-of-threads: 2.0s (83% faster)

Tested: 1,000 concurrent clients, 30 min sustained load, 0 errors.

Signed-off-by: Your Name <your@email.com>
```

---

## Patch 3: AIX Large Pages (MAP_ANON_64K)

### Description

Adds AIX large page support using `MAP_ANON_64K` flag for mmap(). This reduces TLB misses for memory-intensive workloads on POWER processors.

### Technical Details

- AIX default page size: 4K
- AIX large page via `MAP_ANON_64K`: 64K
- Reduces TLB misses for workloads with large memory regions and random access patterns

### Apply and Test

```bash
cd /path/to/mariadb-server

# Apply patch
patch -p1 < /path/to/0004-AIX-large-pages-MAP_ANON_64K.patch

# Verify
grep -A10 "_AIX" mysys/my_largepage.c
```

### Build Verification

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Verify large pages option exists
./sql/mariadbd --help --verbose | grep large-pages
```

### Runtime Test

```bash
# Start with large pages
./sql/mariadbd --large-pages &

# Verify in error log or SHOW VARIABLES
mariadb -e "SHOW VARIABLES LIKE 'large_pages';"
```

### JIRA Ticket

**URL**: https://jira.mariadb.org/
**Project**: MDEV
**Type**: New Feature
**Priority**: Normal
**Component**: InnoDB, Memory allocation
**Summary**: `AIX: Add large page support using MAP_ANON_64K`

**Description**:
```
This patch adds AIX large page support for mmap() allocations using
the MAP_ANON_64K flag.

Background:
MariaDB's --large-pages option supports Linux (MAP_HUGETLB), FreeBSD/macOS
(MAP_ALIGNED), and Windows. AIX was not implemented, treating it as a no-op.

On AIX, mmap() uses 4K pages by default. For workloads with large memory-mapped
regions and random access patterns, 4K pages cause excessive TLB misses on
POWER processors.

Implementation:
AIX provides MAP_ANON_64K (value 0x400) as an mmap() flag to request 64K
anonymous pages. This patch:

1. Defines MAP_ANON_64K if not already defined
2. Adds AIX implementation of my_get_large_page_sizes() (returns 64K)
3. Uses MAP_ANON_64K in my_large_malloc() and my_large_virtual_alloc()

Use case:
Particularly beneficial for MHNSW vector index graph traversal, which
performs pointer-chasing across large memory regions.

Tested on: AIX 7.3 TL4, POWER9, GCC 13.3.0

Patch: https://gitlab.com/librepower/aix/-/blob/main/mariadb11/SOURCES/0004-AIX-large-pages-MAP_ANON_64K.patch
```

### Commit Message

```
MDEV-XXXXX: Add AIX large page support using MAP_ANON_64K

Adds AIX large page support for mmap() allocations. AIX provides
MAP_ANON_64K (0x400) to request 64K pages instead of the default 4K.

This reduces TLB misses for workloads with large memory-mapped regions
and random access patterns on POWER processors.

Changes:
- Define MAP_ANON_64K if not present on AIX
- Add my_get_large_page_sizes() for AIX (reports 64K)
- Use MAP_ANON_64K in my_large_malloc() and my_large_virtual_alloc()

Tested on: AIX 7.3 TL4, POWER9

Signed-off-by: Your Name <your@email.com>
```

---

## Submission Process

### Step 1: Create JIRA Tickets

1. Go to https://jira.mariadb.org/
2. Create account if needed (free)
3. Create 3 tickets using templates above
4. Note the MDEV-XXXXX numbers assigned

### Step 2: Fork Repository

```bash
# Fork on GitHub: https://github.com/MariaDB/server
# Then clone your fork
git clone https://github.com/YOUR_USERNAME/server.git
cd server
git remote add upstream https://github.com/MariaDB/server.git
```

### Step 3: Create Branches and Apply Patches

```bash
# Patch 1: Performance Schema (Bug)
git checkout -b aix-perfschema-fix upstream/11.8
patch -p1 < mariadb-aix-perfschema.patch
git add -A
git commit  # Use commit message from above, replace MDEV-XXXXX

# Patch 2: Thread Pool (Feature)
git checkout -b aix-threadpool upstream/11.8
patch -p1 < threadpool_aix_pollset.patch
git add -A
git commit  # Use commit message from above

# Patch 3: Large Pages (Feature)
git checkout -b aix-largepages upstream/11.8
patch -p1 < 0004-AIX-large-pages-MAP_ANON_64K.patch
git add -A
git commit  # Use commit message from above
```

### Step 4: Push and Create PRs

```bash
git push origin aix-perfschema-fix
git push origin aix-threadpool
git push origin aix-largepages
```

Create 3 Pull Requests on GitHub:
- Base: `MariaDB/server:11.8`
- Compare: `YOUR_USERNAME:aix-perfschema-fix` (etc.)

### Step 5: PR Description Template

```markdown
## Summary
[Brief description]

## JIRA
MDEV-XXXXX

## Testing
- AIX 7.3 TL4, POWER9
- Compiler: GCC 13.3.0
- Build: Success
- Runtime: Verified

## Patch
https://gitlab.com/librepower/aix/-/blob/main/mariadb11/SOURCES/[patch-name].patch

## Full Documentation
https://gitlab.com/librepower/aix/-/blob/main/mariadb11/docs/PATCHES.md
```

---

## Reproduction Steps (For Reviewers)

Anyone can reproduce these patches on AIX:

```bash
# 1. Get MariaDB source
git clone --branch mariadb-11.8.5 https://github.com/MariaDB/server.git
cd server && git submodule update --init

# 2. Get patches
curl -LO https://gitlab.com/librepower/aix/-/raw/main/mariadb11/SOURCES/mariadb-aix-perfschema.patch
curl -LO https://gitlab.com/librepower/aix/-/raw/main/mariadb11/SOURCES/threadpool_aix_pollset.patch
curl -LO https://gitlab.com/librepower/aix/-/raw/main/mariadb11/SOURCES/0004-AIX-large-pages-MAP_ANON_64K.patch

# 3. Apply all patches
patch -p1 < mariadb-aix-perfschema.patch
patch -p1 < threadpool_aix_pollset.patch
patch -p1 < 0004-AIX-large-pages-MAP_ANON_64K.patch

# 4. Build
mkdir build && cd build
export OBJECT_MODE=64
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_C_FLAGS="-mcpu=power9" \
         -DCMAKE_CXX_FLAGS="-mcpu=power9"
make -j$(nproc)

# 5. Test
export LIBPATH=/path/to/build/sql:$LIBPATH
./sql/mariadbd --version
# Expected: mariadbd Ver 11.8.5-MariaDB for AIX...
```

---

## Contact

**Maintainer**: LibrePower Project
**Email**: hello@librepower.org
**Repository**: https://gitlab.com/librepower/aix
**Website**: https://librepower.org

---

## License

All patches are submitted under GPL v2 (MariaDB's license).

---

**Last Updated**: 2026-01-29
