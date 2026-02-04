Name:           gping
Version:        1.20.1
Release:        2.librepower
Summary:        Ping, but with a graph
License:        MIT
Group:          Applications/Internet
URL:            https://github.com/orf/gping
Vendor:         LibrePower
Packager:       LibrePower <hello@librepower.org>

%description
gping is a ping utility with a real-time graph in your terminal.

Part of LibrePower - https://librepower.org

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/bin
cp /opt/freeware/bin/gping %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/gping

%files
%defattr(-,root,system,-)
/opt/freeware/bin/gping

%changelog
* Tue Feb 04 2025 LibrePower <hello@librepower.org> - 1.20.1-2.librepower
- Rebuild with generic AIX OS for AIX 7.2/7.3 compatibility
* Wed Jan 22 2025 LibrePower <hello@librepower.org> - 1.20.1-1.librepower
- Initial AIX port
