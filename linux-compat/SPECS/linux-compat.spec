#============================================================================
# linux-compat.spec - GNU/Linux CLI Experience for AIX
#============================================================================
# Part of LibrePower - Open Source for IBM Power
# https://github.com/librepower/aix
#============================================================================

Name:           linux-compat
Version:        2.2
Release:        1.librepower
Summary:        GNU/Linux CLI experience for AIX system administrators
License:        GPL-3.0
Group:          System Environment/Base
URL:            https://github.com/librepower/aix
Vendor:         SIXE - IBM Business Partner
Packager:       Hugo Blanco <hugo.blanco@sixe.eu>

Source0:        %{name}-%{version}.tar

BuildArch:      noarch

Requires:       coreutils
Requires:       findutils
Requires:       grep
Requires:       sed
Requires:       gawk
Requires:       diffutils
Requires:       tar
Requires:       xz
Requires:       vim-enhanced
Requires:       tmux
Requires:       jq
Requires:       tree
Requires:       bash

%description
Linux Compatibility for AIX provides a familiar GNU/Linux command-line
experience for AIX system administrators coming from Linux environments.

This package does NOT compile any GNU tools - IBM already did that in AIX
Toolbox. We provide a configuration layer that makes them the default.

What IBM provides (as dependencies):
- GNU coreutils, grep, sed, gawk, findutils, tar, vim, tmux, jq, tree
- Production-ready, IBM-supported packages

What this package adds:
- Shell profile with PATH configuration for GNU tools
- Emulated Linux commands: watch, pgrep, pkill, free
- systemctl/service wrappers for AIX SRC
- Convenient aliases and functions
- Bilingual documentation (EN/ES)

Safe installation - only affects interactive shells.
Scripts using #!/bin/sh are NOT affected.

Credit: IBM did the hard work of porting GNU tools. We just configure them.

Compiled for AIX by SIXE - IBM Business Partner
https://sixe.eu | Part of LibrePower initiative (https://librepower.org)

%prep
%setup -c -n %{name}-%{version}

%install
rm -rf %{buildroot}

mkdir -p %{buildroot}/opt/librepower/linux-compat
mkdir -p %{buildroot}/opt/freeware/bin
mkdir -p %{buildroot}/usr/share/doc/%{name}-%{version}

cp linux-compat/linux-compat-setup.sh %{buildroot}/opt/librepower/linux-compat/
cp linux-compat/linux-compat.profile %{buildroot}/opt/librepower/linux-compat/
cp linux-compat/README.md %{buildroot}/usr/share/doc/%{name}-%{version}/
cp linux-compat/README_ES.md %{buildroot}/usr/share/doc/%{name}-%{version}/

ln -sf /opt/librepower/linux-compat/linux-compat-setup.sh %{buildroot}/opt/freeware/bin/linux-compat-setup

%files
%defattr(-,root,system,-)
%dir /opt/librepower
%dir /opt/librepower/linux-compat
/opt/librepower/linux-compat/linux-compat-setup.sh
/opt/librepower/linux-compat/linux-compat.profile
/opt/freeware/bin/linux-compat-setup
%doc /usr/share/doc/%{name}-%{version}/README.md
%doc /usr/share/doc/%{name}-%{version}/README_ES.md

%post
echo ""
echo "╔═══════════════════════════════════════════════════════════════════════╗"
echo "║           Linux Compatibility for AIX - Installed Successfully        ║"
echo "╠═══════════════════════════════════════════════════════════════════════╣"
echo "║                                                                       ║"
echo "║  To install for current user, run:                                    ║"
echo "║      linux-compat-setup install                                       ║"
echo "║                                                                       ║"
echo "║  Then activate with:                                                  ║"
echo "║      source ~/.linux-compat-profile                                   ║"
echo "║                                                                       ║"
echo "║  Or simply start a new shell session.                                 ║"
echo "║                                                                       ║"
echo "║  Type 'linuxhelp' after activation for quick reference.               ║"
echo "║                                                                       ║"
echo "╠═══════════════════════════════════════════════════════════════════════╣"
echo "║  SIXE - IBM Business Partner (https://sixe.eu)                        ║"
echo "║  Part of LibrePower (https://librepower.org)                          ║"
echo "╚═══════════════════════════════════════════════════════════════════════╝"
echo ""

%preun
if [ "$1" = "0" ]; then
    echo ""
    echo "NOTE: User profiles (~/.linux-compat-profile) are not removed."
    echo "      Run 'linux-compat-setup uninstall' before removing if desired."
    echo ""
fi

%changelog
* Wed Jan 08 2025 Hugo Blanco <hugo.blanco@sixe.eu> - 2.2-1.librepower
- Added MANPATH configuration for /opt/freeware/share/man
- Now 'man fzf', 'man nano', etc. work out of the box
- Colorized man pages

* Sun Jan 05 2025 Hugo Blanco <hugo.blanco@sixe.eu> - 2.1-1.librepower
- Replaced custom svcstart/svcstop with proper systemctl/service wrappers
- systemctl now supports groups with @ prefix (e.g., systemctl start @tcpip)
- Added service command for SysV-style syntax
- Added is-active, list-groups commands to systemctl
- Improved help and documentation

* Sat Jan 04 2025 Hugo Blanco <hugo.blanco@sixe.eu> - 2.0-1.librepower
- Initial RPM release
- Native GNU tools: ls, grep, sed, awk, find, tar, diff, vim, jq, tree, tmux
- Emulated functions: watch, pgrep, pkill, free
- Safe installation - only affects interactive shells
- Bilingual documentation (English/Spanish)
