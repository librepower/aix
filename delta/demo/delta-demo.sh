#!/bin/bash
#============================================================================
# delta Demo Script - Syntax-Highlighting Git Diff for AIX
#============================================================================

export PATH=/opt/freeware/bin:$PATH
export TERM=xterm-256color
export COLORTERM=truecolor
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
echo -e "\033[1;36m  delta - Beautiful Diffs for AIX\033[0m"
echo -e "\033[0;35m  First Rust app on AIX! - LibrePower.org\033[0m"
echo ""
sleep 2

# Create a Go file
cat > server.go << 'EOF'
package main

import (
    "fmt"
    "net/http"
)

func handler(w http.ResponseWriter, r *http.Request) {
    fmt.Fprintf(w, "Hello World")
}

func main() {
    http.HandleFunc("/", handler)
    http.ListenAndServe(":8080", nil)
}
EOF
git add server.go
git commit -q -m "Initial Go server"

# Modify with improvements
cat > server.go << 'EOF'
package main

import (
    "fmt"
    "log"
    "net/http"
    "os"
)

func handler(w http.ResponseWriter, r *http.Request) {
    name := r.URL.Query().Get("name")
    if name == "" {
        name = "World"
    }
    log.Printf("Request from %s for %s", r.RemoteAddr, name)
    fmt.Fprintf(w, "Hello, %s!", name)
}

func main() {
    port := os.Getenv("PORT")
    if port == "" {
        port = "8080"
    }
    log.Printf("Server starting on port %s", port)
    http.HandleFunc("/", handler)
    log.Fatal(http.ListenAndServe(":"+port, nil))
}
EOF

# Show diff with delta - Dracula theme
echo -e "\033[0;33m$ git diff | delta --syntax-theme='Dracula'\033[0m"
sleep 1
git diff | $DELTA --syntax-theme='Dracula'
sleep 3

# Commit and create Rust file
git add server.go
git commit -q -m "Add logging and config"

# Create Rust example
cat > lib.rs << 'EOF'
pub fn fibonacci(n: u32) -> u32 {
    if n <= 1 { return n; }
    fibonacci(n - 1) + fibonacci(n - 2)
}
EOF
git add lib.rs
git commit -q -m "Add fibonacci"

# Improve Rust code
cat > lib.rs << 'EOF'
use std::collections::HashMap;

pub struct Fibonacci {
    cache: HashMap<u64, u64>,
}

impl Fibonacci {
    pub fn new() -> Self {
        Fibonacci { cache: HashMap::new() }
    }

    pub fn calculate(&mut self, n: u64) -> u64 {
        if n <= 1 {
            return n;
        }
        if let Some(&result) = self.cache.get(&n) {
            return result;
        }
        let result = self.calculate(n - 1) + self.calculate(n - 2);
        self.cache.insert(n, result);
        result
    }
}
EOF

# Show Rust diff side by side with Nord
echo ""
echo -e "\033[0;33m$ git diff | delta --side-by-side --syntax-theme='Nord'\033[0m"
sleep 1
git diff | $DELTA --side-by-side --width=100 --syntax-theme='Nord'
sleep 3

# Cleanup
cd /
rm -rf $DEMO_DIR

# Footer
echo ""
echo -e "\033[1;32m════════════════════════════════════════════════════════\033[0m"
echo -e "\033[1;37m  20+ themes | 188 languages | Side-by-side view\033[0m"
echo -e "\033[1;37m  dnf install delta\033[0m"
echo -e "\033[0;36m  aix.librepower.org\033[0m"
echo -e "\033[1;32m════════════════════════════════════════════════════════\033[0m"
echo ""
sleep 2
