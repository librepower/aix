Name:           age
Version:        1.2.0
Release:        1.librepower
Summary:        Simple, modern file encryption tool
License:        BSD-3-Clause
URL:            https://github.com/FiloSottile/age
Group:          Applications/Security

%description
age is a simple, modern and secure encryption tool with small explicit keys,
no config options, and UNIX-style composability.

Features:
- Simple and secure encryption
- SSH key support (encrypt to GitHub users!)
- Small, auditable codebase
- No config files
- Passphrase support

Built with Go 1.24.11 for AIX by LibrePower.

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/bin

cp /tmp/age-build/age %{buildroot}/opt/freeware/bin/
cp /tmp/age-build/age-keygen %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/age
chmod 755 %{buildroot}/opt/freeware/bin/age-keygen

%post
echo ""
echo "age encryption tool installed!"
echo ""
echo "Quick start:"
echo "  age-keygen -o key.txt          # Generate key pair"
echo "  age -r <recipient> file.txt    # Encrypt"
echo "  age -d -i key.txt file.age     # Decrypt"
echo ""

%files
%defattr(-,root,system,-)
/opt/freeware/bin/age
/opt/freeware/bin/age-keygen

%changelog
* Wed Jan 08 2025 LibrePower <hello@librepower.org> - 1.2.0-1.librepower
- Initial AIX port
- Compiled with Go 1.24.11 official toolchain
