Name:           starship
Version:        1.24.2
Release:        2.librepower
Summary:        The minimal, blazing-fast, and infinitely customizable prompt
License:        ISC
Group:          Applications/System
URL:            https://github.com/starship/starship
Vendor:         LibrePower
Packager:       LibrePower <hello@librepower.org>

%description
Starship is the minimal, blazing-fast, and infinitely customizable prompt
for any shell. Works with Bash, Zsh, Fish, and many more.

Part of LibrePower - https://librepower.org

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/bin
cp /opt/freeware/bin/starship %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/starship

%files
%defattr(-,root,system,-)
/opt/freeware/bin/starship

%changelog
* Tue Feb 04 2025 LibrePower <hello@librepower.org> - 1.24.2-2.librepower
- Rebuild with generic AIX OS for AIX 7.2/7.3 compatibility
* Wed Jan 22 2025 LibrePower <hello@librepower.org> - 1.24.2-1.librepower
- Initial AIX port
