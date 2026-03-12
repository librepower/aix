# Caddy 2.9.1 for AIX

> Modern web server with **automatic HTTPS**, reverse proxy, and load balancing. **5,300+ req/s** on POWER9.

[![LibrePower](https://img.shields.io/badge/LibrePower-POWER_Computing-blue)](https://librepower.org)
[![AIX](https://img.shields.io/badge/AIX-7.1+-green)](https://www.ibm.com/power/operating-systems/aix)
[![Caddy](https://img.shields.io/badge/Caddy-2.9.1-blue)](https://caddyserver.com)
[![Go](https://img.shields.io/badge/Go-1.24.11-00ADD8)](https://go.dev)
[![License](https://img.shields.io/badge/License-Apache_2.0-lightgrey)](https://www.apache.org/licenses/LICENSE-2.0)

![Caddy on AIX Demo](demo.gif)

---

## Highlights

- **Automatic HTTPS**: Zero-config TLS with Let's Encrypt and ZeroSSL
- **Reverse Proxy**: Load balancing, health checks, circuit breakers
- **File Server**: Static site hosting with directory listings
- **Zero Dependencies**: Single static binary, no runtime needed
- **All AIX Versions**: Works on AIX 7.1, 7.2, and 7.3

---

## Performance

| Metric | AIX 7.3 POWER9 | Ubuntu ppc64le POWER9 |
|--------|-----------------|----------------------|
| **Requests/s (c=50)** | 5,358 | 5,412 |
| **Requests/s (c=100)** | 5,307 | 5,389 |
| **p99 Latency** | 13-24ms | 12-22ms |

> Benchmarked with Apache Bench, 10K requests, static file serving

---

## Installation

### Option 1: DNF (Recommended)

```bash
# Add repository (one-time setup)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install
dnf install caddy
```

### Option 2: Direct RPM

```bash
curl -LO https://aix.librepower.org/packages/caddy-2.9.1-1.librepower.aix7.1.ppc.rpm
rpm -ivh caddy-2.9.1-1.librepower.aix7.1.ppc.rpm
```

---

## Quick Start

### File Server

```bash
# Create a web page
mkdir -p /opt/freeware/var/lib/caddy/www
echo "<h1>Hello from AIX POWER9!</h1>" > /opt/freeware/var/lib/caddy/www/index.html

# Create Caddyfile
cat > /opt/freeware/etc/caddy/Caddyfile << 'EOF'
:8080 {
    root * /opt/freeware/var/lib/caddy/www
    file_server
    log {
        output file /opt/freeware/var/log/caddy/access.log
    }
}
EOF

# Start Caddy
caddy run --config /opt/freeware/etc/caddy/Caddyfile --adapter caddyfile
```

### Reverse Proxy

```bash
cat > /opt/freeware/etc/caddy/Caddyfile << 'EOF'
:8443 {
    reverse_proxy localhost:8080 {
        health_uri /health
        health_interval 10s
    }
    log {
        output file /opt/freeware/var/log/caddy/access.log
    }
}
EOF

caddy run --config /opt/freeware/etc/caddy/Caddyfile --adapter caddyfile
```

### HTTPS (with domain)

```bash
cat > /opt/freeware/etc/caddy/Caddyfile << 'EOF'
your-domain.com {
    root * /opt/freeware/var/lib/caddy/www
    file_server
}
EOF

# Caddy automatically obtains and renews TLS certificates
caddy run --config /opt/freeware/etc/caddy/Caddyfile --adapter caddyfile
```

---

## Features on AIX

| Feature | Status | Notes |
|---------|--------|-------|
| **File server** | Working | Static files, directory listings |
| **Reverse proxy** | Working | Load balancing, health checks |
| **Automatic HTTPS** | Working | Let's Encrypt, ZeroSSL |
| **HTTP/2** | Working | Full support |
| **Logging** | Working | Structured JSON logs |
| **API** | Working | Admin API on localhost:2019 |
| **Caddyfile** | Working | Simplified config format |
| **JSON config** | Working | Full programmatic config |

---

## Paths

| Path | Description |
|------|-------------|
| `/opt/freeware/bin/caddy` | Binary |
| `/opt/freeware/etc/caddy/Caddyfile` | Configuration |
| `/opt/freeware/var/lib/caddy/www/` | Default web root |
| `/opt/freeware/var/log/caddy/` | Logs |

---

## AIX Port Notes

Caddy v2.9.1 compiled with Go 1.24.11 (`CGO_ENABLED=0`, static binary).

### Patch: File Locking (badger)

The embedded key-value store (`dgraph-io/badger` v1 and v2) uses `unix.Flock` for file locking, which is not available on AIX. Both versions were patched to use `syscall.FcntlFlock` (native AIX `fcntl` locking) instead.

**Files patched:**
- `vendor/github.com/dgraph-io/badger/dir_aix.go` — new AIX-specific implementation
- `vendor/github.com/dgraph-io/badger/dir_unix.go` — added `!aix` build tag
- `vendor/github.com/dgraph-io/badger/v2/dir_aix.go` — new AIX-specific implementation
- `vendor/github.com/dgraph-io/badger/v2/dir_unix.go` — added `!aix` build tag

Patch source files are available in `SOURCES/`.

---

## Build Information

| | Details |
|--|---------|
| **Version** | 2.9.1 |
| **Go** | 1.24.11 |
| **Linking** | Static (`CGO_ENABLED=0`) |
| **Architecture** | ppc64 (64-bit XCOFF) |
| **Binary size** | ~51 MB |
| **Dependencies** | None |

### Compatibility

| AIX 7.1 | AIX 7.2 | AIX 7.3 |
|---------|---------|---------|
| ✅      | ✅      | ✅      |

> Static binary — works on all AIX versions without additional libraries.

---

## Package Contents

```
caddy/
├── RPMS/
│   └── caddy-2.9.1-1.librepower.aix7.1.ppc.rpm
├── SPECS/
│   └── caddy.spec
├── SOURCES/
│   ├── dir_aix_v1.go          (badger v1 AIX patch)
│   ├── dir_aix_v2.go          (badger v2 AIX patch)
│   └── build-notes.txt        (compilation instructions)
├── demo.gif                   (terminal demo)
└── README.md
```

---

## License

- Caddy: Apache 2.0 (Matthew Holt / Caddy project)
- AIX patches and packaging: Apache 2.0 (LibrePower)

## Credits

- Caddy by [Matthew Holt](https://github.com/caddyserver/caddy)
- AIX port and packaging by [LibrePower](https://librepower.org)

## Links

- **Download**: [aix.librepower.org](https://aix.librepower.org)
- **Source**: [gitlab.com/librepower/aix](https://gitlab.com/librepower/aix)
- **Caddy docs**: [caddyserver.com/docs](https://caddyserver.com/docs)

---

<div align="center">

**Built by [LibrePower](https://librepower.org) -- Unlocking Power Systems**

[![Download](https://img.shields.io/badge/Download-aix.librepower.org-orange?style=for-the-badge)](https://aix.librepower.org)

</div>

---

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Beta live now!

---

*Part of [LibrePower](https://librepower.org) - Unlocking Power Systems through open source. Unmatched RAS and TCO. Minimal footprint 🌍*
