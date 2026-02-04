Name:           fd
Version:        10.3.0
Release:        2.librepower
Summary:        A simple, fast and user-friendly alternative to find
License:        MIT
Group:          Applications/File
URL:            https://github.com/sharkdp/fd
Vendor:         LibrePower
Packager:       LibrePower <hello@librepower.org>

%description
fd is a program to find entries in your filesystem. It is a simple, fast and
user-friendly alternative to find. While it does not aim to support all of
find's powerful functionality, it provides sensible defaults for a majority
of use cases.

Part of LibrePower - https://librepower.org

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/bin
cp /opt/freeware/bin/fd %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/fd

%files
%defattr(-,root,system,-)
/opt/freeware/bin/fd

%changelog
* Tue Feb 04 2025 LibrePower <hello@librepower.org> - 10.3.0-2.librepower
- Rebuild with generic AIX OS for AIX 7.2/7.3 compatibility
* Wed Jan 22 2025 LibrePower <hello@librepower.org> - 10.3.0-1.librepower
- Initial AIX port
