Summary: MariaDB 11.8.5 LTS Server with Thread Pool for AIX (Open XL Build)
Name: mariadb11-openxl
Version: 11.8.5
Release: 3.librepower
Conflicts: mariadb11
License: GPLv3
Group: Applications/Databases
URL: https://mariadb.org
Packager: LibrePower <hello@librepower.org>

%description
MariaDB 11.8.5 LTS for IBM AIX with native Thread Pool support.
Open XL build -- compiled with IBM Open XL C/C++ 17.1.3 (Clang/LLVM-based)
for optimal POWER9/10/11 performance.

Native AIX pollset thread pool. 64K large pages via LDR_CNTRL.
QA validated: 1000 clients, 30 min sustained load, 0 errors.

Requires IBM Open XL C/C++ 17.1.3 runtime. Please consult IBM for licensing.
For a build with no external dependencies, install mariadb11 (GCC build).

Binaries in /opt/freeware/mariadb, data in /var/mariadb/data.

%prep

%build

%install
rm -rf %{buildroot}
# Server binary from Open XL build, client tools from GCC build
# (client tools don't benefit from Open XL; server is where it matters)
BUILD_XL=/tmp/mariadb-11.8.5-openxl-build
BUILD_GCC=/tmp/mariadb-11.8.5-dev-build

mkdir -p %{buildroot}/opt/freeware/mariadb/bin
mkdir -p %{buildroot}/opt/freeware/mariadb/lib
mkdir -p %{buildroot}/opt/freeware/mariadb/share
mkdir -p %{buildroot}/opt/freeware/mariadb/etc
mkdir -p %{buildroot}/var/mariadb/data

# Server (Open XL compiled)
cp $BUILD_XL/sql/mariadbd %{buildroot}/opt/freeware/mariadb/bin/mariadbd.bin
cp $BUILD_XL/sql/libserver.so %{buildroot}/opt/freeware/mariadb/lib/

# Client tools (GCC compiled)
cp $BUILD_GCC/client/mariadb %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/client/mariadb-admin %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/client/mariadb-binlog %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/client/mariadb-check %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/client/mariadb-conv %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/client/mariadb-dump %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/client/mariadb-import %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/client/mariadb-show %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/client/mariadb-slap %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/client/mariadb-upgrade %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/extra/my_print_defaults %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/extra/resolveip %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/extra/perror %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/extra/replace %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/extra/mariadbd-safe-helper %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/extra/mariadb-waitpid %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/extra/innochecksum %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/scripts/mysqld_safe %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/scripts/mariadb-install-db %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/scripts/mariadb-secure-installation %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/scripts/mysql_install_db %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD_GCC/scripts/fill_help_tables.sql %{buildroot}/opt/freeware/mariadb/share/
cp $BUILD_GCC/scripts/mariadb_system_tables.sql %{buildroot}/opt/freeware/mariadb/share/
cp $BUILD_GCC/scripts/mariadb_system_tables_data.sql %{buildroot}/opt/freeware/mariadb/share/
cp $BUILD_GCC/scripts/mariadb_fix_privilege_tables.sql %{buildroot}/opt/freeware/mariadb/share/
cp $BUILD_GCC/scripts/mariadb_performance_tables.sql %{buildroot}/opt/freeware/mariadb/share/
cp $BUILD_GCC/scripts/mariadb_sys_schema.sql %{buildroot}/opt/freeware/mariadb/share/
cp $BUILD_GCC/scripts/maria_add_gis_sp.sql %{buildroot}/opt/freeware/mariadb/share/
cp $BUILD_GCC/scripts/maria_add_gis_sp_bootstrap.sql %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/english %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/spanish %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/french %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/german %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/japanese %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/chinese %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/portuguese %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/italian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/korean %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/russian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/dutch %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/polish %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/czech %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/danish %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/hungarian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/swedish %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/norwegian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/romanian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/estonian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/greek %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/serbian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/slovak %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/ukrainian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/bulgarian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/georgian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/hindi %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/norwegian-ny %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD_GCC/sql/share/swahili %{buildroot}/opt/freeware/mariadb/share/

# Wrapper with Open XL runtime and 64K pages
cat > %{buildroot}/opt/freeware/mariadb/bin/mariadbd << 'WRAPPER_EOF'
#!/usr/bin/ksh
# Open XL build requires xlclang runtime
export LIBPATH=/opt/freeware/mariadb/lib:/opt/IBM/openxlC/17.1.3/lib:/opt/freeware/lib64:/opt/freeware/lib:/usr/lib
# Use 64K pages for all segments to reduce TLB misses on POWER
# Critical for MHNSW vector index graph traversal (pointer-chasing workload)
export LDR_CNTRL=DATAPSIZE=64K@TEXTPSIZE=64K@STACKPSIZE=64K@SHMPSIZE=64K
exec /opt/freeware/mariadb/bin/mariadbd.bin "$@"
WRAPPER_EOF
chmod +x %{buildroot}/opt/freeware/mariadb/bin/mariadbd

