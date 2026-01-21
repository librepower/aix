Name:           stgtui
Version:        1.6.0
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
  - Single Go binary, no dependencies

Works with any storage: vSCSI, Fiber Channel, SAN arrays (EMC, IBM, NetApp, etc.)

This is a LibrePower original tool, created exclusively for AIX.

%install
mkdir -p %{buildroot}/opt/freeware/bin
cp %{_sourcedir}/stgtui %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/stgtui

%post
echo ""
echo "stgtui 1.6.0 installed - AIX Storage Explorer"
echo ""
echo "Run 'stgtui' - Navigation:"
echo "  1=Dash 2=VGs 3=Health 4=LVs 5=Disk->FS 6=FS->Disk r=Refresh q=Quit"
echo ""

%files
%attr(755, root, system) /opt/freeware/bin/stgtui

%changelog
* Tue Jan 21 2025 LibrePower <hello@librepower.org> - 1.6.0-1
- Compatible with both AIX native df and GNU df (linux-compat)
- Auto-detects df format from header (Available = GNU, Free = AIX)
- Fixed index out of range bug when parsing empty lines

* Tue Jan 21 2025 LibrePower <hello@librepower.org> - 1.5.0-1
- Fixed filesystem display on dashboard
- VG full is now shown as normal (cyan bar, no alerts)
- FS alerts threshold changed to 85%+ (was 80%)
- Refactored getFilesystems() for reliable df parsing
- Changed "VG Usage" to "PP Alloc" for clarity

* Tue Jan 21 2025 LibrePower <hello@librepower.org> - 1.4.0-1
- Added Health Check view with comprehensive storage diagnostics
- Stale PP detection (critical for mirrored environments)
- Multipath/lspath status monitoring (FC and vSCSI)
- Quorum status display per VG
- Paging space monitoring with alerts
- LV Status view showing sync state for all LVs
- Path status shown in disk detail view
- Version display in dashboard header

* Tue Jan 21 2025 LibrePower <hello@librepower.org> - 1.3.0-1
- Fixed df parsing for AIX (correct field index for %Used)
- Added VG Details view with per-PV usage breakdown
- Added Alerts view showing items over 80%/90%
- VG usage bars now show in dashboard and detail views
- Shows size/free in human-readable format (G/M)

* Tue Jan 21 2025 LibrePower <hello@librepower.org> - 1.2.0-1
- Renamed from stg-tui to stgtui
- Complete rewrite with bidirectional navigation
- ASCII logo banner on dashboard
- Visual progress bars with color coding
- Box-style hierarchy display (FS->LV->VG->PV->LUN)

* Tue Jan 21 2025 LibrePower <hello@librepower.org> - 1.1.0-1
- Dashboard view with executive summary
- LUN identification (PVID, unique_id, MPIO vendor/product)

* Tue Jan 21 2025 LibrePower <hello@librepower.org> - 1.0.0-1
- Initial release
