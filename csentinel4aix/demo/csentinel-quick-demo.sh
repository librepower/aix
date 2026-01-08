#!/bin/bash
#============================================================================
# C-Sentinel 15-Second Demo - Maximum Impact for Social Media
#============================================================================
# Record: asciinema rec -c './csentinel-quick-demo.sh' quick.cast
# Convert: agg quick.cast quick.gif --speed 1 --cols 80 --rows 20
#============================================================================

SENTINEL=/opt/freeware/bin/sentinel
DEMO_CONF=/tmp/demo-app.conf

# Cleanup from previous runs
rm -f $DEMO_CONF ~/.sentinel/baseline.dat 2>/dev/null

clear
echo ""
echo -e "\033[1;36m  C-Sentinel - AIX System Monitoring\033[0m"
echo ""
sleep 1

# Show system state
echo -e "\033[0;33m$ sentinel -q -n\033[0m"
sleep 1
$SENTINEL -q -n 2>&1 | head -20
sleep 2

# Create initial config
echo ""
echo -e "\033[0;32m[+] Creating app config: $DEMO_CONF\033[0m"
cat > $DEMO_CONF << 'EOF'
# Application Config
db_host=localhost
db_port=5432
ssl=true
api_key=SECRET-12345
EOF
sleep 1

# Learn baseline with config
echo ""
echo -e "\033[0;33m$ sentinel -l -n $DEMO_CONF\033[0m  \033[0;32m# Learn baseline\033[0m"
sleep 1
$SENTINEL -l -n $DEMO_CONF 2>&1 | tail -15
sleep 2

# Simulate attack - modify config
echo ""
echo -e "\033[0;31m[!] ATTACKER modifying config...\033[0m"
sleep 1
cat > $DEMO_CONF << 'EOF'
# Application Config
db_host=evil-server.com
db_port=31337
ssl=false
api_key=STOLEN-XXXXX
EOF
echo -e "\033[0;31m    db_host: localhost -> evil-server.com\033[0m"
echo -e "\033[0;31m    ssl: true -> false\033[0m"
sleep 2

# Detect drift
echo ""
echo -e "\033[0;33m$ sentinel -b -q -n $DEMO_CONF\033[0m"
sleep 1
$SENTINEL -b -q -n $DEMO_CONF 2>&1 | tail -15
sleep 3

# Footer
echo ""
echo -e "\033[1;32m════════════════════════════════════════════\033[0m"
echo -e "\033[1;37m  dnf install csentinel4aix\033[0m"
echo -e "\033[0;36m  aix.librepower.org\033[0m"
echo -e "\033[1;32m════════════════════════════════════════════\033[0m"
echo ""
sleep 2

rm -f $DEMO_CONF
