Summary: MariaDB 11.8.0 Server - Community developed fork of MySQL
Name: mariadb11
Version: 11.8.0
Release: 4.librepower
License: GPLv2
Group: Applications/Databases
URL: https://mariadb.org
Packager: LibrePower <hello@librepower.org>

%description
MariaDB is a community developed fork of MySQL server. Started by core
members of the original MySQL team, MariaDB actively works with outside
developers to deliver the most featureful, stable, and sanely licensed
open SQL server in the industry.

This build for AIX includes critical patches for Performance Schema
compatibility on AIX/POWER platforms and uses pthread-enabled libstdc++
for proper C++11 threading support.

Binaries installed in /opt/freeware/mariadb, data directory in /var/mariadb/data.

%prep
# No prep needed - building from existing compiled binaries

%build
# Already built in /tmp/mariadb-build

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/mariadb/bin
mkdir -p %{buildroot}/opt/freeware/mariadb/lib
mkdir -p %{buildroot}/opt/freeware/mariadb/share
mkdir -p %{buildroot}/var/mariadb/data

# Install binaries
cp /tmp/mariadb-build/sql/mariadbd %{buildroot}/opt/freeware/mariadb/bin/mariadbd.bin
cp /tmp/mariadb-build/sql/libserver.so %{buildroot}/opt/freeware/mariadb/lib/
cp /tmp/mariadb-build/scripts/mysql_install_db %{buildroot}/opt/freeware/mariadb/bin/
cp /tmp/mariadb-build/scripts/mysqld_safe %{buildroot}/opt/freeware/mariadb/bin/
cp /tmp/mariadb-build/extra/my_print_defaults %{buildroot}/opt/freeware/mariadb/bin/
cp /tmp/mariadb-build/extra/resolveip %{buildroot}/opt/freeware/mariadb/bin/
cp /tmp/mariadb-build/extra/perror %{buildroot}/opt/freeware/mariadb/bin/
cp /tmp/mariadb-build/extra/replace %{buildroot}/opt/freeware/mariadb/bin/
cp /tmp/mariadb-build/extra/mariadbd-safe-helper %{buildroot}/opt/freeware/mariadb/bin/
cp /tmp/mariadb-build/extra/mariadb-waitpid %{buildroot}/opt/freeware/mariadb/bin/

# Create wrapper script for mariadbd
cat > %{buildroot}/opt/freeware/mariadb/bin/mariadbd << 'WRAPPER_EOF'
#!/usr/bin/ksh
export LIBPATH=/opt/freeware/mariadb/lib:/opt/freeware/lib/pthread:/opt/freeware/lib/gcc/powerpc-ibm-aix7.3.0.0/13/pthread:/opt/freeware/lib:/usr/lib
exec /opt/freeware/mariadb/bin/mariadbd.bin "$@"
WRAPPER_EOF
chmod +x %{buildroot}/opt/freeware/mariadb/bin/mariadbd

# Create SRC control script
cat > %{buildroot}/opt/freeware/mariadb/bin/mariadb-src << 'SRC_EOF'
#!/usr/bin/ksh
# MariaDB SRC control script for AIX
# Managed by startsrc/stopsrc

BASEDIR=/opt/freeware/mariadb
DATADIR=/var/mariadb/data
USER=mysql

case "$1" in
  start)
    cd $BASEDIR
    exec $BASEDIR/bin/mariadbd --basedir=$BASEDIR --datadir=$DATADIR --user=$USER
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
chmod +x %{buildroot}/opt/freeware/mariadb/bin/mariadb-src

# Create empty test_db file (workaround for mysql_install_db)
touch %{buildroot}/opt/freeware/mariadb/share/mariadb_test_db.sql

