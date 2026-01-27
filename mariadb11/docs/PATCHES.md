# MariaDB 11.8.5 AIX Patches - Technical Details

This document provides detailed technical information about the patches required to compile, run, and optimize MariaDB 11.8.5 on AIX 7.3 POWER.

---

## Overview

MariaDB 11.8.5 requires **4 source code patches** and **1 runtime configuration** to work correctly and optimally on AIX 7.3 POWER:

| Patch | File | Type | Lines Changed |
|-------|------|------|---------------|
| BUG 1 | `storage/perfschema/CMakeLists.txt` | Source | +14 |
| BUG 2 | `storage/perfschema/CMakeLists.txt` | Source | +14 |
| FEATURE 3 | `sql/threadpool_generic.*`, `sql/CMakeLists.txt` | Source | +191 |
| Config | LIBPATH + LDR_CNTRL | Runtime | N/A |
| FEATURE 4 | `mysys/my_largepage.c` | Source | +26 |

---

## Patch 1: pthread_threadid_np Detection

### Problem

CMake's `CHECK_C_SOURCE_COMPILES()` incorrectly detects `pthread_threadid_np()` as available on AIX, even though it's macOS-specific.

**Why it fails on AIX**:
- The test code compiles successfully (no syntax errors)
- But the function doesn't exist in AIX libc at runtime
- Results in 80+ compilation errors in Performance Schema code

### Root Cause

```cmake
CHECK_C_SOURCE_COMPILES("
  #include <pthread.h>
  int main(int ac, char **av)
  {
    unsigned long long tid64;
    pthread_threadid_np(NULL, &tid64);  # ← Compiles but doesn't link
    return (tid64 != 0 ? 0 : 1);
  }"
  HAVE_PTHREAD_THREADID_NP)
```

On AIX, this test **passes** (code compiles) but `pthread_threadid_np` is not in the runtime library.

### Solution

Add platform check to skip detection on AIX:

```cmake
# Check for pthread_threadid_np()
# Note: pthread_threadid_np() is macOS-specific and not available on AIX
IF(NOT CMAKE_SYSTEM_NAME MATCHES "AIX")
  CHECK_C_SOURCE_COMPILES("
  #include <pthread.h>
  int main(int ac, char **av)
  {
    unsigned long long tid64;
    pthread_threadid_np(NULL, &tid64);
    return (tid64 != 0 ? 0 : 1);
  }"
  HAVE_PTHREAD_THREADID_NP)
ELSE()
  SET(HAVE_PTHREAD_THREADID_NP 0)
ENDIF()
```

### Impact

**Files affected**: 28 Performance Schema source files
**Compilation errors prevented**: 80+
**Runtime behavior**: Uses fallback `pthread_self()` implementation

---

## Patch 2: getthrid Detection

### Problem

Similar to Patch 1, CMake incorrectly detects OpenBSD-specific `getthrid()` on AIX.

**Why it fails on AIX**:
- Test code compiles (no syntax errors)
- Function doesn't exist in AIX libc
- Causes compilation failures in Performance Schema

### Root Cause

```cmake
CHECK_C_SOURCE_COMPILES("
  #include <unistd.h>
  int main(int ac, char **av)
  {
    unsigned long long tid = getthrid();  # ← OpenBSD only
    return (tid != 0 ? 0 : 1);
  }"
  HAVE_GETTHRID)
```

### Solution

Add platform check to skip detection on AIX:

```cmake
# Check for getthrid()
# Note: getthrid() is OpenBSD-specific and not available on AIX
IF(NOT CMAKE_SYSTEM_NAME MATCHES "AIX")
  CHECK_C_SOURCE_COMPILES("
  #include <unistd.h>
  int main(int ac, char **av)
  {
    unsigned long long tid = getthrid();
    return (tid != 0 ? 0 : 1);
  }"
  HAVE_GETTHRID)
ELSE()
  SET(HAVE_GETTHRID 0)
ENDIF()
```

### Impact

**Files affected**: 28 Performance Schema source files
**Compilation errors prevented**: 80+
**Runtime behavior**: Uses fallback `pthread_self()` implementation

---

## Runtime Configuration: Threading Library

### Problem

MariaDB uses C++11 threading features (`std::thread`, `std::condition_variable`, `std::mutex`) that are not exported by the standard GCC libstdc++.a on AIX.

**Missing symbols**:
```
_ZTINSt6thread6_StateE
_ZNSt18condition_variableC1Ev
_ZNSt18condition_variableD1Ev
_ZNSt18condition_variable4waitERSt11unique_lockISt5mutexE
_ZNSt18condition_variable10notify_oneEv
_ZNSt18condition_variable10notify_allEv
_ZNSt6thread6_StateD2Ev
_ZNSt6thread6detachEv
_ZNSt6thread15_M_start_threadE...
_ZNSt6thread20hardware_concurrencyEv
```

