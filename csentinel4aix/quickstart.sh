#!/bin/bash
#
# C-Sentinel Quick Start Script
# 
# This script gets you from zero to first analysis in under 60 seconds.
# Works on Debian/Ubuntu systems.
#
# Usage:
#   curl -fsSL https://raw.githubusercontent.com/williamofai/c-sentinel/main/quickstart.sh | bash
#   
# Or:
#   chmod +x quickstart.sh
#   ./quickstart.sh

set -e

echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║          C-Sentinel Quick Start                               ║"
echo "║          Semantic Observability for UNIX Systems              ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if we're on Linux
if [[ "$(uname)" != "Linux" ]]; then
    echo -e "${RED}Error: C-Sentinel currently only supports Linux.${NC}"
    exit 1
fi

# Check for gcc
if ! command -v gcc &> /dev/null; then
    echo -e "${YELLOW}Installing build tools...${NC}"
    if command -v apt-get &> /dev/null; then
        sudo apt-get update && sudo apt-get install -y build-essential
    elif command -v yum &> /dev/null; then
        sudo yum groupinstall -y "Development Tools"
    else
        echo -e "${RED}Error: Please install gcc manually.${NC}"
        exit 1
    fi
fi

# Check for make
if ! command -v make &> /dev/null; then
    echo -e "${RED}Error: 'make' not found. Please install build-essential.${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Build tools available${NC}"

# Build the C components
echo ""
echo "Building C-Sentinel..."
make clean > /dev/null 2>&1 || true
make

echo ""
echo -e "${GREEN}✓ Build complete${NC}"

# Run quick analysis
echo ""
echo "═══════════════════════════════════════════════════════════════"
echo "Running first analysis..."
echo "═══════════════════════════════════════════════════════════════"
echo ""

./bin/sentinel --quick

echo ""
echo "═══════════════════════════════════════════════════════════════"
echo ""

# Show next steps
echo -e "${GREEN}Success!${NC} C-Sentinel is ready to use."
echo ""
echo "Next steps:"
echo ""
echo "  1. Full JSON fingerprint:"
echo "     ./bin/sentinel > my_system.json"
echo ""
echo "  2. Compare two systems (drift detection):"
echo "     ./bin/sentinel-diff system_a.json system_b.json"
echo ""
echo "  3. AI-powered analysis (requires API key or Ollama):"
echo ""
echo "     # With Anthropic Claude (best results):"
echo "     export ANTHROPIC_API_KEY='your-key'"
echo "     pip install anthropic"
echo "     ./sentinel_analyze.py"
echo ""
echo "     # With local Ollama (free, private):"
echo "     # Install Ollama first: https://ollama.com"
echo "     ollama pull llama3.2:3b"
echo "     pip install openai"
echo "     ./sentinel_analyze.py --local"
echo ""
echo "Documentation: https://github.com/williamofai/c-sentinel"
echo ""