# Install share files
cp -r /tmp/mariadb-build/sql/share/* %{buildroot}/opt/freeware/mariadb/share/
cp /tmp/mariadb-build/scripts/*.sql %{buildroot}/opt/freeware/mariadb/share/ 2>/dev/null || true

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,system)
/opt/freeware/mariadb/bin/*
/opt/freeware/mariadb/lib/*
/opt/freeware/mariadb/share/*
%dir /var/mariadb/data

%pre
# Create mysql user if it doesn't exist (AIX)
if ! lsuser mysql >/dev/null 2>&1; then
    mkuser pgrp=staff home=/var/mariadb/data shell=/usr/bin/ksh mysql 2>/dev/null || true
fi

# Create data directory structure
mkdir -p /var/mariadb/data 2>/dev/null || true

%post
# Set permissions on data directory
if [ -d /var/mariadb/data ]; then
    chown mysql:staff /var/mariadb/data 2>/dev/null || true
    chmod 750 /var/mariadb/data 2>/dev/null || true
fi

# Register SRC subsystem
rmssys -s mariadb11 2>/dev/null || true
mkssys -s mariadb11 -p /opt/freeware/mariadb/bin/mariadb-src -a start -u mysql -S -n 15 -f 9 -R 2>/dev/null || true

echo "====================================================================="
echo " MariaDB 11.8.0 for AIX - Successfully Installed!"
echo "====================================================================="
echo ""
echo "Binaries: /opt/freeware/mariadb"
echo "Data directory: /var/mariadb/data"
echo ""
echo "This build includes critical patches for AIX compatibility:"
echo "  - Fixed Performance Schema pthread_threadid_np detection"
echo "  - Fixed Performance Schema getthrid detection"
echo "  - Uses pthread-enabled libstdc++ for C++11 threading support"
echo ""
echo "To initialize the database (first time only):"
echo "  cd /opt/freeware/mariadb"
echo "  ./bin/mysql_install_db --basedir=/opt/freeware/mariadb --datadir=/var/mariadb/data --user=mysql"
echo ""
echo "To start/stop the server (using AIX SRC):"
echo "  startsrc -s mariadb11"
echo "  stopsrc -s mariadb11"
echo "  lssrc -s mariadb11"
echo ""
echo "Or start manually:"
echo "  cd /opt/freeware/mariadb"
echo "  ./bin/mariadbd --basedir=/opt/freeware/mariadb --datadir=/var/mariadb/data --user=mysql &"
echo ""
echo "Repository: https://gitlab.com/librepower/mariadb"
echo "Patches: Pending submission to upstream MariaDB project"
echo "Support: https://librepower.org"
echo "Subscribe: https://librepower.substack.com"
echo "====================================================================="

%preun
# Stop mariadb if running
stopsrc -s mariadb11 2>/dev/null || true
sleep 2
# Remove SRC subsystem
rmssys -s mariadb11 2>/dev/null || true

%postun
echo "MariaDB 11.8.0 uninstalled."

%changelog
* Mon Jan 14 2025 LibrePower <hello@librepower.org> - 11.8.0-4
- Corrected installation paths: binaries in /opt/freeware/mariadb, data in /var/mariadb/data
- Standard AIX directory structure for database servers
- Added AIX SRC (System Resource Controller) support
- Can now be managed with startsrc/stopsrc commands
- Added mariadb-src wrapper script for SRC integration
- Auto-registers as mariadb11 subsystem on installation
- Added workaround for mariadb_test_db.sql requirement
- Fixed user/group dependencies for dnf compatibility
- Improved %pre script to use lsuser and mkuser (AIX-native commands)

* Mon Jan 13 2025 LibrePower <hello@librepower.org> - 11.8.0-1
- Initial AIX port of MariaDB 11.8.0
- Applied critical patches for Performance Schema on AIX
- Fixed pthread_threadid_np incorrect detection (macOS-specific)
- Fixed getthrid incorrect detection (OpenBSD-specific)
- Uses pthread-enabled libstdc++ for proper C++11 threading
- Successfully compiled on AIX 7.3 POWER with GCC 13.3.0
- Patches pending submission to upstream MariaDB project
