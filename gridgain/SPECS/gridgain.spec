Name:           gridgain
Version:        8.9.30
Release:        2.librepower
Summary:        In-memory computing platform (Apache Ignite based)
License:        Apache-2.0
URL:            https://www.gridgain.com
Group:          Applications/Databases
Requires:       semeru-jdk25 >= 25

%description
GridGain Community Edition - high performance in-memory computing platform
based on Apache Ignite. Provides distributed caching, SQL, compute grid.

Includes OpenJ9 compatibility patch for AIX (IgniteMBeanUtils).

Uses IBM Semeru JDK 25 from /opt/freeware/lib/jvm/semeru-25.
Does NOT modify your system Java.

%install
rm -rf %{buildroot}

# GridGain home
mkdir -p %{buildroot}/opt/freeware/lib/gridgain
cp -R /tmp/gridgain-community-8.9.30/bin %{buildroot}/opt/freeware/lib/gridgain/
cp -R /tmp/gridgain-community-8.9.30/libs %{buildroot}/opt/freeware/lib/gridgain/
cp -R /tmp/gridgain-community-8.9.30/config %{buildroot}/opt/freeware/lib/gridgain/
cp /tmp/gridgain-community-8.9.30/LICENSE %{buildroot}/opt/freeware/lib/gridgain/
cp /tmp/gridgain-community-8.9.30/NOTICE %{buildroot}/opt/freeware/lib/gridgain/

# Config (user-editable)
mkdir -p %{buildroot}/opt/freeware/etc/gridgain
cp /tmp/gridgain-community-8.9.30/config/default-config.xml \
   %{buildroot}/opt/freeware/etc/gridgain/gridgain.xml

# Work/data/log
mkdir -p %{buildroot}/opt/freeware/var/lib/gridgain
mkdir -p %{buildroot}/opt/freeware/var/log/gridgain

# Wrapper script - uses shared Semeru JDK
mkdir -p %{buildroot}/opt/freeware/bin
cat > %{buildroot}/opt/freeware/bin/gridgain << 'WRAPEOF'
#!/bin/sh
# GridGain wrapper - uses shared Semeru JDK 25
GRIDGAIN_HOME=/opt/freeware/lib/gridgain
JAVA_HOME=/opt/freeware/lib/jvm/semeru-25
export JAVA_HOME GRIDGAIN_HOME

if [ ! -x "$JAVA_HOME/bin/java" ]; then
    echo "ERROR: Semeru JDK not found at $JAVA_HOME"
    echo "Install it: dnf install semeru-jdk25"
    exit 1
fi

# Default JVM options for AIX (OpenJ9)
if [ -z "$JVM_OPTS" ]; then
    JVM_OPTS="-Xms1g -Xmx4g -XX:MaxDirectMemorySize=2g -Djava.net.preferIPv4Stack=true"
fi
export JVM_OPTS

export IGNITE_HOME="$GRIDGAIN_HOME"
export IGNITE_WORK_DIR=/opt/freeware/var/lib/gridgain

case "$1" in
    start)
        shift
        CONFIG="${1:-/opt/freeware/etc/gridgain/gridgain.xml}"
        echo "Starting GridGain (JAVA_HOME=$JAVA_HOME)..."
        nohup "$GRIDGAIN_HOME/bin/ignite.sh" "$CONFIG" \
            > /opt/freeware/var/log/gridgain/gridgain.log 2>&1 &
        echo "PID: $!"
        ;;
    stop)
        echo "Stopping GridGain..."
        PID=$(ps -ef | grep "ignite" | grep java | grep -v grep | awk '{print $2}')
        if [ -n "$PID" ]; then
            kill "$PID"
            echo "Stopped PID $PID"
        else
            echo "GridGain not running"
        fi
        ;;
    status)
        "$GRIDGAIN_HOME/bin/control.sh" --state
        ;;
    console)
        shift
        CONFIG="${1:-/opt/freeware/etc/gridgain/gridgain.xml}"
        exec "$GRIDGAIN_HOME/bin/ignite.sh" "$CONFIG"
        ;;
    *)
        echo "Usage: gridgain {start|stop|status|console} [config.xml]"
        echo ""
        echo "  start   - Start GridGain in background"
        echo "  stop    - Stop GridGain"
        echo "  status  - Show cluster state"
        echo "  console - Run GridGain in foreground"
        echo ""
        echo "JAVA_HOME: $JAVA_HOME (shared Semeru JDK, does not affect system Java)"
        echo "Config:    /opt/freeware/etc/gridgain/gridgain.xml"
        echo "Data:      /opt/freeware/var/lib/gridgain/"
        echo "Logs:      /opt/freeware/var/log/gridgain/"
        exit 1
        ;;
esac
WRAPEOF
chmod 755 %{buildroot}/opt/freeware/bin/gridgain

# Control utility wrapper
cat > %{buildroot}/opt/freeware/bin/gridgain-control << 'CTLEOF'
#!/bin/sh
JAVA_HOME=/opt/freeware/lib/jvm/semeru-25
IGNITE_HOME=/opt/freeware/lib/gridgain
export JAVA_HOME IGNITE_HOME
exec "$IGNITE_HOME/bin/control.sh" "$@"
CTLEOF
chmod 755 %{buildroot}/opt/freeware/bin/gridgain-control

# sqlline wrapper
cat > %{buildroot}/opt/freeware/bin/gridgain-sqlline << 'SQLEOF'
#!/bin/sh
JAVA_HOME=/opt/freeware/lib/jvm/semeru-25
IGNITE_HOME=/opt/freeware/lib/gridgain
export JAVA_HOME IGNITE_HOME
exec "$IGNITE_HOME/bin/sqlline.sh" "$@"
SQLEOF
chmod 755 %{buildroot}/opt/freeware/bin/gridgain-sqlline

%files
%defattr(-,root,system,-)
/opt/freeware/bin/gridgain
/opt/freeware/bin/gridgain-control
/opt/freeware/bin/gridgain-sqlline
/opt/freeware/lib/gridgain/bin
/opt/freeware/lib/gridgain/libs
/opt/freeware/lib/gridgain/config
/opt/freeware/lib/gridgain/LICENSE
/opt/freeware/lib/gridgain/NOTICE
%config(noreplace) /opt/freeware/etc/gridgain/gridgain.xml
%dir /opt/freeware/var/lib/gridgain
%dir /opt/freeware/var/log/gridgain

%post
echo ""
echo "GridGain Community Edition installed."
echo ""
echo "  Start:    gridgain start"
echo "  Stop:     gridgain stop"
echo "  Status:   gridgain status"
echo "  Console:  gridgain console"
echo "  SQL:      gridgain-sqlline"
echo "  Control:  gridgain-control --state"
echo ""
echo "Config: /opt/freeware/etc/gridgain/gridgain.xml"
echo "Data:   /opt/freeware/var/lib/gridgain/"
echo "Logs:   /opt/freeware/var/log/gridgain/"
echo ""
echo "Uses shared Semeru JDK at /opt/freeware/lib/jvm/semeru-25"
echo "Your system Java is NOT affected."
echo ""

%changelog
* Wed Mar 12 2026 LibrePower <hello@librepower.org> - 8.9.30-2.librepower
- Changed dependency: gridgain-jdk -> semeru-jdk25 (shared JDK)
- JDK path: /opt/freeware/lib/jvm/semeru-25 (reusable by other apps)
* Wed Mar 12 2026 LibrePower <hello@librepower.org> - 8.9.30-1.librepower
- Initial AIX port
- OpenJ9 compatibility patch (IgniteMBeanUtils.class)
