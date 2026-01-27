# MariaDB AIX Support - Upstream PR Submission Guide

**Author**: LibrePower <hello@librepower.org>

**Repository**: https://gitlab.com/librepower/mariadb

**Status**: Ready for submission

**Target**: MariaDB 11.8.x and later

**Priority**: High (blocks AIX compilation)

---

## Pre-Submission Checklist

- [x] Bugs identified and documented
- [x] Patches created and tested
- [x] Compilation successful on AIX 7.3 POWER
- [x] Bug report written
- [ ] JIRA ticket created
- [ ] GitHub fork created
- [ ] Pull Request submitted
- [ ] Follow-up with MariaDB team

---

## Step 1: Create JIRA Ticket

### Navigate to JIRA
- URL: https://jira.mariadb.org/
- Create account if needed (free)

### Ticket Details

**Project**: MDEV (MariaDB Server)  
**Issue Type**: Bug  
**Summary**: `AIX: Performance Schema fails to compile due to incorrect platform detection`

**Priority**: Major (blocks compilation on entire platform)

**Affects Version/s**: 11.8.0 (likely affects all versions)

**Component/s**: Performance Schema

**Description**:
```
MariaDB 11.8.0 fails to compile on AIX 7.3 due to incorrect platform detection 
in the Performance Schema component's CMakeLists.txt.

Two macOS/BSD-specific functions are incorrectly detected as available on AIX:
1. pthread_threadid_np() (macOS-specific)
2. getthrid() (OpenBSD-specific)

CMake's CHECK_C_SOURCE_COMPILES test passes (code compiles) but the functions 
don't exist at runtime, causing 80+ compilation errors.

Environment:
- OS: AIX 7.3 Technology Level 3
- Architecture: POWER9 (64-bit)
- Compiler: GCC 13.3.0
- CMake: 4.2.0

Error Examples:
```
/storage/perfschema/my_thread.h:50:3: error: 'pthread_threadid_np' was not declared
/storage/perfschema/my_thread.h:74:10: error: 'getthrid' was not declared
```

Impact: Complete compilation failure on AIX platform.

Solution: Add platform-specific guards to exclude these checks on AIX, allowing 
fallback to POSIX-standard pthread_self().

Patch available at: https://gitlab.com/librepower/mariadb
```

**Attachment**: Upload `MARIADB_AIX_BUGS_REPORT.md`

---

## Step 2: Prepare Git Repository

### Fork Repository

Choose one approach:

#### Option A: GitHub Fork (Recommended)
```bash
# Fork on GitHub UI: https://github.com/MariaDB/server
# Then clone your fork
git clone https://github.com/YOUR_USERNAME/server.git mariadb-server
cd mariadb-server
```

#### Option B: GitLab Mirror Fork
```bash
# Fork on GitLab UI: https://gitlab.com/mariadb/server
# Then clone
git clone git@gitlab.com:librepower/mariadb.git mariadb-server
cd mariadb-server
```

### Configure Git Identity

```bash
cd mariadb-server
git config user.name "LibrePower"
git config user.email "hello@librepower.org"
```

### Add Upstream Remote

```bash
git remote add upstream https://github.com/MariaDB/server.git
git fetch upstream
```

---

## Step 3: Create Feature Branch

```bash
# Start from latest 11.8 branch
git checkout -b aix-perfschema-fix upstream/11.8

# Or from main if targeting next release
git checkout -b aix-perfschema-fix upstream/main
```

---

## Step 4: Apply Patches

### Download Patches from AIX Server

```bash
scp aixlibrepower:/tmp/mariadb-11.8.0/storage/perfschema/CMakeLists.txt storage/perfschema/
```

Or apply manually:

```bash
# Edit storage/perfschema/CMakeLists.txt
# Apply changes from /tmp/mariadb-aix-perfschema.patch
```

---

## Step 5: Create Commit

### Commit Message Format

MariaDB requires specific format: `MDEV-XXXXX: Brief description`

```bash
git add storage/perfschema/CMakeLists.txt

git commit -m "MDEV-XXXXX: Fix Performance Schema compilation on AIX

The Performance Schema component incorrectly detects pthread_threadid_np()
and getthrid() as available on AIX, causing compilation failures.

pthread_threadid_np() is macOS-specific and getthrid() is OpenBSD-specific.
CMake's CHECK_C_SOURCE_COMPILES test passes on AIX (the code compiles) but
the functions don't exist at runtime, causing 80+ compilation errors.

This patch adds platform-specific guards to exclude these checks on AIX,
allowing MariaDB to correctly fall back to the POSIX-standard pthread_self().

Root Cause:
- CMake CHECK_C_SOURCE_COMPILES only tests compilation, not runtime availability
- AIX headers allow these functions to compile but they're not in runtime libraries

Changes:
- storage/perfschema/CMakeLists.txt: Added IF(NOT CMAKE_SYSTEM_NAME MATCHES AIX) 
  guards around pthread_threadid_np() and getthrid() detection

Testing:
- Compiled successfully on AIX 7.3 POWER9 with GCC 13.3.0
- All 80+ previous errors resolved
- Performance Schema correctly uses pthread_self() fallback

Signed-off-by: LibrePower <hello@librepower.org>"
```

