#!/bin/bash
#============================================================================
# lpsof Demo Script - List Open Files for AIX
#============================================================================
# Record: asciinema rec -c 'ssh aixlibrepower /root/demos/lpsof-demo.sh' lpsof.cast
# Convert: agg lpsof.cast lpsof.gif --cols 80 --rows 24 --speed 1.3
#============================================================================

LPSOF=/opt/freeware/bin/lpsof

clear
echo ""
echo -e "\033[1;36m  lpsof - List Open Files for AIX\033[0m"
echo -e "\033[0;34m  LibrePower.org\033[0m"
echo ""
sleep 2

# Basic usage
echo -e "\033[0;33m$ lpsof | head -20\033[0m"
sleep 1
$LPSOF 2>/dev/null | head -20
sleep 2

# Filter by user
echo ""
echo -e "\033[0;33m$ lpsof -u root | head -15\033[0m  \033[0;32m# Files by user\033[0m"
sleep 1
$LPSOF -u root 2>/dev/null | head -15
sleep 2

# Network connections
echo ""
echo -e "\033[0;33m$ lpsof -i :22\033[0m  \033[0;32m# SSH connections\033[0m"
sleep 1
$LPSOF -i :22 2>/dev/null
sleep 2

# Summary mode
echo ""
echo -e "\033[0;33m$ lpsof summary\033[0m  \033[0;32m# Top processes by FD count\033[0m"
sleep 1
$LPSOF summary 2>/dev/null | head -15
sleep 2

# Doctor mode
echo ""
echo -e "\033[0;33m$ lpsof doctor\033[0m  \033[0;32m# System diagnostics\033[0m"
sleep 1
$LPSOF doctor 2>/dev/null
sleep 3

# Footer
echo ""
echo -e "\033[1;32m════════════════════════════════════════════\033[0m"
echo -e "\033[1;37m  dnf install lpsof\033[0m"
echo -e "\033[0;36m  aix.librepower.org\033[0m"
echo -e "\033[1;32m════════════════════════════════════════════\033[0m"
echo ""
sleep 2
