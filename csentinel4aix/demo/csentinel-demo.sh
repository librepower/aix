#!/bin/bash
#============================================================================
# C-Sentinel Demo Script for AIX
# Showcases semantic observability features for LinkedIn recording
#============================================================================
# Usage: ./csentinel-demo.sh
# For GIF: Use asciinema or ttygif to record
#============================================================================

# Sentinel path
SENTINEL=/opt/freeware/bin/sentinel

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
NC='\033[0m' # No Color

# Demo config
DEMO_FILE="/tmp/demo-app.conf"
DEMO_PAUSE=${DEMO_PAUSE:-2}  # Seconds between steps

#----------------------------------------------------------------------------
# Helper functions
#----------------------------------------------------------------------------
banner() {
    echo ""
    echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
    echo -e "${WHITE}  $1${NC}"
    echo -e "${CYAN}═══════════════════════════════════════════════════════════════${NC}"
    echo ""
}

step() {
    echo -e "${YELLOW}▶ $1${NC}"
    sleep 1
}

info() {
    echo -e "${BLUE}  ℹ $1${NC}"
}

success() {
    echo -e "${GREEN}  ✓ $1${NC}"
}

warning() {
    echo -e "${RED}  ⚠ $1${NC}"
}

pause() {
    sleep ${DEMO_PAUSE}
}

type_command() {
    echo ""
    echo -e "${WHITE}\$ $1${NC}"
    sleep 1
}

run_sentinel() {
    type_command "$1"
    # Replace 'sentinel' with actual path for execution
    local cmd=$(echo "$1" | sed "s|^sentinel |$SENTINEL |")
    eval "$cmd"
    pause
}

#----------------------------------------------------------------------------
# Demo Introduction
#----------------------------------------------------------------------------
clear
echo ""
echo -e "${CYAN}"
cat << 'LOGO'
   ____      ____             _   _            _
  / ___|    / ___|  ___ _ __ | |_(_)_ __   ___| |
 | |   ____\___ \ / _ \ '_ \| __| | '_ \ / _ \ |
 | |__|_____|__) |  __/ | | | |_| | | | |  __/ |
  \____|   |____/ \___|_| |_|\__|_|_| |_|\___|_|

LOGO
echo -e "${NC}"
echo -e "${WHITE}  Semantic Observability for AIX Systems${NC}"
echo -e "${BLUE}  LibrePower - Unlocking Power Systems through Open Source${NC}"
echo ""
sleep 3

#----------------------------------------------------------------------------
# Part 1: Quick System Analysis
#----------------------------------------------------------------------------
banner "1. QUICK SYSTEM ANALYSIS"

step "Let's see the current state of this AIX system..."
run_sentinel "sentinel -q"

info "C-Sentinel shows: processes, memory, load, and potential issues"
info "Exit code indicates health: 0=OK, 1=Warning, 2=Critical"
pause

#----------------------------------------------------------------------------
# Part 2: Network Monitoring with PID Attribution
#----------------------------------------------------------------------------
banner "2. NETWORK MONITORING"

step "Now let's add network analysis with PID detection..."
run_sentinel "sentinel -q -n"

info "Notice: Each listener shows the process name!"
info "70+ well-known ports mapped to their services"
info "Unusual ports flagged for investigation"
pause

#----------------------------------------------------------------------------
# Part 3: Baseline Learning
#----------------------------------------------------------------------------
banner "3. BASELINE LEARNING"

step "First, let's learn what 'normal' looks like..."
run_sentinel "sentinel -l -n"

success "Baseline captured! C-Sentinel now knows the normal state."
info "Future probes will detect deviations automatically"
pause

#----------------------------------------------------------------------------
# Part 4: Create Demo Config File
#----------------------------------------------------------------------------
banner "4. MONITORING CONFIG FILES"

step "Creating a demo application config..."
type_command "cat > ${DEMO_FILE}"
cat > ${DEMO_FILE} << 'CONF'
# Demo Application Configuration
database_host = localhost
database_port = 5432
max_connections = 100
ssl_enabled = true
api_key = DEMO-KEY-12345
CONF
echo -e "${GREEN}[config created]${NC}"
pause

