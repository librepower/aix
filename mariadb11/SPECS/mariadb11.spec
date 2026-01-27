Summary: MariaDB 11.8.5 LTS Server with Thread Pool for AIX
Name: mariadb11
Version: 11.8.5
Release: 1.librepower
License: GPLv2
Group: Applications/Databases
URL: https://mariadb.org
Packager: LibrePower <hello@librepower.org>

%description
MariaDB 11.8.5 LTS for IBM AIX with native Thread Pool support.

Optimized with -O3 -mcpu=power9. Native AIX pollset thread pool (v11).
83 percent performance improvement for mixed workloads at 100 clients.
QA validated: 1000 clients, 30 min sustained load, 0 errors.

Binaries in /opt/freeware/mariadb, data in /var/mariadb/data.

%prep

%build

%install
rm -rf %{buildroot}
BUILD=/tmp/mariadb-11.8.5-dev-build

mkdir -p %{buildroot}/opt/freeware/mariadb/bin
mkdir -p %{buildroot}/opt/freeware/mariadb/lib
mkdir -p %{buildroot}/opt/freeware/mariadb/share
mkdir -p %{buildroot}/var/mariadb/data

cp $BUILD/sql/mariadbd %{buildroot}/opt/freeware/mariadb/bin/mariadbd.bin
cp $BUILD/sql/libserver.so %{buildroot}/opt/freeware/mariadb/lib/
cp $BUILD/client/mariadb %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/client/mariadb-admin %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/client/mariadb-binlog %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/client/mariadb-check %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/client/mariadb-conv %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/client/mariadb-dump %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/client/mariadb-import %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/client/mariadb-show %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/client/mariadb-slap %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/client/mariadb-upgrade %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/extra/my_print_defaults %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/extra/resolveip %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/extra/perror %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/extra/replace %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/extra/mariadbd-safe-helper %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/extra/mariadb-waitpid %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/extra/innochecksum %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/scripts/mysqld_safe %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/scripts/mariadb-install-db %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/scripts/mariadb-secure-installation %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/scripts/mysql_install_db %{buildroot}/opt/freeware/mariadb/bin/
cp $BUILD/scripts/fill_help_tables.sql %{buildroot}/opt/freeware/mariadb/share/
cp $BUILD/scripts/mariadb_system_tables.sql %{buildroot}/opt/freeware/mariadb/share/
cp $BUILD/scripts/mariadb_system_tables_data.sql %{buildroot}/opt/freeware/mariadb/share/
cp $BUILD/scripts/mariadb_fix_privilege_tables.sql %{buildroot}/opt/freeware/mariadb/share/
cp $BUILD/scripts/mariadb_performance_tables.sql %{buildroot}/opt/freeware/mariadb/share/
cp $BUILD/scripts/mariadb_sys_schema.sql %{buildroot}/opt/freeware/mariadb/share/
cp $BUILD/scripts/maria_add_gis_sp.sql %{buildroot}/opt/freeware/mariadb/share/
cp $BUILD/scripts/maria_add_gis_sp_bootstrap.sql %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/english %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/spanish %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/french %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/german %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/japanese %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/chinese %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/portuguese %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/italian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/korean %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/russian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/dutch %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/polish %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/czech %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/danish %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/hungarian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/swedish %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/norwegian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/romanian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/estonian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/greek %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/serbian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/slovak %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/ukrainian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/bulgarian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/georgian %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/hindi %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/norwegian-ny %{buildroot}/opt/freeware/mariadb/share/
cp -r $BUILD/sql/share/swahili %{buildroot}/opt/freeware/mariadb/share/

cat > %{buildroot}/opt/freeware/mariadb/bin/mariadbd << 'WRAPPER_EOF'
#!/usr/bin/ksh
export LIBPATH=/opt/freeware/mariadb/lib:/opt/freeware/lib/gcc/powerpc-ibm-aix7.3.0.0/13/pthread:/opt/freeware/lib/gcc/powerpc-ibm-aix7.3.0.0/13:/opt/freeware/lib64:/opt/freeware/lib:/usr/lib
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

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,system)
/opt/freeware/mariadb/bin/*
/opt/freeware/mariadb/lib/*
/opt/freeware/mariadb/share/*
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
echo "MariaDB 11.8.5 LTS for AIX - LibrePower (pool-of-threads, -O3)"
echo "Start: startsrc -s mariadb11 | Stop: stopsrc -s mariadb11"

%preun
stopsrc -s mariadb11 2>/dev/null || true
sleep 2
rmssys -s mariadb11 2>/dev/null || true

%changelog
* Tue Jan 27 2026 LibrePower <hello@librepower.org> - 11.8.5-1
- Upgrade to MariaDB 11.8.5 LTS
- Optimized: -O3 -mcpu=power9 -mtune=power9 (Release)
- Native AIX thread pool: pollset ONESHOT v11 with per-pollset mutex
- QA: MTR 709 pass, 1000 clients, 30 min sustained, 0 errors
- Performance: 83 percent faster mixed workloads at 100 clients
