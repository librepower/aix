#!/bin/bash
#============================================================================
# 2FA Demo Script - Google Authenticator for AIX
#============================================================================
# Record: asciinema rec -c 'ssh aixlibrepower /root/demos/2fa-demo.sh' 2fa.cast
# Convert: agg 2fa.cast 2fa.gif --cols 80 --rows 30 --speed 1.2
#============================================================================

clear
echo ""
echo -e "\033[1;36m  2FA Made Simple - Google Authenticator for AIX\033[0m"
echo -e "\033[0;34m  LibrePower.org\033[0m"
echo ""
sleep 2

# Show what's installed
echo -e "\033[0;33m$ rpm -qa | grep -i auth\033[0m"
sleep 1
rpm -qa 2>/dev/null | grep -i auth || echo "google-authenticator-1.09-1.librepower"
sleep 2

# Show the setup wizard options
echo ""
echo -e "\033[0;33m$ google-authenticator --help\033[0m"
sleep 1
/opt/freeware/bin/google-authenticator --help 2>&1 | head -20
sleep 2

# Show QR code generation (non-interactive demo)
echo ""
echo -e "\033[1;33m--- Generating QR Code (demo) ---\033[0m"
echo ""
sleep 1

# Generate a demo secret and QR code
echo -e "\033[0;32mYour new secret key is: JBSWY3DPEHPK3PXP\033[0m"
echo ""
sleep 1

# Show a text representation of what QR looks like
echo "Scan this QR code with Google Authenticator:"
echo ""
cat << 'QR'
    █▀▀▀▀▀█ ▄▄▄▄▄ █▀▀▀▀▀█
    █ ███ █ █▄▀▄█ █ ███ █
    █ ▀▀▀ █ ▀█▀▀▄ █ ▀▀▀ █
    ▀▀▀▀▀▀▀ █▄▀▄█ ▀▀▀▀▀▀▀
    ▀▀██▀▀▀▄▄ ▀▄▀▄█▄▀█▄▀▄
    █ ▀▄▀█▀▄▄▀▄▀▄ ▀█▀ ▀▄█
    ▀▀▀▀▀▀▀ █▄▀▄█▀▄█▄▀▄██
    █▀▀▀▀▀█ ▀█▀▄▀ ▄▀█▀▄ █
    █ ███ █ ▄▀▄█▀▄█▄ ▀▄▀█
    █ ▀▀▀ █ █▀▄▀▄▀▄█▄▀▄██
    ▀▀▀▀▀▀▀ ▀▀▀▀▀▀▀▀▀▀▀▀▀
QR
echo ""
sleep 3

# Show scratch codes
echo -e "\033[0;33mEmergency scratch codes:\033[0m"
echo "  12345678"
echo "  87654321"
echo "  11223344"
echo "  44332211"
echo "  99887766"
echo ""
sleep 2

# Show PAM config
echo -e "\033[0;33m$ cat /etc/pam.d/sshd | grep auth\033[0m"
sleep 1
echo "auth required pam_google_authenticator.so nullok"
echo "auth required pam_unix.so"
sleep 2

# Footer
echo ""
echo -e "\033[1;32m════════════════════════════════════════════════════\033[0m"
echo -e "\033[1;37m  dnf install google-authenticator-libpam\033[0m"
echo -e "\033[0;36m  Full setup: google-authenticator\033[0m"
echo -e "\033[0;36m  aix.librepower.org\033[0m"
echo -e "\033[1;32m════════════════════════════════════════════════════\033[0m"
echo ""
sleep 2
