Name:           google-authenticator-setup
Version:        1.2
Release:        1.librepower.aix7.3
Summary:        Easy 2FA setup wizards for AIX/VIOS
License:        Apache-2.0
URL:            https://librepower.org

%description
Easy-to-use setup wizards for Google Authenticator 2FA on AIX and VIOS.
Includes both English (google-authenticator-setup) and Spanish
(google-authenticator-configura) versions.

v1.2: Improved NTP validation - detects unconfigured xntpd and high offset

%install
mkdir -p %{buildroot}/opt/freeware/bin
cp %{_sourcedir}/google-authenticator-setup %{buildroot}/opt/freeware/bin/
cp %{_sourcedir}/google-authenticator-configura %{buildroot}/opt/freeware/bin/
cp %{_sourcedir}/2fa-check %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/google-authenticator-setup
chmod 755 %{buildroot}/opt/freeware/bin/google-authenticator-configura
chmod 755 %{buildroot}/opt/freeware/bin/2fa-check

%files
%attr(755, root, system) /opt/freeware/bin/google-authenticator-setup
%attr(755, root, system) /opt/freeware/bin/google-authenticator-configura
%attr(755, root, system) /opt/freeware/bin/2fa-check

%changelog
* Tue Jan 21 2025 LibrePower <hello@librepower.org> - 1.2-1
- Improved NTP validation: now detects when xntpd is running but not synchronized
- Added offset verification: warns if >5s, blocks if >30s (TOTP window)
- Better error messages guiding users to check /etc/ntp.conf
- Still works for non-root users

* Sun Jan 19 2025 LibrePower <hello@librepower.org> - 1.1-1
- Fixed NTP detection for non-root users
- Added fallback methods: ps -ef and ntpq -p

* Wed Jan 15 2025 LibrePower <hello@librepower.org> - 1.0-1
- Initial release
