#!/bin/bash
#============================================================================
# nano Demo Script - GNU Text Editor for AIX
#============================================================================

export PATH=/opt/freeware/bin:$PATH
export TERM=xterm-256color

clear
echo ""
echo -e "\033[1;36m  nano - Simple Text Editor for AIX\033[0m"
echo -e "\033[0;34m  The friendly editor - LibrePower.org\033[0m"
echo ""
sleep 2

# Show version
echo -e "\033[0;33m$ nano --version\033[0m"
sleep 1
nano --version 2>/dev/null | head -3
sleep 2

# Show syntax highlighting support
echo ""
echo -e "\033[0;33m$ ls /opt/freeware/share/nano/*.nanorc | wc -l\033[0m"
sleep 1
count=$(ls /opt/freeware/share/nano/*.nanorc 2>/dev/null | wc -l)
echo "$count syntax definitions available!"
sleep 1

echo ""
echo -e "\033[0;33m$ ls /opt/freeware/share/nano/\033[0m  \033[0;32m# Supported languages\033[0m"
sleep 1
ls /opt/freeware/share/nano/*.nanorc 2>/dev/null | xargs -n1 basename | sed 's/.nanorc//' | head -20 | column
sleep 2

# Show what nano looks like (simulated)
echo ""
echo -e "\033[0;33m$ nano server.py\033[0m"
sleep 1
echo ""
echo -e "\033[7m  GNU nano 8.3              server.py                          \033[0m"
echo ""
echo -e "\033[38;5;202m#!/usr/bin/env python3\033[0m"
echo -e "\033[38;5;244m\"\"\"Simple HTTP server.\"\"\"\033[0m"
echo ""
echo -e "\033[38;5;33mimport\033[0m \033[38;5;214mhttp.server\033[0m"
echo -e "\033[38;5;33mimport\033[0m \033[38;5;214msocketserver\033[0m"
echo ""
echo -e "\033[38;5;33mPORT\033[0m = \033[38;5;141m8080\033[0m"
echo ""
echo -e "\033[38;5;33mclass\033[0m \033[38;5;78mHandler\033[0m(\033[38;5;214mhttp.server.SimpleHTTPRequestHandler\033[0m):"
echo -e "    \033[38;5;33mdef\033[0m \033[38;5;78mdo_GET\033[0m(\033[38;5;208mself\033[0m):"
echo -e "        \033[38;5;208mself\033[0m.send_response(\033[38;5;141m200\033[0m)"
echo -e "        \033[38;5;208mself\033[0m.end_headers()"
echo -e "        \033[38;5;208mself\033[0m.wfile.write(\033[38;5;186mb\"Hello AIX!\"\033[0m)"
echo ""
echo -e "\033[7m^G Help  ^O Write Out  ^W Where Is  ^K Cut    ^T Execute  ^C Location  \033[0m"
echo -e "\033[7m^X Exit  ^R Read File  ^\\ Replace   ^U Paste  ^J Justify  ^/ Go To Line\033[0m"
sleep 3

# Key features
echo ""
echo -e "\033[1;33mWhy nano on AIX?\033[0m"
echo -e "\033[0;32m  ✓ No learning curve (unlike vi/vim)\033[0m"
echo -e "\033[0;32m  ✓ On-screen shortcuts always visible\033[0m"
echo -e "\033[0;32m  ✓ Syntax highlighting for 40+ languages\033[0m"
echo -e "\033[0;32m  ✓ UTF-8 support\033[0m"
echo -e "\033[0;32m  ✓ Default editor on most Linux distros\033[0m"
sleep 2

# Footer
echo ""
echo -e "\033[1;32m════════════════════════════════════════════════════════\033[0m"
echo -e "\033[1;37m  dnf install nano\033[0m"
echo -e "\033[0;36m  aix.librepower.org\033[0m"
echo -e "\033[1;32m════════════════════════════════════════════════════════\033[0m"
echo ""
sleep 2
