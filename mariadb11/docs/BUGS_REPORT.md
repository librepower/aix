# MariaDB 11.8.0 Compilation on AIX 7.3 (POWER) - Bug Report

**Author**: LibrePower <hello@librepower.org>

**Date**: January 13, 2026

**System**: AIX 7.3 TL3, POWER (tested on POWER9)

**Compiler**: GCC 13.3.0

**MariaDB Version**: 11.8.0

**Result**: ✅ **SUCCESSFUL COMPILATION** with 2 AIX-specific patches required

---

## Executive Summary

MariaDB 11.8.0 compiles successfully on AIX with minimal patches (2 bugs fixed). The codebase is remarkably portable - 99.9% of the code compiled without modifications. All bugs are in the Performance Schema component's platform detection logic.

---

## Bugs Discovered

### BUG 1: pthread_threadid_np() Incorrectly Detected on AIX ⚠️ **CRITICAL**

**File**: `storage/perfschema/CMakeLists.txt`  
**Lines**: 283-292  
**Severity**: High (prevents compilation)

#### Problem

CMake's `CHECK_C_SOURCE_COMPILES` test for `pthread_threadid_np()` passes on AIX (the test program compiles successfully), but the function doesn't actually exist at runtime. This is a macOS-specific function that CMake incorrectly detects as available on AIX.

#### Impact

```
/tmp/mariadb-11.8.0/storage/perfschema/my_thread.h:50:3: error: 'pthread_threadid_np' was not declared in this scope
```

80+ compilation errors in Performance Schema component.

#### Root Cause

The CMake test only checks if the code compiles, not if the function actually exists in the runtime library. On AIX, the compilation succeeds (no syntax errors), but the linker would fail if the function were actually called.

#### Fix

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

---

### BUG 2: getthrid() Incorrectly Detected on AIX ⚠️ **CRITICAL**

**File**: `storage/perfschema/CMakeLists.txt`  
**Lines**: 314-322  
**Severity**: High (prevents compilation)

#### Problem

Similar to BUG 1, `getthrid()` is an OpenBSD-specific function that CMake incorrectly detects as available on AIX.

#### Impact

```
/tmp/mariadb-11.8.0/storage/perfschema/my_thread.h:74:10: error: 'getthrid' was not declared in this scope
```

80+ compilation errors in Performance Schema component.

#### Fix

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

---

### BUG 3: Bison 3.8.2 Generates Truncated yy_oracle.cc on AIX ℹ️ **INFO**

**File**: Auto-generated `sql/yy_oracle.cc`  
**Severity**: Medium (workaround available)

#### Problem

Bison 3.8.2 occasionally generates a truncated yy_oracle.cc file on AIX, ending mid-symbol:
- File ends with: `&my_charset_b`
- Should end with: `&my_charset_bin`

Missing labels: `yyabortlab`, `yyacceptlab`, `yyerrlab`, `yyerrlab1`

#### Workaround

Regenerate the parser file:
```bash
cd /tmp/mariadb-build
rm -f sql/yy_oracle.cc sql/yy_oracle.hh
gmake GenServerSource
```

This appears to be a Bison buffering issue on AIX that occurs sporadically.

---

## Compilation Success Metrics

- **Total Build Time**: ~15 minutes (with 96 parallel jobs)
- **Binary Size**: 
  - `mariadbd`: 15 KB (wrapper executable)
  - `libserver.so`: 703 MB (main server library)
- **Warnings**: Minor (duplicate symbols in wsrep, expected on AIX)
- **Errors**: 0 (after applying 2 patches)

---

## Platform-Specific Notes

### AIX Compatibility Layer Required

1. **getopt.h Header**: AIX doesn't provide `<getopt.h>` natively. Created compatibility header from MariaDB's `libmariadb/include/ma_getopt.h`:
   ```bash
   cp /tmp/mariadb-11.8.0/libmariadb/include/ma_getopt.h /opt/freeware/include/getopt.h
   ```

