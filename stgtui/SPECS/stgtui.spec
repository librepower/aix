Name:           stgtui
Version:        1.0.1
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

KEY FEATURES:
  - Health monitoring: stale PPs, multipath status, quorum, capacity alerts
  - Multipath/SAN support: lspath status for FC and vSCSI
  - LUN identification: PVID, unique_id, MPIO vendor/product
  - Bidirectional navigation: FS<->LV<->VG<->PV<->LUN
  - Visual progress bars with color coding (green/yellow/red)
  - Paging space monitoring
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
echo "           |___/     v1.0.1"
echo ""
echo " AIX Storage Explorer installed!"
echo ""
echo " Run 'stgtui' - Keys:"
echo "   1=Dash 2=VGs 3=Health 4=LVs 5=Disk->FS 6=FS->Disk r=Refresh q=Quit"
echo ""
echo " More AIX packages:  https://aix.librepower.org"
echo " LibrePower Project: https://librepower.org"
echo ""

%files
%attr(755, root, system) /opt/freeware/bin/stgtui

%changelog
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
