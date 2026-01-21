Name:           stg-tui
Version:        1.0.0
Release:        1.librepower.aix7.3
Summary:        Interactive TUI for AIX LVM/Storage exploration
License:        Apache-2.0
URL:            https://librepower.org

%description
stg-tui is a modern Terminal User Interface (TUI) for exploring AIX
storage configuration. It provides an interactive view of:

- Volume Groups (VGs) with status and capacity
- Logical Volumes (LVs) in each VG
- Physical Volumes (PVs) and their allocation

Features:
- Fast, lightweight Go binary
- No dependencies - single binary
- Color-coded status display
- Keyboard navigation (arrow keys, q to quit)
- Real-time LVM data from system commands

This is a LibrePower original tool, built exclusively for AIX.

%install
mkdir -p %{buildroot}/opt/freeware/bin
cp %{_sourcedir}/stg-tui %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/stg-tui

%post
echo ""
echo "stg-tui installed! Run 'stg-tui' to explore your storage."
echo ""
echo "Keyboard shortcuts:"
echo "  Up/Down  - Navigate volume groups"
echo "  l        - Show Logical Volumes"
echo "  p        - Show Physical Volumes"  
echo "  q        - Quit"
echo ""

%files
%attr(755, root, system) /opt/freeware/bin/stg-tui

%changelog
* Tue Jan 21 2025 LibrePower <hello@librepower.org> - 1.0.0-1
- Initial release
- TUI interface for AIX LVM exploration
- Volume Group, LV, and PV visualization
- Built with Go 1.24.11 and tview