### Root Cause

GCC on AIX provides two versions of libstdc++.a:

1. **Standard** (`/opt/freeware/lib/gcc/.../libstdc++.a`)
   - Does NOT export threading symbols
   - Used by default

2. **pthread-enabled** (`/opt/freeware/lib/gcc/.../pthread/libstdc++.a`)
   - DOES export threading symbols
   - Requires explicit LIBPATH configuration

### Solution

Set LIBPATH to prioritize pthread-enabled libraries:

```bash
export LIBPATH=/opt/freeware/lib/pthread:\
/opt/freeware/lib/gcc/powerpc-ibm-aix7.3.0.0/13/pthread:\
/opt/freeware/lib:\
/usr/lib
```

**Verification**:
```bash
$ nm /opt/freeware/lib/gcc/powerpc-ibm-aix7.3.0.0/13/pthread/libstdc++.a | grep thread.*State
std::thread::_State::~_State() T   270374368
std::thread::_State::~_State() T   270374368          64
```

### Impact

**Required at**: Runtime
**Affects**: All MariaDB binaries (mariadbd, utilities)
**Implementation**: Wrapper script in RPM package

---

## Patch 4: AIX Large Page Support (`MAP_ANON_64K`)

### Problem

MariaDB's `--large-pages` option has no AIX implementation. The existing code supports Linux (`MAP_HUGETLB`), FreeBSD/macOS (`MAP_ALIGNED`), and Windows, but AIX is treated as a no-op. This means `mmap()` allocations use 4K pages, causing excessive TLB misses on POWER processors for workloads with large memory-mapped regions and random access patterns.

This is particularly impactful for **MHNSW vector index graph traversal**, which performs pointer-chasing across a ~300MB graph structure -- worst case for TLB misses with 4K pages.

### Root Cause

AIX provides `MAP_ANON_64K` (value `0x400`) as an `mmap()` flag to request 64K anonymous pages, compared to the default 4K page size. However, `my_largepage.c` has no `_AIX` code paths.

### Solution

Add three AIX-specific code sections to `mysys/my_largepage.c`:

1. **Define `MAP_ANON_64K`** (if not already defined):
```c
#ifdef _AIX
#ifndef MAP_ANON_64K
#define MAP_ANON_64K 0x400
#endif
#endif
```

2. **Page size detection** (`my_get_large_page_sizes`):
```c
#elif defined(_AIX)
#define my_large_page_sizes_length 2
static size_t my_large_page_sizes[my_large_page_sizes_length];
static void my_get_large_page_sizes(size_t sizes[])
{
  /* AIX supports 64K anonymous pages via MAP_ANON_64K mmap flag */
  sizes[0]= 65536;
  sizes[1]= 0;
}
```

3. **mmap flag injection** in both `my_large_malloc()` and `my_large_virtual_alloc()`:
```c
#elif defined(_AIX) && defined(MAP_ANON_64K)
        /* AIX: use 64K pages for anonymous mappings */
        mapflag|= MAP_ANON_64K;
```

### Impact

**Files affected**: 1 (`mysys/my_largepage.c`)
**Lines added**: 26
**Runtime behavior**: With `--large-pages`, all `mmap` regions use 64K pages on AIX
**Performance**: Reduces TLB misses for memory-intensive workloads on POWER
**Compatibility**: No effect on non-AIX platforms; guarded by `#ifdef _AIX`

### Patch File

**File**: `SOURCES/0004-AIX-large-pages-MAP_ANON_64K.patch`
**Format**: Git-style patch
**Apply with**:
```bash
cd mariadb-11.8.5
patch -p1 < 0004-AIX-large-pages-MAP_ANON_64K.patch
```

### Note on LDR_CNTRL

`MAP_ANON_64K` controls page sizes for `mmap()` allocations only. For the process's text, data, stack, and shared memory segments, AIX uses the `LDR_CNTRL` environment variable:

```bash
export LDR_CNTRL=DATAPSIZE=64K@TEXTPSIZE=64K@STACKPSIZE=64K@SHMPSIZE=64K
```

The RPM wrapper script (`mariadbd`) sets both: `LDR_CNTRL` for process segments and `--large-pages` for mmap regions.

---

## Patch Files

All source code changes are available as unified diffs in `SOURCES/`:

