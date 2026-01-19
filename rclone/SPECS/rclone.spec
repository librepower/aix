Name:           rclone
Version:        1.73.0
Release:        1.librepower
Summary:        Rsync for cloud storage - sync files to and from cloud providers
License:        MIT
Group:          Applications/Internet
URL:            https://rclone.org
Vendor:         LibrePower
Packager:       LibrePower <hello@librepower.org>

%description
Rclone is a command-line program to manage files on cloud storage.
It is a feature-rich alternative to cloud vendors' web storage interfaces.
Over 70 cloud storage products support rclone including S3 object stores,
business & consumer file storage services, as well as standard transfer protocols.

Features on AIX:
- Full sync, copy, move, check operations
- 70+ cloud provider support (S3, Google Drive, Dropbox, Azure, etc.)
- Bidirectional sync (bisync)
- Encryption and compression
- Serve files via HTTP, WebDAV, FTP, NFS
- Remote control daemon (rcd)

Note: mount (FUSE) and ncdu are not available on AIX.
Alternative: Use 'rclone serve nfs' to mount via NFS.

Compiled for AIX by LibrePower (https://librepower.org)

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/bin

cp /tmp/rclone-pkg/rclone %{buildroot}/opt/freeware/bin/
chmod 755 %{buildroot}/opt/freeware/bin/rclone

%files
%defattr(-,root,system,-)
/opt/freeware/bin/rclone

%post
echo "========================================================"
echo " rclone %{version} installed successfully!"
echo ""
echo " Quick start:"
echo "   rclone config              # Configure remotes"
echo "   rclone listremotes         # List configured remotes"
echo "   rclone ls remote:path      # List files"
echo "   rclone copy src remote:dst # Copy to cloud"
echo "   rclone sync src remote:dst # Sync to cloud"
echo ""
echo " Serve files locally:"
echo "   rclone serve http /path    # HTTP server"
echo "   rclone serve webdav /path  # WebDAV server"
echo "   rclone serve ftp /path     # FTP server"
echo "   rclone serve nfs /path     # NFS server (mount alternative)"
echo ""
echo " Documentation: https://rclone.org/docs/"
echo ""
echo " NOTE: mount (FUSE) is not available on AIX."
echo "       Use 'rclone serve nfs' as alternative."
echo ""
echo " LibrePower - https://librepower.org"
echo "========================================================"

%changelog
* Sun Jan 19 2026 LibrePower <hello@librepower.org> - 1.73.0-1.librepower
- Initial AIX port
- Built with Go 1.24.11 (static linking)
- All sync/copy/move operations working
- serve http/webdav/ftp/nfs working
- bisync (bidirectional sync) working
- mount/ncdu excluded (not supported on AIX)
