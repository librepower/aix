#!/bin/bash
#============================================================================
# delta Demo Script - Syntax-Highlighting Git Diff for AIX
#============================================================================
# Record: asciinema rec -c 'ssh aixlibrepower /root/demos/delta-demo.sh' delta.cast
# Convert: agg delta.cast delta.gif --cols 100 --rows 30 --speed 1.3
#============================================================================

export PATH=/opt/freeware/bin:$PATH
DELTA=/opt/freeware/bin/delta

# Create temp git repo for demo
DEMO_DIR=/tmp/delta-demo-$$
mkdir -p $DEMO_DIR
cd $DEMO_DIR
git init -q
git config user.email "demo@example.com"
git config user.name "Demo"

clear
echo ""
echo -e "\033[1;36m  delta - Syntax-Highlighting Diff for AIX\033[0m"
echo -e "\033[0;34m  First Rust app ported to AIX! - LibrePower.org\033[0m"
echo ""
sleep 2

# Create initial file
cat > hello.py << 'EOF'
#!/usr/bin/env python3
def hello():
    print("Hello World")

if __name__ == "__main__":
    hello()
EOF
git add hello.py
git commit -q -m "Initial commit"

# Modify file
cat > hello.py << 'EOF'
#!/usr/bin/env python3
import sys

def hello(name="World"):
    """Greet someone by name."""
    print(f"Hello {name}!")
    return 0

if __name__ == "__main__":
    name = sys.argv[1] if len(sys.argv) > 1 else "World"
    sys.exit(hello(name))
EOF

# Show diff with delta
echo -e "\033[0;33m$ git diff | delta\033[0m"
sleep 1
git diff | $DELTA
sleep 3

# Side by side
echo ""
echo -e "\033[0;33m$ git diff | delta --side-by-side\033[0m"
sleep 1
git diff | $DELTA --side-by-side --width=100
sleep 3

# Show themes
echo ""
echo -e "\033[0;33m$ delta --list-themes | head -10\033[0m"
sleep 1
$DELTA --list-themes 2>/dev/null | head -10
sleep 2

# Cleanup
cd /
rm -rf $DEMO_DIR

# Footer
echo ""
echo -e "\033[1;32m════════════════════════════════════════════\033[0m"
echo -e "\033[1;37m  dnf install delta\033[0m"
echo -e "\033[0;36m  aix.librepower.org\033[0m"
echo -e "\033[1;32m════════════════════════════════════════════\033[0m"
echo ""
sleep 2