cat > %{buildroot}/opt/freeware/mariadb/bin/mariadb-src << 'SRC_EOF'
#!/usr/bin/ksh
BASEDIR=/opt/freeware/mariadb
DATADIR=/var/mariadb/data
USER=mysql
case "$1" in
  start)
    cd $BASEDIR
    exec $BASEDIR/bin/mariadbd \
      --basedir=$BASEDIR \
      --datadir=$DATADIR \
      --user=$USER \
      --thread-handling=pool-of-threads \
      --thread-pool-size=12 \
      --thread-stack=512K \
      --log-warnings=9
    ;;
  stop)
    exit 0
    ;;
  *)
    echo "Usage: $0 {start|stop}"
    exit 1
    ;;
esac
SRC_EOF
chmod +x %{buildroot}/opt/freeware/mariadb/bin/mariadb-src

cat > %{buildroot}/opt/freeware/mariadb/etc/mariadb11.cnf << 'CNF_EOF'
# MariaDB 11.8 for AIX - LibrePower Open XL optimized defaults
# /opt/freeware/mariadb/etc/mariadb11.cnf
#
# Load via: mariadbd --defaults-file=/opt/freeware/mariadb/etc/mariadb11.cnf
# Or copy to /etc/my.cnf.d/ if using AIX Toolbox config layout.

[mariadbd]
# --- Storage ---
basedir  = /opt/freeware/mariadb
datadir  = /var/mariadb/data
socket   = /tmp/mysql.sock
port     = 3306

# --- Thread Pool (AIX pollset) ---
thread_handling    = pool-of-threads
thread_pool_size   = 12
thread_stack       = 512K

# --- InnoDB ---
innodb_buffer_pool_size    = 1G
innodb_adaptive_hash_index = ON
innodb_read_io_threads     = 12
innodb_write_io_threads    = 12

# --- Connections ---
max_connections = 2000

# --- Vector Index (MHNSW) ---
# Default 16MB is too small for real workloads. The MHNSW graph cache
# has no LRU -- it evicts entirely when exceeded. For 100K vectors at
# 768 dimensions (M=16), the graph needs ~300MB.
# Set generously; memory is only used when vector indexes are accessed.
mhnsw_max_cache_size = 4294967296

# --- Logging ---
log_error    = /var/mariadb/mariadbd.err
log_warnings = 2
CNF_EOF

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,system)
/opt/freeware/mariadb/bin/*
/opt/freeware/mariadb/lib/*
/opt/freeware/mariadb/share/*
%config(noreplace) /opt/freeware/mariadb/etc/mariadb11.cnf
%dir /var/mariadb/data

%pre
if ! lsuser mysql >/dev/null 2>&1; then
    mkuser pgrp=staff home=/var/mariadb/data shell=/usr/bin/ksh mysql 2>/dev/null || true
fi
mkdir -p /var/mariadb/data 2>/dev/null || true

%post
chown mysql:staff /var/mariadb/data 2>/dev/null || true
chmod 750 /var/mariadb/data 2>/dev/null || true
rmssys -s mariadb11 2>/dev/null || true
mkssys -s mariadb11 -p /opt/freeware/mariadb/bin/mariadb-src -a start -u mysql -S -n 15 -f 9 -R 2>/dev/null || true
echo "MariaDB 11.8.5 LTS for AIX - LibrePower (Open XL, pool-of-threads)"
echo "Start: startsrc -s mariadb11 | Stop: stopsrc -s mariadb11"
echo ""
echo "NOTE: Requires IBM Open XL C/C++ 17.1.3 runtime in /opt/IBM/openxlC/17.1.3/lib"

%preun
stopsrc -s mariadb11 2>/dev/null || true
sleep 2
rmssys -s mariadb11 2>/dev/null || true

%changelog
* Tue Jan 28 2026 LibrePower <hello@librepower.org> - 11.8.5-3.openxl
- Compiler: IBM Open XL C/C++ 17.1.3 (Clang/LLVM-based)
- Optimized for POWER9/10/11
- Stability validated under sustained load
- Same features as GCC build: pollset thread pool, 64K pages, 4GB MHNSW cache
- Separate package name (mariadb11-openxl) to coexist in DNF repo with mariadb11

* Mon Jan 27 2026 LibrePower <hello@librepower.org> - 11.8.5-2
- Wrapper: enable 64K pages via LDR_CNTRL (DATAPSIZE, TEXTPSIZE, STACKPSIZE, SHMPSIZE)
  Reduces TLB misses for MHNSW vector graph traversal on POWER
- Add default config /opt/freeware/mariadb/etc/mariadb11.cnf with:
  mhnsw_max_cache_size=4GB (upstream default 16MB causes full cache eviction),
  thread pool, InnoDB, and connection tuning
- MHNSW vector search: 42 QPS -> 1200+ QPS (12 workers, 100K vectors, warm cache)

* Tue Jan 27 2026 LibrePower <hello@librepower.org> - 11.8.5-1
- Upgrade to MariaDB 11.8.5 LTS
- Optimized: -O3 -mcpu=power9 -mtune=power9 (Release)
- Native AIX thread pool: pollset ONESHOT v11 with per-pollset mutex
- QA: MTR 709 pass, 1000 clients, 30 min sustained, 0 errors
- Performance: 83 percent faster mixed workloads at 100 clients
