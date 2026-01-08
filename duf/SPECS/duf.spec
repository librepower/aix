Name:           duf
Version:        0.9.1
Release:        1.librepower
Summary:        Disk Usage/Free Utility - a better df alternative
License:        MIT
URL:            https://github.com/muesli/duf
Group:          Applications/System

%description
duf is a modern replacement for df. It displays disk usage with a colorful,
easy-to-read table format. Features include sorting, filtering by filesystem
type, JSON output, and automatic hiding of pseudo filesystems.

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/bin
mkdir -p %{buildroot}/opt/freeware/share/doc/%{name}

cp /tmp/duf/duf %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/duf

cp /tmp/duf/README.md %{buildroot}/opt/freeware/share/doc/%{name}/
chmod 644 %{buildroot}/opt/freeware/share/doc/%{name}/README.md

%post
echo ""
echo "duf installed - Disk Usage/Free utility!"
echo ""
echo "Usage:"
echo "  duf              # Show all filesystems"
echo "  duf /home /tmp   # Show specific paths"
echo "  duf --json       # JSON output"
echo ""

%files
%defattr(-,root,system,-)
/opt/freeware/bin/duf
/opt/freeware/share/doc/%{name}/README.md

%changelog
* Wed Jan 08 2026 LibrePower <hello@librepower.org> - 0.9.1-1.librepower
- Initial AIX port
- Added filesystems_aix.go and mounts_aix.go
- PR submitted upstream: https://github.com/muesli/duf/pull/354
- Built with Go 1.24.11 (official)