| Patch File | Description |
|-----------|-------------|
| `mariadb-aix-perfschema.patch` | CMake bug fixes (patches 1 & 2) |
| `threadpool_aix_pollset.patch` | AIX pollset thread pool (patch 3) |
| `0004-AIX-large-pages-MAP_ANON_64K.patch` | AIX large pages (patch 4) |

**Apply all**:
```bash
cd mariadb-11.8.5
patch -p1 < mariadb-aix-perfschema.patch
patch -p1 < threadpool_aix_pollset.patch
patch -p1 < 0004-AIX-large-pages-MAP_ANON_64K.patch
```

---

## Testing Results

### Compilation

```bash
$ cmake /tmp/mariadb-11.8.0 -DCMAKE_BUILD_TYPE=Release
$ gmake -j96
...
[100%] Built target mariadbd
[100%] Built target libserver.so
```

**Result**: ✅ Clean build, no errors

### Binary Check

```bash
$ ldd sql/mariadbd
mariadbd needs:
  /tmp/mariadb-build/sql/libserver.so
  /opt/freeware/lib/gcc/.../pthread/libstdc++.a(libstdc++.so.6)
  /usr/lib/libc.a(shr_64.o)
  ...
```

**Result**: ✅ Links correctly with pthread libstdc++

### Version Check

```bash
$ export LIBPATH=/opt/freeware/lib/pthread:...
$ ./mariadbd --version
mariadbd Ver 11.8.0-MariaDB for AIX5.7.3 on powerpc (Source distribution)
```

**Result**: ✅ Executes successfully

### Database Initialization

```bash
$ ./bin/mysql_install_db --basedir=/tmp/test --datadir=/tmp/test/data
Installing MariaDB/MySQL system tables in '/tmp/test/data' ...
OK
```

**Result**: ✅ Database initializes successfully

---

## Platform Detection Logic

After patches, the thread ID detection follows this order:

1. ~~`pthread_threadid_np()`~~ - ❌ Skipped on AIX (macOS only)
2. ~~`gettid()`~~ - ❌ Not available (Linux glibc 2.30+)
3. ~~`SYS_gettid`~~ - ❌ Not available (Linux syscall)
4. ~~`GetCurrentThreadId()`~~ - ❌ Not available (Windows)
5. ~~`pthread_getthreadid_np()`~~ - ❌ Not available (FreeBSD)
6. ~~`getthrid()`~~ - ❌ Skipped on AIX (OpenBSD only)
7. **`pthread_self()`** - ✅ **Used on AIX** (POSIX fallback)

The resulting `pfs_config.h`:
```c
/* #undef HAVE_PTHREAD_THREADID_NP */
/* #undef HAVE_SYS_GETTID */
/* #undef HAVE_GETTID */
/* #undef HAVE_GETTHRID */
/* #undef HAVE_PTHREAD_GETTHREADID_NP */
#define HAVE_INTEGER_PTHREAD_SELF 1  // ← Selected for AIX
```

---

## Why These Patches Are Minimal

These patches follow the principle of **minimal invasive changes**:

1. **No code changes to Performance Schema** - only CMake configuration
2. **No new code** - only guards around existing detection logic
3. **No AIX-specific implementations** - uses existing POSIX fallback
4. **No conditional compilation** - just prevents wrong feature detection

This makes them ideal candidates for **upstream inclusion** in the official MariaDB distribution.

---

## Upstream Submission Status

**Status**: Ready for submission
**Target**: MariaDB 11.9.x or 11.8.x bugfix release
**JIRA**: TBD
**Pull Request**: TBD

See [UPSTREAM_PR_GUIDE.md](./UPSTREAM_PR_GUIDE.md) for the submission process.

---

## References

- [MariaDB Performance Schema Documentation](https://mariadb.com/kb/en/performance-schema/)
- [CMake CHECK_C_SOURCE_COMPILES](https://cmake.org/cmake/help/latest/module/CheckCSourceCompiles.html)
- [pthread_self() POSIX Specification](https://pubs.opengroup.org/onlinepubs/9699919799/)
- [AIX Thread Identification](https://www.ibm.com/docs/en/aix/7.3?topic=p-pthread-self-subroutine)

---

**Author**: LibrePower <hello@librepower.org>

**Last Updated**: 2026-01-27

**MariaDB Version**: 11.8.5

**AIX Version**: 7.3 TL4

**Compiler**: GCC 13.3.0

---

## 📬 Stay Updated

**📮 [Subscribe to LibrePower Newsletter](https://librepower.substack.com)**
Get AIX package updates

**🌐 [Visit LibrePower.org](https://librepower.org)**
More AIX POWER packages

**📦 [AIX Repository](https://aix.librepower.org)**
Download RPMs
