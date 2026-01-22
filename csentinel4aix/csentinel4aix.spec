Name:           csentinel4aix
Version:        0.6.0
Release:        2.librepower
Summary:        Semantic Observability for AIX Systems with Audit Integration

License:        MIT
URL:            https://github.com/librepower/c-sentinel4aix
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc >= 8.0
BuildRequires:  make
Requires:       /bin/sh

%description
AIX port of C-Sentinel. Features process monitoring, network analysis,
configuration drift detection, AIX audit integration, and full file
integrity mode (171 critical files, PowerSC RTC comparable).

%prep
%setup -q

%build
echo "Using pre-built binaries"

%install
rm -rf $RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT/opt/freeware/bin
mkdir -p $RPM_BUILD_ROOT/opt/freeware/libexec/sentinel
mkdir -p $RPM_BUILD_ROOT/etc/sentinel
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/csentinel
mkdir -p $RPM_BUILD_ROOT/var/lib/sentinel

cp bin/sentinel $RPM_BUILD_ROOT/opt/freeware/bin/
cp bin/sentinel-diff $RPM_BUILD_ROOT/opt/freeware/bin/
chmod 0755 $RPM_BUILD_ROOT/opt/freeware/bin/sentinel
chmod 0755 $RPM_BUILD_ROOT/opt/freeware/bin/sentinel-diff

cp libexec/*.sh $RPM_BUILD_ROOT/opt/freeware/libexec/sentinel/
chmod 0755 $RPM_BUILD_ROOT/opt/freeware/libexec/sentinel/*.sh

cat > $RPM_BUILD_ROOT/etc/sentinel/config << 'CONFEOF'
# C-Sentinel Configuration for AIX
[sentinel]
baseline_path = /var/lib/sentinel/baseline.dat
include_network = true
watch_interval = 300
CONFEOF

cp README.md README.AIX.md AIX_PORT_STATUS.md LICENSE $RPM_BUILD_ROOT/usr/share/doc/csentinel/
chmod 0644 $RPM_BUILD_ROOT/usr/share/doc/csentinel/*

%post
echo ""
echo "=========================================="
echo "C-Sentinel 0.6.0 installed!"
echo "=========================================="
echo ""
echo "  sentinel -q -n -a    # Quick analysis with audit"
echo "  sentinel -F -q       # Full file integrity check"
echo ""
echo "OPTIONAL - Prevent audit trail disk fill:"
echo "  /opt/freeware/libexec/sentinel/setup-audit-rotation.sh"
echo ""

%preun
if [ "$1" = "0" ]; then
    CRONTAB=/var/spool/cron/crontabs/root
    [ -f "$CRONTAB" ] && grep -v "sentinel/audit-rotate" "$CRONTAB" > "${CRONTAB}.new" && mv "${CRONTAB}.new" "$CRONTAB" 2>/dev/null
fi

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,system,-)
/opt/freeware/bin/sentinel
/opt/freeware/bin/sentinel-diff
%dir /opt/freeware/libexec/sentinel
/opt/freeware/libexec/sentinel/audit-rotate.sh
/opt/freeware/libexec/sentinel/setup-audit-rotation.sh
%dir /etc/sentinel
%config(noreplace) /etc/sentinel/config
%dir /var/lib/sentinel
%dir /usr/share/doc/csentinel
%doc /usr/share/doc/csentinel/*

%changelog
* Wed Jan 22 2026 LibrePower Team <hello@librepower.org> - 0.6.0-2.librepower
- Added optional audit trail rotation
- Safe for existing installations
