#!/bin/bash
#============================================================================
# C-Sentinel GIF Demo - Optimized for short, impactful recording
#============================================================================
# Best recorded with: asciinema rec demo.cast
# Convert to GIF with: agg demo.cast demo.gif --cols 80 --rows 24
#============================================================================

SENTINEL=/opt/freeware/bin/sentinel

run() {
    echo -e "\033[0;33m\$ $1\033[0m"
    sleep 1
    eval "$1"
    sleep 2
}

section() {
    echo ""
    echo -e "\033[1;36m--- $1 ---\033[0m"
    echo ""
    sleep 1
}

clear
echo ""
echo -e "\033[1;37m   C-Sentinel for AIX - System Observability Demo\033[0m"
echo -e "\033[0;34m   LibrePower.org\033[0m"
sleep 2

section "1. Quick System Analysis"
run "$SENTINEL -q"

section "2. Network Monitoring (with PID detection)"
run "$SENTINEL -q -n"

section "3. Learn Normal State (Baseline)"
run "$SENTINEL -l -n"
echo -e "\033[0;32m   ✓ Baseline saved\033[0m"
sleep 1

section "4. Simulate Config Tampering"
echo -e "\033[0;31m   [!] Attacker modifying /tmp/app.conf...\033[0m"
echo "api_key=STOLEN" > /tmp/app.conf
sleep 2

section "5. Detect Drift!"
run "$SENTINEL -b -q"
echo ""
echo -e "\033[1;31m   ⚠ DRIFT DETECTED - Config changed!\033[0m"
sleep 2

section "6. JSON for AI Analysis"
echo -e "\033[0;33m\$ sentinel -j -n | head -20\033[0m"
sleep 1
$SENTINEL -j -n 2>/dev/null | head -20
sleep 2

echo ""
echo -e "\033[1;32m================================================\033[0m"
echo -e "\033[1;37m   dnf install csentinel4aix\033[0m"
echo -e "\033[0;36m   https://aix.librepower.org\033[0m"
echo -e "\033[1;32m================================================\033[0m"
echo ""
sleep 3

rm -f /tmp/app.conf
