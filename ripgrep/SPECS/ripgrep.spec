Name:           ripgrep
Version:        14.1.1
Release:        1.librepower
Summary:        Fast regex-based search tool
License:        MIT
URL:            https://github.com/BurntSushi/ripgrep
Group:          Applications/Text

%description
ripgrep is a line-oriented search tool that recursively searches the current
directory for a regex pattern. By default, ripgrep will respect gitignore
rules and automatically skip hidden files/directories and binary files.

ripgrep has first-class support on Windows, macOS and Linux, with binary
downloads available for every release.

Built with Rust for AIX by LibrePower.

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/bin
mkdir -p %{buildroot}/opt/freeware/share/doc/%{name}

cp /tmp/rg-build/rg %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/rg

cp /tmp/rg-build/README.md %{buildroot}/opt/freeware/share/doc/%{name}/
cp /tmp/rg-build/COPYING %{buildroot}/opt/freeware/share/doc/%{name}/
cp /tmp/rg-build/LICENSE-MIT %{buildroot}/opt/freeware/share/doc/%{name}/
chmod 644 %{buildroot}/opt/freeware/share/doc/%{name}/*

%post
echo ""
echo "ripgrep installed successfully!"
echo ""
echo "Usage: rg PATTERN [PATH]"
echo ""
echo "Examples:"
echo "  rg 'error' /var/log     # Search for 'error' in logs"
echo "  rg -i 'todo'            # Case-insensitive search"
echo "  rg -l 'function'        # List files with matches"
echo ""

%files
%defattr(-,root,system,-)
/opt/freeware/bin/rg
/opt/freeware/share/doc/%{name}/README.md
/opt/freeware/share/doc/%{name}/COPYING
/opt/freeware/share/doc/%{name}/LICENSE-MIT

%changelog
* Wed Jan 08 2025 LibrePower <hello@librepower.org> - 14.1.1-1.librepower
- Initial AIX port
- Compiled with IBM Open SDK for Rust 1.90
- 64-bit XCOFF binary
