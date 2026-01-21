Name:           stgtui
Version:        1.1.0
Release:        1.librepower.aix7.3
Summary:        AIX Storage Explorer - Professional TUI for LVM/SAN Management
License:        Apache-2.0
URL:            https://librepower.org

%description
stgtui is a professional Terminal User Interface for AIX storage management.
Essential tool for sysadmins managing LVM, SAN, and multipath storage.

VIEWS (press number keys):
  [1] Dashboard   - System overview with VG/FS usage bars and health status
  [2] VG Details  - Volume Groups with PV breakdown, quorum status
  [3] Health Check- Comprehensive storage health: stale PPs, paths, capacity
  [4] LV Status   - All Logical Volumes with sync state
  [5] Disk -> FS  - Navigate from LUN/PV to VG to LV to Filesystem
  [6] FS -> Disk  - Navigate from Filesystem to LV to VG to PV/LUN
  [7] I/O Stats   - Real-time iostat metrics per disk
  [8] Mirror      - Mirror status for all LVs

KEY FEATURES:
  - Health monitoring: stale PPs, multipath status, quorum, capacity alerts
  - Unused disk detection: clean vs VGDA remnants
  - Multipath/SAN support: lspath status for FC and vSCSI
  - LUN identification: PVID, unique_id, MPIO vendor/product
  - Bidirectional navigation: FS<->LV<->VG<->PV<->LUN
  - Visual progress bars with color coding (green/yellow/red)
  - Search function [/] across VGs, LVs, FSs, disks
  - Export [e] current view to /tmp/stgtui-report-*.txt
  - Config file ~/.stgtuirc for custom thresholds
  - vim keys: j/k scroll, g/G top/bottom
  - NFS mounts included in filesystem views
  - Compatible with linux-compat (GNU coreutils)
  - Single Go binary, no dependencies

Works with any storage: vSCSI, Fiber Channel, SAN arrays (EMC, IBM, NetApp, etc.)

This is a LibrePower original tool, created exclusively for AIX.

%install
mkdir -p %{buildroot}/opt/freeware/bin
cp %{_sourcedir}/stgtui %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/stgtui

%post
echo ""
echo "  ____  _          _____        _"
echo " / ___|| |_ __ _  |_   _|_   _ (_)"
echo " \\___ \\| __/ _\` |   | | | | | || |"
echo "  ___) | || (_| |   | | | |_| || |"
echo " |____/ \\__\\__, |   |_|  \\__,_||_|"
echo "           |___/     v1.1.0"
echo ""
echo " AIX Storage Explorer installed!"
echo ""
echo " Run 'stgtui' - Keys:"
echo "   1-6=Views 7=I/O 8=Mirror /=Search e=Export r=Refresh q=Quit"
echo ""
echo " Config: ~/.stgtuirc (warn_threshold=85, crit_threshold=90)"
echo ""
echo " More AIX packages:  https://aix.librepower.org"
echo " LibrePower Project: https://librepower.org"
echo ""

%files
%attr(755, root, system) /opt/freeware/bin/stgtui

%changelog
* Wed Jan 22 2025 LibrePower <hello@librepower.org> - 1.1.0-1
- NEW: Search [/] - Find VGs, LVs, FSs, disks
- NEW: Export [e] - Save current view to file
- NEW: iostat view [7] - Real-time disk I/O metrics
- NEW: Mirror status [8] - Visualize LV mirrors (single/2-way/3-way)
- NEW: Unused disk detection with status indicators
  - ○ clean disk (ready for mkvg)
  - ◐ has VGDA remnants (needs chpv -C)
- NEW: NFS mounts included in filesystem views
- NEW: Config file ~/.stgtuirc for custom thresholds
- NEW: vim keys (j/k scroll, g/G top/bottom)
- Health Check now detects disks with VGDA remnants
- Improved disk list shows unused disks first

* Tue Jan 21 2025 LibrePower <hello@librepower.org> - 1.0.1-1
- Compatible with linux-compat (auto-detects AIX vs GNU df format)
- Health Check now includes errpt disk errors
- VG at 100% shown as normal (cyan bars, no alerts)
- FS alerts at 85%+ warning, 90%+ critical
- Improved menu spacing

* Tue Jan 21 2025 LibrePower <hello@librepower.org> - 1.0.0-1
- Initial release
- Dashboard with VG/FS usage bars and health status
- VG Details with PV breakdown and quorum status
- Health Check: stale PPs, multipath, paging, capacity
- LV Status with sync state
- Bidirectional mapping: FS<->LV<->VG<->PV<->LUN
- LUN identification (PVID, unique_id, MPIO vendor/product)
- Works with vSCSI, FC, any SAN array
