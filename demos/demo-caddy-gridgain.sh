#!/bin/bash
# =============================================================================
# LibrePower Demo: Caddy + GridGain on AIX POWER9
# =============================================================================
#
# This script demonstrates a complete API caching stack on AIX:
#   Caddy (reverse proxy) → GridGain (in-memory cache)
#
# Prerequisites:
#   dnf install caddy gridgain
#
# Usage:
#   chmod +x demo-caddy-gridgain.sh
#   ./demo-caddy-gridgain.sh
#
# What it does:
#   1. Starts GridGain (in-memory cache)
#   2. Configures Caddy as reverse proxy fronting GridGain's REST API
#   3. Populates cache with sample data via the Caddy proxy
#   4. Runs queries through the full stack
#   5. Benchmarks the stack (if 'ab' is available)
#   6. Cleans up
# =============================================================================

set -e

# Colors (if terminal supports them)
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

export PATH=/opt/freeware/bin:$PATH

# ---------------------------------------------------------------------------
header() { printf "\n${BOLD}${BLUE}━━━ %s ━━━${NC}\n\n" "$1"; }
step()   { printf "${GREEN}▶${NC} %s\n" "$1"; }
info()   { printf "${CYAN}  ℹ${NC} %s\n" "$1"; }
warn()   { printf "${YELLOW}  ⚠${NC} %s\n" "$1"; }
ok()     { printf "${GREEN}  ✔${NC} %s\n" "$1"; }
fail()   { printf "${RED}  ✖${NC} %s\n" "$1"; }

CADDY_PORT=8443
GRIDGAIN_REST=8080
GRIDGAIN_READY=0

# ---------------------------------------------------------------------------
cleanup() {
    header "Cleanup"
    step "Stopping Caddy..."
    caddy stop 2>/dev/null || true
    ok "Caddy stopped"

    step "Stopping GridGain..."
    gridgain stop 2>/dev/null || true
    ok "GridGain stopped"

    rm -f /tmp/demo-caddyfile 2>/dev/null
    printf "\n${BOLD}Demo complete.${NC}\n"
}
trap cleanup EXIT

# ---------------------------------------------------------------------------
wait_for_port() {
    local port=$1 max=$2 i=0
    while [ $i -lt $max ]; do
        if curl -s -o /dev/null -w '' "http://localhost:$port/" 2>/dev/null; then
            return 0
        fi
        sleep 2
        i=$((i + 1))
        printf "."
    done
    return 1
}

# ===========================================================================
header "LibrePower Demo: Caddy + GridGain on AIX"
printf "${BOLD}Stack:${NC} Caddy (reverse proxy) → GridGain (in-memory cache)\n"
printf "${BOLD}OS:${NC}    $(oslevel -s 2>/dev/null || echo 'AIX')\n"
printf "${BOLD}Arch:${NC}  $(uname -p)\n\n"

# ===========================================================================
header "Step 1: Start GridGain"
step "Starting GridGain in-memory cache..."
gridgain start

step "Waiting for GridGain REST API (port $GRIDGAIN_REST)..."
printf "  "
if wait_for_port $GRIDGAIN_REST 30; then
    printf "\n"
    ok "GridGain is ready"
    GRIDGAIN_READY=1
else
    printf "\n"
    fail "GridGain didn't start in time"
    warn "Check logs: /opt/freeware/var/log/gridgain/"
    exit 1
fi

# ===========================================================================
header "Step 2: Configure Caddy as Reverse Proxy"
step "Creating Caddyfile..."

cat > /tmp/demo-caddyfile << 'CADDYEOF'
:8443 {
    # Reverse proxy to GridGain REST API
    handle /ignite* {
        reverse_proxy localhost:8080
    }

    # Health endpoint
    handle /health {
        respond "OK" 200
    }

    # Welcome page
    handle {
        respond `
<!DOCTYPE html>
<html>
<head><title>LibrePower Demo</title>
<style>
  body { font-family: sans-serif; max-width: 700px; margin: 40px auto; padding: 20px; background: #0a0a0b; color: #e8e8e8; }
  h1 { color: #e9cb8e; } code { background: #1a1a1b; padding: 2px 8px; border-radius: 4px; }
  a { color: #68aec9; } .badge { display: inline-block; background: #1a3a1a; color: #81c784; padding: 4px 12px; border-radius: 12px; font-size: 0.85em; }
</style></head>
<body>
<h1>LibrePower Demo</h1>
<p><span class="badge">LIVE on AIX POWER9</span></p>
<p>This page is served by <strong>Caddy</strong>, proxying to <strong>GridGain</strong> in-memory cache.</p>
<h2>Try it</h2>
<ul>
  <li><a href="/ignite?cmd=version">GET /ignite?cmd=version</a> — GridGain version</li>
  <li><a href="/ignite?cmd=top&attr=false&mtr=false">GET /ignite?cmd=top</a> — Cluster topology</li>
  <li><a href="/health">GET /health</a> — Health check</li>
</ul>
<h2>API Examples</h2>
<pre>
# Create cache
curl "http://HOST:8443/ignite?cmd=getorcreate&cacheName=demo"

# Write data
curl "http://HOST:8443/ignite?cmd=put&cacheName=demo&key=city&val=Madrid"

# Read data
curl "http://HOST:8443/ignite?cmd=get&cacheName=demo&key=city"
</pre>
<p><em>Powered by <a href="https://librepower.org">LibrePower</a></em></p>
</body></html>` 200 {
            Content-Type text/html
        }
    }

    log {
        output file /opt/freeware/var/log/caddy/demo-access.log
    }
}
CADDYEOF

