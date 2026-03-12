Summary: Container engine for AIX using WPARs — podman-compatible CLI
Name: podman-aix
Version: 0.5.0
Release: 1.librepower
License: GPLv3
Group: System/Containers
URL: https://gitlab.com/librepower/podman-aix
Packager: LibrePower <hello@librepower.org>

%description
podman-aix is a container engine for IBM AIX that leverages System WPARs
(Workload Partitions) to provide container-like functionality with a
podman-compatible command-line interface.

Features:
  - Container lifecycle: create, start, stop, rm, run, exec, ps, inspect
  - Image management: build, commit, images, push, pull
  - Built-in image registry with token authentication
  - Automatic network configuration (IP alias on host interface)
  - Blue/green deployment demo (podman demo)
  - Layer-based images with fast cloning via savewpar/restwpar

Binary installed as /opt/freeware/bin/podman. State in /var/lib/podman-aix/.

%prep
# No prep — binary is cross-compiled from Go source

%build
# Cross-compiled on build host:
# CGO_ENABLED=0 GOOS=aix GOARCH=ppc64 go build -ldflags "..." -o podman .

%install
rm -rf %{buildroot}

# Binary
mkdir -p %{buildroot}/opt/freeware/bin
cp %{_sourcedir}/podman %{buildroot}/opt/freeware/bin/podman
chmod 0755 %{buildroot}/opt/freeware/bin/podman

# State directories
mkdir -p %{buildroot}/var/lib/podman-aix/images
mkdir -p %{buildroot}/var/lib/podman-aix/logs
mkdir -p %{buildroot}/var/lib/podman-aix/registry

%post
# Create state directories if they don't exist (upgrade case)
mkdir -p /var/lib/podman-aix/images 2>/dev/null || true
mkdir -p /var/lib/podman-aix/logs 2>/dev/null || true
mkdir -p /var/lib/podman-aix/registry 2>/dev/null || true

echo ""
echo "podman-aix %{version} installed successfully."
echo ""
echo "Quick start:"
echo "  podman image build aix73-minimal   # Build base image (~10 min, one-time)"
echo "  podman run -d --name myapp aix73-minimal"
echo "  podman exec myapp hostname"
echo "  podman demo                        # Run multi-container demo"
echo ""
echo "Documentation: https://gitlab.com/librepower/podman-aix"
echo ""

%preun
# Stop and remove any running demo containers on uninstall
if [ "$1" = "0" ]; then
    /opt/freeware/bin/podman demo --cleanup 2>/dev/null || true
fi

%files
%defattr(-,root,system)
/opt/freeware/bin/podman
%dir /var/lib/podman-aix
%dir /var/lib/podman-aix/images
%dir /var/lib/podman-aix/logs
%dir /var/lib/podman-aix/registry

%changelog
* Wed Mar 12 2026 LibrePower <hello@librepower.org> - 0.5.0-1.librepower
- Initial public release
- Container lifecycle: create, start, stop, rm, run, exec, ps, inspect, logs
- Image management: build, commit, images, push, pull
- Built-in HTTP image registry with Bearer token auth
- Automatic network detection and IP allocation (.200-.253 range)
- Layer-based images with manifest tracking
- Blue/green deployment demo (podman demo)
- Cross-compiled Go binary (CGO_ENABLED=0, static)
