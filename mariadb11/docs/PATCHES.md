# MariaDB 11.8.0 AIX Patches - Technical Details

This document provides detailed technical information about the patches required to compile and run MariaDB 11.8.0 on AIX.

---

## Overview

MariaDB 11.8.0 requires **2 source code patches** and **1 runtime configuration** to work correctly on AIX 7.3 POWER:

| Patch | File | Type | Lines Changed |
|-------|------|------|---------------|
| BUG 1 | `storage/perfschema/CMakeLists.txt` | Source | +14 |
| BUG 2 | `storage/perfschema/CMakeLists.txt` | Source | +14 |
| Config | LIBPATH | Runtime | N/A |

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

## Unified Patch File

The source code changes are available as a unified diff:

**File**: `mariadb-aix-perfschema.patch`
**Format**: Git unified diff
**Size**: ~50 lines
**Apply with**:
```bash
cd mariadb-11.8.0
patch -p1 < mariadb-aix-perfschema.patch
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

**Last Updated**: 2026-01-13

**MariaDB Version**: 11.8.0

**AIX Version**: 7.3 TL5 SP2

**Compiler**: GCC 13.3.0

---

## 📬 Stay Updated

**📮 [Subscribe to LibrePower Newsletter](https://librepower.substack.com)**
Get AIX package updates

**🌐 [Visit LibrePower.org](https://librepower.org)**
More AIX POWER packages

**📦 [AIX Repository](https://aix.librepower.org)**
Download RPMs
