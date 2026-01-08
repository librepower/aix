Name:           gron
Version:        0.7.1
Release:        1.librepower
Summary:        Make JSON greppable
License:        MIT
URL:            https://github.com/tomnomnom/gron
Group:          Applications/Text

%description
gron transforms JSON into discrete assignments to make it easier to grep
and see the absolute path to each value. It also works in reverse, so you
can pipe grepped results back to gron to reconstruct valid JSON.

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/bin
mkdir -p %{buildroot}/opt/freeware/share/doc/%{name}

cp /tmp/gron/gron %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/gron

cp /tmp/gron/README.mkd %{buildroot}/opt/freeware/share/doc/%{name}/README.md
chmod 644 %{buildroot}/opt/freeware/share/doc/%{name}/README.md

%post
echo ""
echo "gron installed - Make JSON greppable!"
echo ""
echo "Examples:"
echo "  curl -s api.example.com | gron       # JSON to greppable"
echo "  gron data.json | grep 'users'        # Search JSON paths"
echo "  gron -u < gron-output.txt            # Back to JSON"
echo ""

%files
%defattr(-,root,system,-)
/opt/freeware/bin/gron
/opt/freeware/share/doc/%{name}/README.md

%changelog
* Wed Jan 08 2026 LibrePower <hello@librepower.org> - 0.7.1-1.librepower
- Initial AIX port
- Built with Go 1.24.11 (official)
- 64-bit XCOFF binary
