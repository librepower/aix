Name:           delta
Version:        0.18.2
Release:        1.librepower
Summary:        A syntax-highlighting pager for git, diff, and grep output
License:        MIT
URL:            https://github.com/dandavison/delta
Group:          Development/Tools
Packager:       Hugo Blanco <hugo.blanco@sixe.eu>

%description
Delta provides language syntax highlighting, within-line insertion/deletion
detection, and restructured diff output for git on the command line.

Features:
- Language syntax highlighting with color themes
- Within-line highlights based on a Levenshtein edit inference algorithm
- Side-by-side view with line-wrapping
- Line numbering
- Navigate to next/previous diff with keyboard shortcuts
- Improved merge conflict display
- Improved git blame display

First Rust application ported to AIX for LibrePower.
https://librepower.org

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/bin
mkdir -p %{buildroot}/opt/freeware/share/doc/%{name}

cp /tmp/delta/target/release/delta %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/delta

cat > %{buildroot}/opt/freeware/share/doc/%{name}/README << 'DOCEOF'
Delta - A syntax-highlighting pager for git and diff output

Usage:
  git diff | delta
  diff -u file1 file2 | delta
  delta file1 file2

Configure git to use delta:
  git config --global core.pager delta
  git config --global interactive.diffFilter "delta --color-only"

More info: https://github.com/dandavison/delta
DOCEOF
chmod 644 %{buildroot}/opt/freeware/share/doc/%{name}/README

%post
echo ""
echo "Delta installed successfully!"
echo ""
echo "To use delta as your git pager, run:"
echo "  git config --global core.pager delta"
echo ""

%files
%defattr(-,root,system,-)
/opt/freeware/bin/delta
/opt/freeware/share/doc/%{name}/README

%changelog
* Wed Jan 08 2025 LibrePower <hello@librepower.org> - 0.18.2-1.librepower
- Initial AIX port
- Compiled with IBM Open SDK for Rust 1.90
- Patched nix crate for AIX timespec compatibility