step "Let's verify C-Sentinel detects the config..."
run_sentinel "sentinel -j ${DEMO_FILE} 2>/dev/null | grep -A5 '\"config_files\"'"

success "Config file tracked with SHA256 checksum!"
pause

#----------------------------------------------------------------------------
# Part 5: Simulate Attack - Modify Config
#----------------------------------------------------------------------------
banner "5. DETECTING CONFIG DRIFT"

step "Simulating an attacker modifying the config..."
echo ""
echo -e "${RED}  [ATTACKER]${NC} Changing database port and disabling SSL..."
sleep 1

# Modify the config (simulating attack)
cat > ${DEMO_FILE} << 'CONF'
# Demo Application Configuration
database_host = evil-server.example.com
database_port = 31337
max_connections = 100
ssl_enabled = false
api_key = STOLEN-KEY-XXXXX
CONF

warning "Config modified by 'attacker'!"
pause

step "Now let's compare against our baseline..."
run_sentinel "sentinel -b -q"

echo ""
echo -e "${RED}  ══════════════════════════════════════════════════════════${NC}"
echo -e "${RED}  DRIFT DETECTED! Config checksum changed from baseline!${NC}"
echo -e "${RED}  ══════════════════════════════════════════════════════════${NC}"
pause

#----------------------------------------------------------------------------
# Part 6: Service Change Detection
#----------------------------------------------------------------------------
banner "6. SERVICE MONITORING"

step "Checking current network listeners..."
run_sentinel "sentinel -q -n 2>&1 | grep -A20 'Listeners:' | head -15"

info "If a service starts or stops, baseline comparison will detect it"
info "New listeners = potential backdoor"
info "Missing listeners = potential DoS or misconfiguration"
pause

#----------------------------------------------------------------------------
# Part 7: JSON Output for AI Analysis
#----------------------------------------------------------------------------
banner "7. JSON OUTPUT FOR AI/LLM ANALYSIS"

step "Generate JSON fingerprint for semantic analysis..."
run_sentinel "sentinel -j -n 2>/dev/null | head -30"

echo -e "${BLUE}  ... (truncated for demo)${NC}"
echo ""
info "Full JSON includes: system info, all processes, network state"
info "Feed this to Claude, GPT, or your favorite LLM for analysis!"
pause

#----------------------------------------------------------------------------
# Part 8: Continuous Monitoring
#----------------------------------------------------------------------------
banner "8. CONTINUOUS MONITORING (Watch Mode)"

step "For production, use watch mode with baseline comparison..."
echo ""
echo -e "${WHITE}\$ sentinel -w -i 300 -b -n${NC}"
echo ""
info "This would probe every 5 minutes and alert on drift"
info "Integrate with cron for scheduled monitoring:"
echo ""
echo -e "${CYAN}  # Crontab example:${NC}"
echo -e "${WHITE}  */5 * * * * /opt/freeware/bin/sentinel -b -q -n >> /var/log/sentinel.log 2>&1${NC}"
pause

#----------------------------------------------------------------------------
# Cleanup
#----------------------------------------------------------------------------
rm -f ${DEMO_FILE} 2>/dev/null

#----------------------------------------------------------------------------
# Summary
#----------------------------------------------------------------------------
banner "DEMO COMPLETE"

echo -e "${GREEN}"
cat << 'SUMMARY'
  C-Sentinel provides:

    [1] Quick system health checks (-q)
    [2] Network monitoring with PID attribution (-n)
    [3] Baseline learning for drift detection (-l, -b)
    [4] Config file integrity monitoring (SHA256)
    [5] JSON output for AI/LLM analysis (-j)
    [6] Continuous watch mode (-w)

  Perfect for:
    - Security monitoring
    - Compliance auditing
    - Incident response
    - Change detection
    - AI-assisted troubleshooting
SUMMARY
echo -e "${NC}"

echo ""
echo -e "${CYAN}  Install: dnf install csentinel4aix${NC}"
echo -e "${CYAN}  Repo:    https://aix.librepower.org${NC}"
echo -e "${CYAN}  Source:  https://gitlab.com/librepower/aix${NC}"
echo ""
echo -e "${WHITE}  LibrePower - Open Source for IBM Power Systems${NC}"
echo ""
