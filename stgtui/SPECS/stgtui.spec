Name:           stgtui
Version:        1.2.0
Release:        1.librepower.aix7.3
Summary:        AIX Storage Explorer - Interactive TUI for LVM Navigation
License:        Apache-2.0
URL:            https://librepower.org

%description
stgtui is a modern Terminal User Interface (TUI) for AIX storage exploration.
Provides bidirectional navigation between storage layers with visual feedback.

VIEWS (press number keys):
  [1] Dashboard  - Overview with ASCII progress bars for VGs and FSs
  [2] Disk->FS   - Navigate from LUN/PV to VG to LV to Filesystem
  [3] FS->Disk   - Navigate from Filesystem to LV to VG to PV/LUN

NAVIGATION:
  - Select a disk in view 2 to see all filesystems using it
  - Select a filesystem in view 3 to see all LUNs backing it
  - Full mapping: LUN -> PV -> VG -> LV -> FS (and reverse)

KEY FEATURES:
  - Bidirectional storage mapping (FS->LUN and LUN->FS)
  - LUN identification (PVID, unique_id, MPIO vendor/product)
  - ASCII progress bars with color coding (<80% green, 80-90% yellow, >90% red)
  - Visual box layout showing storage hierarchy
  - Single Go binary, no dependencies
  - Real-time refresh with 'r' key

This is a LibrePower original tool, created exclusively for AIX.

%install
mkdir -p %{buildroot}/opt/freeware/bin
cp %{_sourcedir}/stgtui %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/stgtui

%post
echo ""
echo "stgtui 1.2.0 installed!"
echo ""
echo "Run 'stgtui' and use number keys to navigate:"
echo "  1=Dashboard  2=Disk->FS  3=FS->Disk  r=Refresh  Esc=Back  q=Quit"
echo ""

%files
%attr(755, root, system) /opt/freeware/bin/stgtui

%changelog
* Tue Jan 21 2025 LibrePower <hello@librepower.org> - 1.2.0-1
- Renamed from stg-tui to stgtui
- Complete rewrite with bidirectional navigation
- ASCII logo banner on dashboard
- Visual progress bars with color coding
- Box-style hierarchy display (FS->LV->VG->PV->LUN)
- Reverse mapping (LUN->VG->LV->FS)
- VG layer properly included in all views

* Tue Jan 21 2025 LibrePower <hello@librepower.org> - 1.1.0-1
- Dashboard view with executive summary
- LUN identification (PVID, unique_id, MPIO vendor/product)
- Filesystem view with usage percentages
- Color-coded warnings (>80% yellow, >90% red)
- Multiple views with number key navigation

* Tue Jan 21 2025 LibrePower <hello@librepower.org> - 1.0.0-1
- Initial release