ok "Caddyfile created"

step "Starting Caddy on port $CADDY_PORT..."
caddy start --config /tmp/demo-caddyfile --adapter caddyfile

sleep 2
if curl -s -o /dev/null "http://localhost:$CADDY_PORT/health"; then
    ok "Caddy is ready"
else
    fail "Caddy didn't start"
    exit 1
fi

# ===========================================================================
header "Step 3: Populate Cache via Caddy Proxy"
PROXY="http://localhost:$CADDY_PORT"

step "Creating 'cities' cache..."
curl -s "$PROXY/ignite?cmd=getorcreate&cacheName=cities" | head -1
ok "Cache created"

step "Inserting data through Caddy → GridGain..."
cities=(
    "nyc:New York:8336817"
    "tok:Tokyo:13960000"
    "lon:London:8982000"
    "par:Paris:2161000"
    "mad:Madrid:3223000"
    "syd:Sydney:5312000"
    "sao:Sao Paulo:12330000"
    "bei:Beijing:21540000"
    "mum:Mumbai:20410000"
    "mex:Mexico City:21580000"
)

for city_data in "${cities[@]}"; do
    IFS=':' read -r key name pop <<< "$city_data"
    curl -s -o /dev/null "$PROXY/ignite?cmd=put&cacheName=cities&key=$key&val=$name ($pop)"
    printf "  ${GREEN}+${NC} %-12s → %s (%s)\n" "$key" "$name" "$pop"
done
ok "10 cities loaded into in-memory cache"

# ===========================================================================
header "Step 4: Query Cache via Caddy Proxy"

step "Reading data through the full stack..."
printf "\n"
for key in nyc tok mad bei; do
    result=$(curl -s "$PROXY/ignite?cmd=get&cacheName=cities&key=$key")
    value=$(echo "$result" | grep -o '"response":"[^"]*"' | cut -d'"' -f4)
    printf "  ${CYAN}GET${NC} %-6s → ${BOLD}%s${NC}\n" "$key" "$value"
done
printf "\n"

step "Checking GridGain version via Caddy..."
version=$(curl -s "$PROXY/ignite?cmd=version")
info "Response: $version"

step "Checking cluster topology via Caddy..."
topo=$(curl -s "$PROXY/ignite?cmd=top&attr=false&mtr=false" | head -c 200)
info "Topology: ${topo}..."

# ===========================================================================
header "Step 5: Benchmark"

step "Testing throughput: Caddy → GridGain cache reads..."
printf "\n"

if command -v ab >/dev/null 2>&1; then
    info "Running Apache Bench: 5,000 requests, concurrency 50"
    printf "\n"
    ab -n 5000 -c 50 -q "$PROXY/ignite?cmd=get&cacheName=cities&key=nyc" 2>&1 | \
        grep -E '(Requests per second|Time per request|Transfer rate|Complete|Failed)'
    printf "\n"

    info "Running Apache Bench: 10,000 requests, concurrency 100"
    printf "\n"
    ab -n 10000 -c 100 -q "$PROXY/ignite?cmd=get&cacheName=cities&key=tok" 2>&1 | \
        grep -E '(Requests per second|Time per request|Transfer rate|Complete|Failed)'
else
    warn "'ab' (Apache Bench) not found, using curl loop instead"
    start_time=$(date +%s)
    for i in $(seq 1 1000); do
        curl -s -o /dev/null "$PROXY/ignite?cmd=get&cacheName=cities&key=nyc"
    done
    end_time=$(date +%s)
    elapsed=$((end_time - start_time))
    if [ $elapsed -gt 0 ]; then
        rps=$((1000 / elapsed))
        info "1,000 requests in ${elapsed}s ≈ ${rps} req/s (single-threaded)"
    fi
fi

# ===========================================================================
header "Step 6: Welcome Page"

step "Caddy serves a web dashboard at http://localhost:$CADDY_PORT/"
info "Open in your browser or:"
info "  curl http://localhost:$CADDY_PORT/"
printf "\n"

# ===========================================================================
header "Summary"

printf "${BOLD}Stack running on AIX:${NC}\n\n"
printf "  ┌─────────────┐     ┌──────────────────┐\n"
printf "  │   ${CYAN}Caddy${NC}     │────▶│   ${YELLOW}GridGain${NC}       │\n"
printf "  │  :${CADDY_PORT}      │     │  :${GRIDGAIN_REST} REST     │\n"
printf "  │  rev proxy  │     │  in-memory cache │\n"
printf "  └─────────────┘     └──────────────────┘\n\n"
printf "  ${GREEN}▶${NC} Web UI:     http://localhost:$CADDY_PORT/\n"
printf "  ${GREEN}▶${NC} Cache API:  http://localhost:$CADDY_PORT/ignite?cmd=get&cacheName=cities&key=nyc\n"
printf "  ${GREEN}▶${NC} Health:     http://localhost:$CADDY_PORT/health\n"
printf "\n"
printf "${BOLD}Press Enter to stop the demo and clean up...${NC}"
read -r

# cleanup runs via trap