---

## Step 6: Push to Your Fork

```bash
# Push to your fork
git push origin aix-perfschema-fix
```

---

## Step 7: Create Pull Request

### On GitHub

1. Navigate to https://github.com/MariaDB/server
2. Click "New Pull Request"
3. Select: `base: 11.8` ← `compare: YOUR_USERNAME:aix-perfschema-fix`
4. Fill PR details:

**Title**: `MDEV-XXXXX: Fix Performance Schema compilation on AIX`

**Description**:
```markdown
## Summary

Fixes compilation failure of Performance Schema component on AIX platform.

## Issue

MariaDB 11.8.0 fails to compile on AIX 7.3 due to incorrect detection of 
platform-specific thread ID functions in Performance Schema's CMake configuration.

JIRA: MDEV-XXXXX

## Root Cause

Two functions are incorrectly detected as available on AIX:

1. **pthread_threadid_np()** - macOS-specific function
2. **getthrid()** - OpenBSD-specific function

CMake's `CHECK_C_SOURCE_COMPILES` test passes on AIX because the test code 
compiles successfully, but the functions don't actually exist in AIX's runtime 
libraries. This causes 80+ compilation errors when building Performance Schema.

## Solution

Added platform-specific guards in `storage/perfschema/CMakeLists.txt` to skip 
these checks on AIX, allowing fallback to POSIX-standard `pthread_self()`.

## Testing

- ✅ Compiled successfully on AIX 7.3 TL3 (POWER9, 64-bit)
- ✅ Compiler: GCC 13.3.0
- ✅ CMake: 4.2.0
- ✅ Build: libserver.so (703 MB) generated successfully
- ✅ All previous compilation errors resolved

## Files Changed

- `storage/perfschema/CMakeLists.txt` (+14 lines, -2 lines)

## Additional Information

Complete bug report and testing documentation available at:
https://gitlab.com/librepower/mariadb/-/blob/main/MARIADB_AIX_BUGS_REPORT.md

## License

This contribution is submitted under the BSD 3-Clause License as required by 
MariaDB Foundation.
```

5. Submit Pull Request

---

## Step 8: Follow-Up

### Monitor PR Status

- Check GitHub notifications
- Respond to review comments promptly
- Be ready to make adjustments

### Expected Timeline

- Initial reply: Within 1-2 weeks (guaranteed by MariaDB)
- Review: 2-4 weeks (depends on complexity and workload)
- Merge: 4-8 weeks if approved

### Communication

- Be professional and responsive
- Explain technical decisions clearly
- Offer to provide additional testing if needed

---

## Backup Plan: Direct Patch Submission

If PR process is too slow, can also:

1. Email patch to mariadb-developers mailing list
2. Post on MariaDB community forums
3. Reach out to specific maintainers on JIRA ticket

---

## Post-Merge Actions

Once merged:

1. Update LibrePower documentation
2. Blog post announcing AIX support in MariaDB
3. Update RPM spec to use upstream version (no patches needed)
4. Announce on social media

---

## Files to Include

Located on AIX server at `/tmp/`:

1. **MARIADB_AIX_BUGS_REPORT.md** - Complete bug analysis
2. **mariadb-aix-perfschema.patch** - Unified diff patch
3. **CMakeLists.txt** - Modified file with fixes applied

Also downloaded to:
- `~/.config/librepower/aix-ports/MARIADB_AIX_BUGS_REPORT.md`
- `~/.config/librepower/aix-ports/mariadb-aix-perfschema.patch`

---

## Contact Information

**Submitter**: LibrePower Project  
**Email**: hello@librepower.org  
**Website**: https://librepower.org  
**GitLab**: https://gitlab.com/librepower  

---

## Notes

- Keep commit message professional and technical
- Reference JIRA ticket number in all communications
- Be patient - MariaDB reviews thoroughly
- Offer to maintain AIX support going forward if needed

---

**Last Updated**: January 13, 2026  
**Status**: Ready for submission - waiting for JIRA ticket creation
