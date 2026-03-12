# podman-aix 0.5.0

> **Podman CLI. Native WPARs. POWER9/10/11 ready.**

[![LibrePower](https://img.shields.io/badge/LibrePower-POWER_Computing-blue)](https://librepower.org)
[![AIX](https://img.shields.io/badge/AIX-7.2+-green)](https://www.ibm.com/power/operating-systems/aix)
[![podman-aix](https://img.shields.io/badge/podman--aix-0.5.0-blue)](https://gitlab.com/librepower/aix/-/tree/main/podman-aix)
[![Go](https://img.shields.io/badge/Go-1.24.11-00ADD8)](https://go.dev)
[![License](https://img.shields.io/badge/License-GPL_3.0-lightgrey)](https://www.gnu.org/licenses/gpl-3.0.html)

![podman-aix Demo](demo.gif)

> *Built on WPARs — AIX native container technology since 2007. Now with the CLI you know.*

---

## Highlights

- **podman-compatible CLI**: `run`, `stop`, `rm`, `exec`, `ps`, `logs`, `inspect`
- **Image management**: `build`, `commit`, `push`, `pull`, `images`
- **Built-in registry**: HTTP registry with Bearer token auth
- **Automatic networking**: IP alias on host interface (.200-.253 range)
- **Fast cloning**: Layer-based images via `savewpar`/`restwpar` (~48s create)
- **Multi-container demo**: Full stack — Caddy + Go app + MariaDB with blue/green deployment

---

## Installation

### Option 1: DNF (Recommended)

```bash
# Add repository (one-time setup)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install
dnf install podman-aix
```

### Option 2: Direct RPM

```bash
curl -LO https://aix.librepower.org/packages/podman-aix-0.5.0-1.librepower.aix7.1.ppc.rpm
rpm -ivh podman-aix-0.5.0-1.librepower.aix7.1.ppc.rpm
```

---

## Quick Start

### Run a container

```bash
# Build the base image (one-time, ~10 minutes)
podman image build aix73-minimal

# Run a container
podman run -d --name myapp aix73-minimal

# Execute commands inside the container
podman exec myapp hostname
podman exec myapp uname -a
podman exec myapp oslevel -s

# List running containers
podman ps

# View logs
podman logs myapp

# Stop and remove
podman stop myapp
podman rm myapp
```

### Multi-container demo

```bash
# Deploy a full-stack application (Caddy + Go app + MariaDB)
podman demo

# Verify
podman ps
curl http://<web-ip>/health
curl http://<web-ip>/api/pets

# Blue/green zero-downtime switch
podman exec demo-web /tmp/switch-to-green.sh
podman exec demo-web /tmp/switch-to-blue.sh

# Cleanup
podman demo --cleanup
```

---

## Commands

| Command | Description |
|---------|-------------|
| `podman create [--name NAME] IMAGE` | Create a container |
| `podman start CONTAINER` | Start a stopped container |
| `podman stop CONTAINER` | Stop a running container |
| `podman rm [-f] CONTAINER` | Remove a container |
| `podman run [-d] [--name NAME] IMAGE [CMD]` | Create, start, and optionally exec |
| `podman exec CONTAINER CMD [ARGS...]` | Execute command in running container |
| `podman ps [-a]` | List containers |
| `podman logs CONTAINER` | Fetch container logs |
| `podman inspect CONTAINER` | Display container details (JSON) |
| `podman commit CONTAINER IMAGE` | Create image from container |
| `podman push IMAGE REGISTRY_URL` | Push image to registry |
| `podman pull IMAGE REGISTRY_URL` | Pull image from registry |
| `podman images` | List available images |
| `podman image build IMAGE` | Build savewpar image |
| `podman registry serve [OPTIONS]` | Start local image registry |
| `podman demo [--cleanup]` | Run/clean multi-container demo |
| `podman version` | Show version info |

---

## Features on AIX

| Feature | Status | Notes |
|---------|--------|-------|
| **Container lifecycle** | Working | create, start, stop, rm, run, exec |
| **Image management** | Working | build, commit, push, pull, images |
| **Image registry** | Working | HTTP with Bearer token auth |
| **Networking** | Working | Automatic IP alias on host interface |
| **Logs** | Working | Per-container log files |
| **Inspect** | Working | JSON container details |
| **Multi-container** | Working | `podman demo` — full stack |
| **Blue/green deploy** | Working | Zero-downtime traffic switch |

---

## How It Works

podman-aix uses AIX **System WPARs** (Workload Partitions) as the container runtime:

1. **Images** are stored as `savewpar` snapshots in `/var/lib/podman-aix/images/`
2. **Container creation** restores a WPAR from the saved image (`restwpar`)
3. **Networking** assigns IP aliases on the host interface (auto-detected)
4. **Exec** uses `clogin` to run commands inside WPARs
5. **State** is tracked in `/var/lib/podman-aix/state.json`

---

## Paths

| Path | Description |
|------|-------------|
| `/opt/freeware/bin/podman` | Binary |
| `/var/lib/podman-aix/` | State directory |
| `/var/lib/podman-aix/images/` | Saved WPAR images |
| `/var/lib/podman-aix/logs/` | Container logs |
| `/var/lib/podman-aix/state.json` | Container state |

---

## Build Information

| | Details |
|--|---------|
| **Version** | 0.5.0 |
| **Go** | 1.24.11 |
| **Linking** | Static (`CGO_ENABLED=0`) |
| **Architecture** | ppc64 (64-bit XCOFF) |
| **Dependencies** | None (single binary) |

### Compatibility

| AIX 7.1 | AIX 7.2 | AIX 7.3 |
|---------|---------|---------|
| ❌      | ✅      | ✅      |

> Requires AIX 7.2+ — System WPARs with the features used by podman-aix are available from AIX 7.2 onwards.

---

## Package Contents

```
podman-aix/
├── RPMS/
│   └── podman-aix-0.5.0-1.librepower.aix7.1.ppc.rpm
├── SPECS/
│   └── podman-aix.spec
├── demo.gif                   (terminal demo)
└── README.md
```

---

## License

- podman-aix: GPL-3.0 (LibrePower)

## Credits

- podman-aix by [LibrePower](https://librepower.org)
- Inspired by [Podman](https://github.com/containers/podman) (Red Hat)
- Built on [AIX System WPARs](https://www.ibm.com/docs/en/aix/7.3?topic=partitions-workload) (IBM)

## Links

- **Download**: [aix.librepower.org](https://aix.librepower.org)
- **Source**: [gitlab.com/librepower/aix](https://gitlab.com/librepower/aix)

---

<div align="center">

**Built by [LibrePower](https://librepower.org) — Unlocking Power Systems**

[![Download](https://img.shields.io/badge/Download-aix.librepower.org-orange?style=for-the-badge)](https://aix.librepower.org)

</div>

---

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Beta live now!

---

*Part of [LibrePower](https://librepower.org) - Unlocking Power Systems through open source. Unmatched RAS and TCO. Minimal footprint 🌍*
