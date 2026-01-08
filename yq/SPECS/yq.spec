Name:           yq
Version:        4.40.5
Release:        1.librepower
Summary:        Portable command-line YAML/JSON/XML processor
License:        MIT
URL:            https://github.com/mikefarah/yq
Group:          Applications/Text

%description
yq is a lightweight and portable command-line YAML, JSON, and XML processor.
It uses jq-like syntax but works with YAML files as well as JSON and XML.

Features:
- Parse, filter, and transform YAML/JSON/XML
- Convert between formats (YAML <-> JSON <-> XML)
- Supports multiple documents
- Built-in operators and functions

Built with Go 1.24.11 for AIX by LibrePower.

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/bin
mkdir -p %{buildroot}/opt/freeware/share/doc/%{name}

cp /tmp/yq-build/yq %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/yq

%post
echo ""
echo "yq installed successfully!"
echo ""
echo "Usage examples:"
echo "  yq .key file.yaml          # Get value"
echo "  yq -o json file.yaml         # YAML to JSON"
echo "  yq -p json -o yaml file.json # JSON to YAML"
echo ""

%files
%defattr(-,root,system,-)
/opt/freeware/bin/yq

%changelog
* Wed Jan 08 2025 LibrePower <hello@librepower.org> - 4.40.5-1.librepower
- Initial AIX port
- Compiled with Go 1.24.11 official toolchain
- 64-bit XCOFF binary
