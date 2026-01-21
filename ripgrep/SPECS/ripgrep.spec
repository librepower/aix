Name:           ripgrep
Version:        15.1.0
Release:        1.librepower.aix7.3
Summary:        Fast regex-based search tool (like grep but faster)
License:        MIT/Unlicense
URL:            https://github.com/BurntSushi/ripgrep

%description
ripgrep (rg) is a line-oriented search tool that recursively searches
the current directory for a regex pattern. By default, ripgrep will
respect gitignore rules and automatically skip hidden files/directories
and binary files.

ripgrep has first class support on Windows, macOS and Linux, with binary
downloads available for every release. ripgrep is similar to other popular
search tools like The Silver Searcher, ack and grep.

Features:
- Automatically respects .gitignore
- Faster than grep, ag, ack, and most other tools
- Smart case search by default
- Search compressed files
- Unicode support

Compiled with IBM Open SDK for Rust on AIX.

%install
mkdir -p %{buildroot}/opt/freeware/bin
mkdir -p %{buildroot}/opt/freeware/share/man/man1
mkdir -p %{buildroot}/opt/freeware/share/bash-completion/completions
mkdir -p %{buildroot}/opt/freeware/share/zsh/site-functions
mkdir -p %{buildroot}/opt/freeware/share/fish/vendor_completions.d

cp %{_sourcedir}/rg %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/rg

# Man page
cp %{_sourcedir}/rg.1 %{buildroot}/opt/freeware/share/man/man1/

# Shell completions
cp %{_sourcedir}/rg.bash %{buildroot}/opt/freeware/share/bash-completion/completions/rg
cp %{_sourcedir}/_rg %{buildroot}/opt/freeware/share/zsh/site-functions/
cp %{_sourcedir}/rg.fish %{buildroot}/opt/freeware/share/fish/vendor_completions.d/

%files
%attr(755, root, system) /opt/freeware/bin/rg
/opt/freeware/share/man/man1/rg.1
/opt/freeware/share/bash-completion/completions/rg
/opt/freeware/share/zsh/site-functions/_rg
/opt/freeware/share/fish/vendor_completions.d/rg.fish

%changelog
* Tue Jan 21 2025 LibrePower <hello@librepower.org> - 15.1.0-1
- Initial release for AIX
- Compiled with IBM Open SDK for Rust 1.90
- First modern grep alternative available natively on AIX
