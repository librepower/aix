Name:           librepower-compat
Version:        1.0
Release:        1.librepower
Summary:        AIX 7.1/7.2 compatibility shim for LibrePower packages
License:        MIT
URL:            https://aix.librepower.org
Vendor:         LibrePower
Packager:       LibrePower <hello@librepower.org>

%description
Provides compatibility shim for packages compiled on AIX 7.3 to run on 
AIX 7.1 and 7.2. Supplies the single_locale symbol missing from older libc.

This package is automatically installed as a dependency when needed.
Safe: uses /opt/freeware/lib/librepower/, no system library modification.

%install
mkdir -p %{buildroot}/opt/freeware/lib/librepower
cp /tmp/libcompat.a %{buildroot}/opt/freeware/lib/librepower/

%files
%dir /opt/freeware/lib/librepower
/opt/freeware/lib/librepower/libcompat.a

%changelog
* Tue Feb 04 2026 LibrePower <hello@librepower.org> - 1.0-1
- Initial release
- Provides single_locale symbol for AIX 7.1/7.2 compatibility
