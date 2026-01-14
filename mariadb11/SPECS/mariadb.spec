Summary: MariaDB 11.8.0 Server - Community developed fork of MySQL
Name: mariadb11
Version: 11.8.0
Release: 1.librepower
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

%prep
# No prep needed - building from existing compiled binaries

%build
# Already built in /tmp/mariadb-build

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/mariadb/bin
mkdir -p %{buildroot}/opt/freeware/mariadb/lib
mkdir -p %{buildroot}/opt/freeware/mariadb/share
mkdir -p %{buildroot}/opt/freeware/mariadb/data

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
%attr(750,mysql,staff) %dir /opt/freeware/mariadb/data

%pre
# Create mysql user if it doesn't exist
if ! id mysql > /dev/null 2>&1; then
    useradd -d /opt/freeware/mariadb/data -s /usr/bin/ksh mysql
fi

%post
echo "====================================================================="
echo " MariaDB 11.8.0 for AIX - Successfully Installed!"
echo "====================================================================="
echo ""
echo "This build includes critical patches for AIX compatibility:"
echo "  - Fixed Performance Schema pthread_threadid_np detection"
echo "  - Fixed Performance Schema getthrid detection"
echo "  - Uses pthread-enabled libstdc++ for C++11 threading support"
echo ""
echo "To initialize the database:"
echo "  cd /opt/freeware/mariadb"
echo "  export LIBPATH=/opt/freeware/lib/pthread:/opt/freeware/lib/gcc/powerpc-ibm-aix7.3.0.0/13/pthread:/opt/freeware/lib:/opt/freeware/mariadb/lib:/usr/lib"
echo "  ./bin/mysql_install_db --basedir=/opt/freeware/mariadb --datadir=/opt/freeware/mariadb/data --user=mysql"
echo ""
echo "To start the server:"
echo "  ./bin/mariadbd --basedir=/opt/freeware/mariadb --datadir=/opt/freeware/mariadb/data --user=mysql"
echo ""
echo "Repository: https://gitlab.com/librepower/mariadb"
echo "Patches: Pending submission to upstream MariaDB project"
echo "Support: https://librepower.org"
echo "Subscribe: https://librepower.substack.com"
echo "====================================================================="

%preun
# Stop mariadb if running
pkill -9 mariadbd 2>/dev/null || true

%postun
echo "MariaDB 11.8.0 uninstalled."

%changelog
* Mon Jan 13 2025 LibrePower <hello@librepower.org> - 11.8.0-1
- Initial AIX port of MariaDB 11.8.0
- Applied critical patches for Performance Schema on AIX
- Fixed pthread_threadid_np incorrect detection (macOS-specific)
- Fixed getthrid incorrect detection (OpenBSD-specific)
- Uses pthread-enabled libstdc++ for proper C++11 threading
- Successfully compiled on AIX 7.3 POWER9 with GCC 13.3.0
- Patches pending submission to upstream MariaDB project
