# AIX RPM Spec Best Practices - LibrePower

**Author**: LibrePower <hello@librepower.org>
**Date**: January 14, 2026
**Purpose**: Guidelines for creating portable, dnf-compatible RPM specs for AIX

---

## Table of Contents

1. [User and Group Management](#user-and-group-management)
2. [File Ownership and Permissions](#file-ownership-and-permissions)
3. [Directory Structure](#directory-structure)
4. [Dependencies](#dependencies)
5. [SRC Integration](#src-integration)
6. [Testing Checklist](#testing-checklist)

---

## User and Group Management

### ❌ NEVER Do This

```spec
%files
%attr(750,mysql,staff) %dir /var/app/data
```

**Problem**: Creates hard dependency on `user(mysql)` and `group(staff)` that dnf cannot validate, causing installation failures.

### ✅ DO This Instead

```spec
%files
%dir /var/app/data

%pre
# Create user if needed (AIX-native commands)
if ! lsuser myuser >/dev/null 2>&1; then
    mkuser pgrp=staff home=/var/app/data shell=/usr/bin/ksh myuser 2>/dev/null || true
fi

%post
# Set permissions after files are installed
if [ -d /var/app/data ]; then
    chown myuser:staff /var/app/data 2>/dev/null || true
    chmod 750 /var/app/data 2>/dev/null || true
fi
```

**Why**:
- No RPM dependency on user/group
- Uses AIX-native `lsuser`/`mkuser` commands
- Permissions set during installation, not as RPM requirement
- `|| true` prevents installation failure if commands fail

---

## File Ownership and Permissions

### ❌ Avoid Hard-Coded Ownership in %files

```spec
%files
%defattr(0755,mysql,mysql)  # BAD: requires mysql user to exist before RPM install
/opt/app/bin/*
```

### ✅ Use Generic Ownership, Set Specific in Scripts

```spec
%files
%defattr(-,root,system)  # Good: uses existing system users
/opt/app/bin/*
/opt/app/lib/*
%dir /var/app/data       # No ownership specified here

%post
# Set specific ownership after installation
chown -R appuser:staff /var/app/data 2>/dev/null || true
chmod 750 /var/app/data 2>/dev/null || true
```

**Benefits**:
- RPM installs without user dependencies
- Flexible ownership configuration
- Works with dnf/yum validation

---

## Directory Structure

### Standard AIX Paths

```spec
# Binaries and libraries - use /opt/freeware
/opt/freeware/app/bin/*      # Application binaries
/opt/freeware/app/lib/*      # Libraries and shared objects
/opt/freeware/app/share/*    # Static data, docs

# Variable/runtime data - use /var
/var/app/data                # Database files, logs
/var/app/run                 # PID files, sockets
/var/app/tmp                 # Temporary files
```

### Disk Space Considerations

Check available space on AIX:
```bash
df -g /opt /var
```

**Guidelines**:
- Large binaries (>500MB): Check `/opt` has space, consider `/usr/local` if needed
- Data directories: Always use `/var` (typically has most space)
- Logs: `/var/app/log` or `/var/log/app`

### Directory Creation

```spec
%install
# Create full directory structure in buildroot
mkdir -p %{buildroot}/opt/freeware/app/{bin,lib,share}
mkdir -p %{buildroot}/var/app/{data,log,run}

%files
# List directories that should be owned by package
%dir /opt/freeware/app
%dir /opt/freeware/app/bin
%dir /var/app
%dir /var/app/data

%pre
# Ensure directories exist before installation
mkdir -p /var/app/data 2>/dev/null || true
mkdir -p /var/app/log 2>/dev/null || true
```

---

## Dependencies

### System Dependencies

```spec
# ✅ Good: Standard AIX/Linux libraries
Requires: /usr/bin/ksh
Requires: libc.a(shr_64.o)
Requires: libpthread.a(shr_xpg5_64.o)

# ❌ Avoid: User/group requirements
# Requires: user(mysql)     # Will break dnf install
# Requires: group(staff)    # Will break dnf install

# ✅ Good: Other packages
Requires: openssl >= 1.1.1
Requires: pcre2-devel
```

### Auto-Generated Dependencies

RPM automatically detects:
- Shared library dependencies (`.so` files)
- Shell interpreters (`#!/bin/ksh`, `#!/usr/bin/perl`)
- Binary dependencies via `ldd`

**Verify auto-dependencies**:
```bash
rpm -qp --requires mypackage.rpm
```

### Preventing False Dependencies

```spec
# Disable automatic dependency generation if needed
%global __requires_exclude ^perl\\(
%global __provides_exclude_from ^/opt/app/internal/

# Or filter specific items
%filter_from_requires /^user(/d
%filter_from_requires /^group(/d
%filter_setup
```

---

## SRC Integration

### Complete SRC Implementation

```spec
%install
# ... install binaries ...

# Create SRC control script
cat > %{buildroot}/opt/freeware/app/bin/app-src << 'SRC_EOF'
#!/usr/bin/ksh
# AIX SRC control script

BASEDIR=/opt/freeware/app
USER=appuser

case "$1" in
  start)
    cd $BASEDIR
    exec $BASEDIR/bin/appd --config=/etc/app.conf
    ;;
  stop)
    # SRC will send SIGTERM to the process
    exit 0
    ;;
  *)
    echo "Usage: $0 {start|stop}"
    exit 1
    ;;
esac
SRC_EOF
chmod +x %{buildroot}/opt/freeware/app/bin/app-src

%post
# Register SRC subsystem
rmssys -s myapp 2>/dev/null || true
mkssys -s myapp \
    -p /opt/freeware/app/bin/app-src \
    -a start \
    -u appuser \
    -S \
    -n 15 \
    -f 9 \
    -R \
    2>/dev/null || true

echo "Start with: startsrc -s myapp"
echo "Stop with:  stopsrc -s myapp"
echo "Status:     lssrc -s myapp"

%preun
# Stop and unregister before uninstall
stopsrc -s myapp 2>/dev/null || true
sleep 2
rmssys -s myapp 2>/dev/null || true
```

**SRC Parameters Explained**:
- `-s myapp`: Subsystem name
- `-p /path/script`: Path to control script
- `-a start`: Argument to pass (tells script to start)
- `-u appuser`: Run as this user
- `-S`: Subsystem (not a group)
- `-n 15`: Wait 15 seconds for SIGTERM
- `-f 9`: Force with SIGKILL if SIGTERM fails
- `-R`: Restart on failure

---

## Testing Checklist

### Before Building RPM

- [ ] All source files exist in expected locations
- [ ] Build directory has sufficient space
- [ ] All required build dependencies installed

### After Building RPM

```bash
# 1. Verify RPM structure
rpm -qilp mypackage.rpm

# 2. Check dependencies
rpm -qp --requires mypackage.rpm
rpm -qp --provides mypackage.rpm

# 3. Verify no user/group requirements
rpm -qp --requires mypackage.rpm | grep -E 'user\(|group\('
# Should return nothing

# 4. Check file permissions
rpm -qp --dump mypackage.rpm | grep -E '^/'

# 5. Validate RPM integrity
rpm -K --nosignature mypackage.rpm
```

### Installation Testing

```bash
# 1. Clean test (without dnf)
rpm -ivh --nosignature --nodigest mypackage.rpm

# 2. Verify installation
rpm -q mypackage
rpm -ql mypackage

# 3. Test functionality
/opt/freeware/app/bin/app --version
lssrc -s myapp

# 4. Test SRC integration
startsrc -s myapp
lssrc -s myapp
stopsrc -s myapp

# 5. Clean uninstall
rpm -e mypackage

# 6. Verify cleanup
ls /opt/freeware/app  # Should not exist
lssrc -s myapp        # Should error (not registered)
```

### DNF/YUM Testing

```bash
# 1. Clean metadata
dnf clean all

# 2. Check package availability
dnf list available | grep mypackage

# 3. Install via dnf
dnf install -y mypackage

# 4. Verify no dependency errors
# Should install cleanly without user/group errors

# 5. Remove via dnf
dnf remove -y mypackage
```

---

## Common Pitfalls

### 1. Large Files in RPM

**Problem**: Files >700MB may cause cpio errors during extraction

**Solution**:
- Keep individual files <500MB if possible
- Split large datasets into multiple sub-packages
- Document manual extraction if needed

### 2. Hard-Coded Paths

**Problem**:
```spec
export LIBPATH=/opt/freeware/lib/gcc/powerpc-ibm-aix7.3.0.0/13/pthread
```

**Solution**:
```spec
export LIBPATH=/opt/freeware/lib/pthread:\
/opt/freeware/lib/gcc/powerpc-ibm-aix*/*/pthread:\
/opt/freeware/lib:\
/usr/lib
```

### 3. Missing Error Handling

**Problem**:
```spec
%pre
mkuser mysql  # Fails if user exists
```

**Solution**:
```spec
%pre
if ! lsuser mysql >/dev/null 2>&1; then
    mkuser pgrp=staff home=/var/app shell=/usr/bin/ksh mysql 2>/dev/null || true
fi
```

### 4. Assuming GNU Commands

**Problem**:
```bash
pkill -9 myapp  # Not available on AIX
```

**Solution**:
```bash
ps -ef | grep myapp | grep -v grep | awk '{print $2}' | xargs kill -9 2>/dev/null || true
```

---

## Example: Complete Minimal Spec

```spec
Summary: My Application - Description
Name: myapp
Version: 1.0.0
Release: 1.librepower
License: MIT
Group: Applications/System
URL: https://example.com
Packager: LibrePower <hello@librepower.org>

%description
My application description.
Works on AIX 7.2+ on POWER architecture.

%prep
# Build already completed

%build
# Build already completed

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/myapp/bin
mkdir -p %{buildroot}/var/myapp/data

# Install files
cp /tmp/build/myapp %{buildroot}/opt/freeware/myapp/bin/

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,system)
/opt/freeware/myapp/bin/*
%dir /var/myapp/data

%pre
if ! lsuser myappuser >/dev/null 2>&1; then
    mkuser pgrp=staff home=/var/myapp shell=/usr/bin/ksh myappuser 2>/dev/null || true
fi
mkdir -p /var/myapp/data 2>/dev/null || true

%post
if [ -d /var/myapp/data ]; then
    chown myappuser:staff /var/myapp/data 2>/dev/null || true
    chmod 750 /var/myapp/data 2>/dev/null || true
fi

echo "MyApp installed successfully"
echo "Run: /opt/freeware/myapp/bin/myapp"

%preun
# Cleanup before uninstall
ps -ef | grep myapp | grep -v grep | awk '{print $2}' | xargs kill -9 2>/dev/null || true

%postun
echo "MyApp uninstalled"

%changelog
* Mon Jan 14 2026 LibrePower <hello@librepower.org> - 1.0.0-1
- Initial release for AIX
```

---

## Resources

- AIX Documentation: https://www.ibm.com/docs/en/aix/
- RPM Packaging Guide: https://rpm-packaging-guide.github.io/
- LibrePower Repository: https://gitlab.com/librepower/aix

---

**Questions or Issues?**

Contact: hello@librepower.org
Repository: https://gitlab.com/librepower/aix/-/issues
