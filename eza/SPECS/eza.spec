Name:           eza
Version:        0.23.4
Release:        2.librepower
Summary:        A modern replacement for ls
License:        MIT
Group:          Applications/File
URL:            https://github.com/eza-community/eza
Vendor:         LibrePower
Packager:       LibrePower <hello@librepower.org>

%description
eza is a modern, maintained replacement for the venerable file-listing
command-line program ls that ships with Unix and Linux operating systems.

eza has more features and better defaults. It uses colours to distinguish
file types and metadata. It knows about symlinks, extended attributes, and Git.

Part of LibrePower - https://librepower.org

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/bin
cp /opt/freeware/bin/eza %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/eza

%files
%defattr(-,root,system,-)
/opt/freeware/bin/eza

%changelog
* Tue Feb 04 2025 LibrePower <hello@librepower.org> - 0.23.4-2.librepower
- Rebuild with generic AIX OS for AIX 7.2/7.3 compatibility
* Wed Jan 22 2025 LibrePower <hello@librepower.org> - 0.23.4-1.librepower
- Initial AIX port
