Name:           csentinel4aix
Version:        1.0.0
Release:        1%{?dist}
Summary:        Semantic Observability for AIX Systems

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

This AIX port provides 95%% feature parity supporting AIX 7.1, 7.2, and 7.3.

%prep
%setup -q

%build
/opt/freeware/bin/make -f Makefile.aix

%install
rm -rf $RPM_BUILD_ROOT

# Create directories
mkdir -p $RPM_BUILD_ROOT/usr/local/bin
mkdir -p $RPM_BUILD_ROOT/etc/sentinel
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/csentinel
mkdir -p $RPM_BUILD_ROOT/var/lib/sentinel

# Install binaries (AIX compatible)
cp bin/sentinel $RPM_BUILD_ROOT/usr/local/bin/
cp bin/sentinel-diff $RPM_BUILD_ROOT/usr/local/bin/
chmod 0755 $RPM_BUILD_ROOT/usr/local/bin/sentinel
chmod 0755 $RPM_BUILD_ROOT/usr/local/bin/sentinel-diff

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
           -p /usr/local/bin/sentinel \
           -u 0 \
           -a "-w -i 300 -n" \
           -O -Q -S -n 15 -f 9 \
           -R 2>/dev/null || true

    echo ""
    echo "=========================================="
    echo "C-Sentinel installed successfully!"
    echo "=========================================="
    echo ""
    echo "Quick start:"
    echo "  sentinel -q -n          # Quick analysis"
    echo "  sentinel -l -n          # Learn baseline"
    echo "  sentinel -b -q -n       # Compare against baseline"
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
/usr/local/bin/sentinel
/usr/local/bin/sentinel-diff
%dir /etc/sentinel
%config(noreplace) /etc/sentinel/config
%dir /var/lib/sentinel
%dir /usr/share/doc/csentinel
%doc /usr/share/doc/csentinel/README.md
%doc /usr/share/doc/csentinel/README.AIX.md
%doc /usr/share/doc/csentinel/AIX_PORT_STATUS.md
%license /usr/share/doc/csentinel/LICENSE

%changelog
* Sun Jan 05 2025 LibrePower Team <team@librepower.org> - 1.0.0-1
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
