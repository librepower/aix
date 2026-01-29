# Building MariaDB 11.8.5 for AIX from Source

This guide explains how to build MariaDB 11.8.5 for AIX from source and generate the RPM packages. Anyone with access to an AIX 7.3+ system can reproduce these builds.

---

## Prerequisites

### System Requirements

- AIX 7.3 or later
- POWER9, POWER10, or POWER11 processor
- At least 8GB RAM (16GB+ recommended for parallel builds)
- At least 10GB free disk space

### Required Packages

```bash
# Install from AIX Toolbox (dnf)
dnf install gcc gcc-c++ cmake make git patch rpm-build

# Or via yum
yum install gcc gcc-c++ cmake make git patch rpm-build
```

### For Open XL Build (Optional)

```bash
# IBM Open XL C/C++ 17.1.3 compiler
# Install from IBM Passport Advantage or contact IBM for licensing
installp -aXYgd /path/to/openxlC.17.1.3 openxlC.17.1.3
installp -aXYgd /path/to/openxlCrte.17.1.3 openxlCrte.17.1.3
```

---

## Step 1: Get MariaDB Source

```bash
cd /tmp

# Clone MariaDB 11.8.5
git clone --branch mariadb-11.8.5 --depth 1 https://github.com/MariaDB/server.git mariadb-11.8.5
cd mariadb-11.8.5

# Initialize submodules
git submodule update --init --recursive
```

---

## Step 2: Get Patches

```bash
cd /tmp/mariadb-11.8.5

# Option A: Clone LibrePower repo
git clone https://gitlab.com/librepower/aix.git /tmp/librepower-aix
cp /tmp/librepower-aix/mariadb11/SOURCES/*.patch .

# Option B: Download directly
curl -LO https://gitlab.com/librepower/aix/-/raw/main/mariadb11/SOURCES/mariadb-aix-perfschema.patch
curl -LO https://gitlab.com/librepower/aix/-/raw/main/mariadb11/SOURCES/threadpool_aix_pollset.patch
curl -LO https://gitlab.com/librepower/aix/-/raw/main/mariadb11/SOURCES/0004-AIX-large-pages-MAP_ANON_64K.patch
```

---

## Step 3: Apply Patches

```bash
cd /tmp/mariadb-11.8.5

# Apply all three patches
patch -p1 < mariadb-aix-perfschema.patch
patch -p1 < threadpool_aix_pollset.patch
patch -p1 < 0004-AIX-large-pages-MAP_ANON_64K.patch

# Verify patches applied
grep -l "CMAKE_SYSTEM_NAME.*AIX" storage/perfschema/CMakeLists.txt
grep -l "HAVE_POLLSET" sql/CMakeLists.txt
grep -l "MAP_ANON_64K" mysys/my_largepage.c
```

---

## Step 4: Build (GCC)

This builds the GCC version (no external dependencies).

```bash
cd /tmp/mariadb-11.8.5

# Create build directory
mkdir -p /tmp/mariadb-11.8.5-dev-build
cd /tmp/mariadb-11.8.5-dev-build

# Set 64-bit mode (REQUIRED on AIX)
export OBJECT_MODE=64

# Configure
cmake /tmp/mariadb-11.8.5 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS="-O3 -mcpu=power9 -pthread" \
    -DCMAKE_CXX_FLAGS="-O3 -mcpu=power9 -pthread" \
    -DWITH_SSL=bundled \
    -DWITH_ZLIB=bundled \
    -DPLUGIN_TOKUDB=NO \
    -DPLUGIN_MROONGA=NO \
    -DPLUGIN_SPIDER=NO \
    -DPLUGIN_OQGRAPH=NO \
    -DPLUGIN_ROCKSDB=NO \
    -DPLUGIN_CONNECT=NO \
    -DPLUGIN_SPHINX=NO

# Build (adjust -j for your CPU count)
gmake -j$(nproc)

# Verify build
ls -la sql/mariadbd sql/libserver.so
./sql/mariadbd --version
```

---

## Step 5: Build (Open XL) - Optional

This builds the Open XL version (requires IBM Open XL runtime).

```bash
cd /tmp/mariadb-11.8.5

# Create separate build directory
mkdir -p /tmp/mariadb-11.8.5-openxl-build
cd /tmp/mariadb-11.8.5-openxl-build

# Set 64-bit mode and Open XL paths
export OBJECT_MODE=64
export PATH=/opt/IBM/openxlC/17.1.3/bin:$PATH
export CC=ibm-clang
export CXX=ibm-clang++

# Configure
cmake /tmp/mariadb-11.8.5 \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS="-O3 -mcpu=power9" \
    -DCMAKE_CXX_FLAGS="-O3 -mcpu=power9" \
    -DWITH_SSL=bundled \
    -DWITH_ZLIB=bundled \
    -DPLUGIN_TOKUDB=NO \
    -DPLUGIN_MROONGA=NO \
    -DPLUGIN_SPIDER=NO \
    -DPLUGIN_OQGRAPH=NO \
    -DPLUGIN_ROCKSDB=NO \
    -DPLUGIN_CONNECT=NO \
    -DPLUGIN_SPHINX=NO

# Build
gmake -j$(nproc)

# Verify
ls -la sql/mariadbd sql/libserver.so
```

