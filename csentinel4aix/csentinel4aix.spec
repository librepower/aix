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
AIX port of C-Sentinel (https://github.com/williamofai/c-sentinel).
Original project by William (@williamofai).

C-Sentinel is a lightweight, portable system prober that captures
"system fingerprints" for AI-assisted analysis of non-obvious risks.
Features process monitoring, network analysis with PID attribution
for 70+ ports (SSH, databases, IBM middleware, SAP), configuration
drift detection, and baseline learning.

New in 0.6.0:
- AIX native audit integration (-a flag) with brute-force detection
- Full file integrity mode (-F flag) - 171 critical files (PowerSC RTC comparable)
- SIEM integration (-S/-L flags) - Real-time events to QRadar, XSIAM, Wazuh
- Based on CIS AIX Benchmark, DoD STIG, and IBM Security guidelines

This AIX port provides full feature parity supporting AIX 7.1, 7.2, and 7.3.

%prep
%setup -q

%build
# Binaries are pre-built in the source tarball
# If you need to rebuild, uncomment below:
# export PATH=/opt/freeware/bin:$PATH
# export CC=/opt/freeware/bin/gcc
# /opt/freeware/bin/gmake "CC=$CC"
echo "Using pre-built binaries"

%install
rm -rf $RPM_BUILD_ROOT

# Create directories
mkdir -p $RPM_BUILD_ROOT/opt/freeware/bin
mkdir -p $RPM_BUILD_ROOT/etc/sentinel
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/csentinel
mkdir -p $RPM_BUILD_ROOT/var/lib/sentinel

# Install binaries (AIX compatible)
cp bin/sentinel $RPM_BUILD_ROOT/opt/freeware/bin/
cp bin/sentinel-diff $RPM_BUILD_ROOT/opt/freeware/bin/
chmod 0755 $RPM_BUILD_ROOT/opt/freeware/bin/sentinel
chmod 0755 $RPM_BUILD_ROOT/opt/freeware/bin/sentinel-diff

# Install configuration template
cat > $RPM_BUILD_ROOT/etc/sentinel/config << 'EOF'
# C-Sentinel Configuration for AIX

[sentinel]
# Baseline storage location
baseline_path = /var/lib/sentinel/baseline.dat

# Default monitoring options
include_network = true
watch_interval = 300

# Alert thresholds
zombie_threshold = 5
high_fd_threshold = 1000
stuck_process_minutes = 5
EOF

# Install documentation (AIX compatible)
cp README.md $RPM_BUILD_ROOT/usr/share/doc/csentinel/
cp README.AIX.md $RPM_BUILD_ROOT/usr/share/doc/csentinel/
cp AIX_PORT_STATUS.md $RPM_BUILD_ROOT/usr/share/doc/csentinel/
cp LICENSE $RPM_BUILD_ROOT/usr/share/doc/csentinel/
chmod 0644 $RPM_BUILD_ROOT/usr/share/doc/csentinel/*

%post
# Create sentinel user if it doesn't exist
if ! grep -q "^sentinel:" /etc/passwd 2>/dev/null; then
    mkuser id=999 pgrp=system home=/var/lib/sentinel shell=/bin/false sentinel 2>/dev/null || true
fi

# Set permissions
chown -R sentinel:system /var/lib/sentinel 2>/dev/null || true
chmod 750 /var/lib/sentinel

# Create SRC subsystem for continuous monitoring (optional)
if command -v mkssys >/dev/null 2>&1; then
    # Remove old subsystem if exists
    rmssys -s csentinel 2>/dev/null || true

    # Create new SRC subsystem
    mkssys -s csentinel \
           -p /opt/freeware/bin/sentinel \
           -u 0 \
           -a "-w -i 300 -n" \
           -O -Q -S -n 15 -f 9 \
           -R 2>/dev/null || true

    echo ""
    echo "=========================================="
    echo "C-Sentinel 0.6.0 installed successfully!"
    echo "=========================================="
    echo ""
    echo "Quick start:"
    echo "  sentinel -q -n          # Quick analysis with network"
    echo "  sentinel -q -n -a       # Include security audit events"
    echo "  sentinel -F -q          # Full file integrity (~171 files)"
    echo "  sentinel -l -n          # Learn baseline"
    echo "  sentinel -b -q -n       # Compare against baseline"
    echo ""
    echo "Security audit (requires: /usr/sbin/audit start):"
    echo "  sentinel -q -a          # Include audit events"
    echo "  sentinel -F -q -n -a    # Full analysis"
    echo ""
    echo "SIEM integration (QRadar, XSIAM, Wazuh):"
    echo "  sentinel -w -i 60 -n -a -S 10.0.0.50:514       # Syslog (CEF)"
    echo "  sentinel -w -i 60 -n -a -S 10.0.0.50:514 -R json  # JSON"
    echo "  sentinel -w -i 60 -n -a -L /var/log/sentinel.log  # Logfile"
    echo ""
    echo "Continuous monitoring (optional):"
    echo "  startsrc -s csentinel   # Start background monitoring"
    echo "  lssrc -s csentinel      # Check status"
    echo "  stopsrc -s csentinel    # Stop monitoring"
    echo ""
    echo "Documentation: /usr/share/doc/csentinel/"
    echo "Configuration: /etc/sentinel/config"
    echo "=========================================="
    echo ""
fi

%preun
# Stop and remove SRC subsystem if exists
if command -v stopsrc >/dev/null 2>&1; then
    stopsrc -s csentinel 2>/dev/null || true
    rmssys -s csentinel 2>/dev/null || true
fi

%postun
# Clean up data directory if package is being removed (not upgraded)
if [ "$1" = "0" ]; then
    rm -rf /var/lib/sentinel 2>/dev/null || true
fi

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,system,-)
/opt/freeware/bin/sentinel
/opt/freeware/bin/sentinel-diff
%dir /etc/sentinel
%config(noreplace) /etc/sentinel/config
%dir /var/lib/sentinel
%dir /usr/share/doc/csentinel
%doc /usr/share/doc/csentinel/README.md
%doc /usr/share/doc/csentinel/README.AIX.md
%doc /usr/share/doc/csentinel/AIX_PORT_STATUS.md
%license /usr/share/doc/csentinel/LICENSE

%changelog
* Wed Jan 22 2026 LibrePower Team <hello@librepower.org> - 0.6.0-2.librepower
- SIEM integration for QRadar, Palo Alto XSIAM, Wazuh, Splunk
- Syslog output (CEF/JSON format) via -S flag
- Log file output (JSON lines) via -L flag for Wazuh/Filebeat agents
- Email alerts for critical events via -M flag
- Configurable alert threshold via -T flag
- QA test suite for SIEM features

* Wed Jan 22 2026 LibrePower Team <hello@librepower.org> - 0.6.0-1.librepower
- AIX native audit integration via -a flag
- Full file integrity mode via -F flag (171 critical files)
- PowerSC RTC comparable file monitoring
- Based on CIS AIX Benchmark, DoD STIG, and IBM Security guidelines
- Brute force detection (5+ consecutive auth failures)
- Risk scoring and security posture assessment
- Expanded default config file list (12 files)

* Wed Jan 08 2026 LibrePower Team <hello@librepower.org> - 1.0.0-4.librepower
- Changed install path from /usr/local/bin to /opt/freeware/bin for PATH consistency

* Sun Jan 05 2025 LibrePower Team <hello@librepower.org> - 1.0.0-1
- Initial AIX port release
- 95%% feature parity with Linux version
- PID attribution for 70+ network ports (SSH, databases, IBM middleware, SAP)
- Support for AIX 7.1, 7.2, 7.3
- libperfstat integration for system monitoring
- Process monitoring via /proc/psinfo
- Configuration drift detection with SHA256 checksums
- Baseline learning and anomaly detection
- Process chain analysis for security
- SRC (System Resource Controller) integration
