#!/bin/sh
# LibrePower AIX Repository Installer
# https://aix.librepower.org
#
# Usage: curl -fsSL https://aix.librepower.org/install.sh | sh
#
# Part of LibrePower initiative by SIXE - IBM Business Partner
# https://sixe.eu | https://librepower.org

set -e

REPO_FILE="/opt/freeware/etc/yum/repos.d/librepower.repo"
REPO_URL="https://aix.librepower.org"

echo ""
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║                                                                ║"
echo "║   LibrePower AIX Repository Installer                         ║"
echo "║                                                                ║"
echo "║   🌍 Unlocking Power Systems through open source               ║"
echo "║                                                                ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

# Check if running as root
if [ "$(id -u)" -ne 0 ]; then
    echo "Warning: This script requires root privileges."
    echo "Please run: sudo sh -c 'curl -fsSL ${REPO_URL}/install.sh | sh'"
    exit 1
fi

# Check if AIX
if [ "$(uname -s)" != "AIX" ]; then
    echo "Warning: This repository is for AIX systems only."
    echo "Detected OS: $(uname -s)"
    exit 1
fi

# Check if DNF is installed
if ! command -v dnf >/dev/null 2>&1; then
    echo "Warning: DNF is not installed."
    echo "Please install DNF from AIX Toolbox first:"
    echo "  https://public.dhe.ibm.com/aix/freeSoftware/aixtoolbox/"
    exit 1
fi

# Check if repo directory exists
if [ ! -d "/opt/freeware/etc/yum/repos.d" ]; then
    echo "Creating repos.d directory..."
    mkdir -p /opt/freeware/etc/yum/repos.d
fi

# Create repository file
echo "Installing LibrePower repository..."
cat > "${REPO_FILE}" << 'REPOEOF'
[librepower]
name=LibrePower AIX Toolbox
baseurl=https://aix.librepower.org/packages/
enabled=1
gpgcheck=0
REPOEOF

echo ""
echo "✓ LibrePower repository installed successfully!"
echo ""
echo "Repository file: ${REPO_FILE}"
echo ""
echo "You can now install packages with:"
echo "  dnf install fzf"
echo "  dnf install linux-compat"
echo ""
echo "To list available packages:"
echo "  dnf --repo=librepower list available"
echo ""
echo "────────────────────────────────────────────────────────────────"
echo "  LibrePower - SIXE IBM Business Partner"
echo "  https://sixe.eu | https://librepower.org"
echo "────────────────────────────────────────────────────────────────"
echo ""
