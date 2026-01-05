#!/bin/bash
#
# Build RPM for C-Sentinel on AIX
#

set -e

NAME="csentinel4aix"
VERSION="1.0.0"
RELEASE="1"

echo "================================================"
echo "Building ${NAME}-${VERSION}-${RELEASE} RPM"
echo "================================================"

# Create RPM build directories
echo "Creating RPM build directory structure..."
mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

# Create source tarball
echo "Creating source tarball..."
TMPDIR=$(mktemp -d)
SRCDIR="${TMPDIR}/${NAME}-${VERSION}"

# Copy source files
mkdir -p "${SRCDIR}"
cp -r src include Makefile.aix README.md README.AIX.md AIX_PORT_STATUS.md LICENSE "${SRCDIR}/"

# Create tarball
cd "${TMPDIR}"
tar czf ~/rpmbuild/SOURCES/${NAME}-${VERSION}.tar.gz ${NAME}-${VERSION}
cd -

# Clean up temp directory
rm -rf "${TMPDIR}"

# Copy spec file
echo "Copying spec file..."
cp csentinel4aix.spec ~/rpmbuild/SPECS/

# Build RPM
echo "Building RPM..."
cd ~/rpmbuild/SPECS

if command -v rpmbuild >/dev/null 2>&1; then
    rpmbuild -ba csentinel4aix.spec

    echo ""
    echo "================================================"
    echo "RPM build completed successfully!"
    echo "================================================"
    echo ""
    echo "RPMs location:"
    find ~/rpmbuild/RPMS -name "${NAME}*.rpm" -type f
    echo ""
    echo "SRPM location:"
    find ~/rpmbuild/SRPMS -name "${NAME}*.rpm" -type f
    echo ""
else
    echo "ERROR: rpmbuild not found. Please install rpm-build package:"
    echo "  dnf install rpm-build"
    exit 1
fi
