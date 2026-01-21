Name:           stg-tui
Version:        1.1.0
Release:        1.librepower.aix7.3
Summary:        Interactive TUI Dashboard for AIX Storage/LVM
License:        Apache-2.0
URL:            https://librepower.org

%description
stg-tui is a modern Terminal User Interface (TUI) dashboard for AIX storage.
Provides executive summary views with drill-down capability.

VIEWS (press number keys):
  [1] Dashboard  - Overview of VGs + Disks
  [2] VGs        - Volume Groups with capacity
  [3] Disks/PVs  - Physical volumes with LUN IDs, PVID, MPIO info
  [4] LVs        - Logical Volumes with sizes and mount points
  [5] Filesystems - All FS with usage percentages

KEY FEATURES:
  - LUN identification (PVID, unique_id, vendor/product)
  - Color-coded status and usage warnings
  - Single Go binary, no dependencies
  - Real-time refresh with 'r' key

This is a LibrePower original tool, created exclusively for AIX.

%install
mkdir -p %{buildroot}/opt/freeware/bin
cp %{_sourcedir}/stg-tui %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/stg-tui

%post
echo ""
echo "stg-tui 1.1.0 installed!"
echo ""
echo "Run 'stg-tui' and use number keys to navigate:"
echo "  1=Dashboard  2=VGs  3=Disks  4=LVs  5=FS  r=Refresh  q=Quit"
echo ""

%files
%attr(755, root, system) /opt/freeware/bin/stg-tui

%changelog
* Tue Jan 21 2025 LibrePower <hello@librepower.org> - 1.1.0-1
- Dashboard view with executive summary
- LUN identification (PVID, unique_id, MPIO vendor/product)
- Filesystem view with usage percentages
- Color-coded warnings (>80% yellow, >90% red)
- Multiple views with number key navigation

* Tue Jan 21 2025 LibrePower <hello@librepower.org> - 1.0.0-1
- Initial release
