#!/bin/sh
# LibrePower AIX Repository Installer
# https://aix.librepower.org
#
# Usage: curl -fsSL https://aix.librepower.org/install.sh | sh

set -e

REPO_FILE="/opt/freeware/etc/yum.repos.d/librepower.repo"

echo ""
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║                                                                ║"
echo "║   LibrePower AIX Repository Installer                         ║"
echo "║                                                                ║"
echo "║   Unlocking Power Systems through open source                  ║"
echo "║                                                                ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

# Check if running as root
if [ "$(id -u)" -ne 0 ]; then
    echo "Error: This script requires root privileges."
    echo "Please run: sudo sh -c 'curl -fsSL https://aix.librepower.org/install.sh | sh'"
    exit 1
fi

# Check if AIX
if [ "$(uname -s)" != "AIX" ]; then
    echo "Error: This repository is for AIX systems only."
    echo "Detected OS: $(uname -s)"
    exit 1
fi

# Check if DNF is available
if ! command -v dnf >/dev/null 2>&1 && [ ! -x /opt/freeware/bin/dnf ]; then
    echo "Error: DNF is not installed."
    echo "Install it from AIX Toolbox: https://public.dhe.ibm.com/aix/freeSoftware/aixtoolbox/"
    exit 1
fi

# Create repo directory if needed
mkdir -p /opt/freeware/etc/yum.repos.d 2>/dev/null

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
echo "Done! Repository installed: ${REPO_FILE}"
echo ""
echo "Install packages with:"
echo "  dnf install fzf"
echo "  dnf install linux-compat"
echo "  dnf install nano"
echo ""
echo "List available:"
echo "  dnf --repo=librepower list available"
echo ""
echo "────────────────────────────────────────────────────────────────"
echo "  LibrePower - https://librepower.org"
echo "────────────────────────────────────────────────────────────────"
echo ""