---

## Step 6: Generate RPMs

### Setup RPM Build Environment

```bash
mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

# Copy specs
cp /tmp/librepower-aix/mariadb11/SPECS/mariadb11.spec ~/rpmbuild/SPECS/
cp /tmp/librepower-aix/mariadb11/SPECS/mariadb11-openxl.spec ~/rpmbuild/SPECS/
```

### Build GCC RPM

```bash
# Ensure build exists at expected location
ls /tmp/mariadb-11.8.5-dev-build/sql/mariadbd

# Build RPM
cd ~/rpmbuild
rpmbuild -bb SPECS/mariadb11.spec

# Output will be in RPMS/ppc/
ls -la RPMS/ppc/mariadb11-*.rpm
```

### Build Open XL RPM (if Open XL build was done)

```bash
# Ensure build exists at expected location
ls /tmp/mariadb-11.8.5-openxl-build/sql/mariadbd

# Build RPM
rpmbuild -bb SPECS/mariadb11-openxl.spec

# Output
ls -la RPMS/ppc/mariadb11-openxl-*.rpm
```

---

## Step 7: Install and Test

```bash
# Install (choose one)
rpm -ivh ~/rpmbuild/RPMS/ppc/mariadb11-11.8.5-2.librepower.aix7.3.ppc.rpm
# OR
rpm -ivh ~/rpmbuild/RPMS/ppc/mariadb11-openxl-11.8.5-3.librepower.aix7.3.ppc.rpm

# Initialize database
cd /opt/freeware/mariadb
./bin/mysql_install_db --basedir=/opt/freeware/mariadb --datadir=/var/mariadb/data --user=mysql

# Start service
startsrc -s mariadb11

# Verify
./bin/mariadb -u root -S /tmp/mysql.sock -e "SELECT VERSION();"
# Expected: 11.8.5-MariaDB

# Verify thread pool
./bin/mariadb -u root -S /tmp/mysql.sock -e "SHOW VARIABLES LIKE 'thread_handling';"
# Expected: pool-of-threads
```

---

## Troubleshooting

### "OBJECT_MODE" errors

Always set `export OBJECT_MODE=64` before cmake and gmake.

### Missing pthread symbols

If you get unresolved symbols like `_ZNSt6thread...`, the wrong libstdc++ is being used. The RPM wrapper script handles this, but for manual testing:

```bash
export LIBPATH=/opt/freeware/lib/gcc/powerpc-ibm-aix7.3.0.0/13/pthread:/opt/freeware/lib64:/opt/freeware/lib:/usr/lib
```

### CMake can't find compiler

```bash
# For GCC
export CC=/opt/freeware/bin/gcc
export CXX=/opt/freeware/bin/g++

# For Open XL
export CC=/opt/IBM/openxlC/17.1.3/bin/ibm-clang
export CXX=/opt/IBM/openxlC/17.1.3/bin/ibm-clang++
```

### Build runs out of memory

Reduce parallelism: `gmake -j4` instead of `gmake -j$(nproc)`

---

## Build Matrix

| Build | Compiler | Flags | Output Directory |
|-------|----------|-------|------------------|
| GCC | GCC 13.3.0 | `-O3 -mcpu=power9 -pthread` | `/tmp/mariadb-11.8.5-dev-build` |
| Open XL | IBM Open XL 17.1.3 | `-O3 -mcpu=power9` | `/tmp/mariadb-11.8.5-openxl-build` |

---

## Files Reference

| File | Purpose |
|------|---------|
| `SOURCES/mariadb-aix-perfschema.patch` | Fix Performance Schema CMake detection |
| `SOURCES/threadpool_aix_pollset.patch` | Add AIX thread pool support |
| `SOURCES/0004-AIX-large-pages-MAP_ANON_64K.patch` | Add AIX 64K page support |
| `SPECS/mariadb11.spec` | RPM spec for GCC build |
| `SPECS/mariadb11-openxl.spec` | RPM spec for Open XL build |

---

## Contact

**Maintainer**: LibrePower Project
**Email**: hello@librepower.org
**Repository**: https://gitlab.com/librepower/aix

---

**Last Updated**: 2026-01-29