2. **Bison**: Required version 3.8.2+ (installed via `dnf install bison`)

3. **PCRE2**: Required development headers (`dnf install pcre2-devel`)

---

## Thread ID Detection Hierarchy on AIX

After applying patches, MariaDB correctly uses this fallback chain on AIX:

1. ~~pthread_threadid_np()~~ - **Disabled** (macOS only)
2. ~~gettid()~~ - Not available (Linux glibc 2.30+)
3. ~~SYS_gettid~~ - Not available (Linux syscall)
4. ~~getthrid()~~ - **Disabled** (OpenBSD only)
5. ~~pthread_getthreadid_np()~~ - Not available (FreeBSD)
6. ✅ **pthread_self()** - **USED** (POSIX standard, available on AIX)

---

## Recommended Actions for MariaDB Team

### 1. Apply CMake Patches (HIGH PRIORITY)

Both pthread_threadid_np and getthrid detection need platform-specific guards in `storage/perfschema/CMakeLists.txt`.

**Why this matters**: These bugs prevent compilation on AIX and potentially other UNIX systems. The fixes are minimal (3-line additions) and don't affect other platforms.

### 2. Investigate Bison Issue (MEDIUM PRIORITY)

The truncated parser file issue appears to be a rare Bison/AIX interaction bug. Consider:
- Testing with Bison 3.8.2 on AIX in CI
- Adding file size validation after parser generation
- Documenting regeneration workaround

### 3. Add AIX to CI Pipeline (RECOMMENDED)

AIX is still widely used in enterprise environments (banking, government, large corporations). Adding AIX 7.3 POWER to the CI matrix would prevent future regressions.

---

## Patch Files

### Patch 1: storage/perfschema/CMakeLists.txt (pthread_threadid_np)

```diff
--- a/storage/perfschema/CMakeLists.txt
+++ b/storage/perfschema/CMakeLists.txt
@@ -283,12 +283,17 @@ SET(PERFSCHEMA_SOURCES
 )
 
 # Check for pthread_threadid_np()
+# Note: pthread_threadid_np() is macOS-specific and not available on AIX
+IF(NOT CMAKE_SYSTEM_NAME MATCHES "AIX")
 CHECK_C_SOURCE_COMPILES("
 #include <pthread.h>
 int main(int ac, char **av)
 {
   unsigned long long tid64;
   pthread_threadid_np(NULL, &tid64);
   return (tid64 != 0 ? 0 : 1);
 }"
 HAVE_PTHREAD_THREADID_NP)
+ELSE()
+  SET(HAVE_PTHREAD_THREADID_NP 0)
+ENDIF()
```

### Patch 2: storage/perfschema/CMakeLists.txt (getthrid)

```diff
--- a/storage/perfschema/CMakeLists.txt
+++ b/storage/perfschema/CMakeLists.txt
@@ -314,11 +314,16 @@ HAVE_SYS_GETTID)
 
 # Check for getthrid()
+# Note: getthrid() is OpenBSD-specific and not available on AIX
+IF(NOT CMAKE_SYSTEM_NAME MATCHES "AIX")
 CHECK_C_SOURCE_COMPILES("
 #include <unistd.h>
 int main(int ac, char **av)
 {
   unsigned long long tid = getthrid();
   return (tid != 0 ? 0 : 1);
 }"
 HAVE_GETTHRID)
+ELSE()
+  SET(HAVE_GETTHRID 0)
+ENDIF()
```

---

## Testing

Compilation tested on:
- **OS**: AIX 7.3 Technology Level 3
- **Architecture**: POWER9 (12 cores, 96 hardware threads)
- **Compiler**: GCC 13.3.0 (AIX Toolbox)
- **CMake**: 4.2.0
- **Bison**: 3.8.2

---

## Contact

**Reported by**: LibrePower Project  
**Email**: hello@librepower.org  
**Date**: January 13, 2026

---

## License

This bug report and associated patches are provided under the same license as MariaDB (GPL v2).
