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

## What It Is

podman-aix brings the familiar `podman` command-line interface to IBM AIX, using **System WPARs** (Workload Partitions) as the container runtime.

WPARs are AIX's native isolation technology — lightweight system partitions that share the host kernel, available since AIX 6.1 (2007). podman-aix wraps WPARs with a CLI that Linux administrators already know: `run`, `stop`, `exec`, `ps`, `logs`, `inspect`.

This is not a port of Podman or a Docker-compatible runtime. It is a purpose-built tool that maps container concepts to WPAR operations, giving AIX teams a modern workflow without leaving the platform.

---

## What It Is Not

Transparency matters. podman-aix is **not**:

- **OCI-compatible** — Images are `savewpar` snapshots, not OCI/Docker images. You cannot pull from Docker Hub or use Dockerfiles.
- **A drop-in Podman replacement** — The CLI is familiar, but the runtime is fundamentally different. Not all podman flags are implemented.
- **Portable across OS versions** — Images are WPAR snapshots tied to a specific AIX oslevel. An image built on AIX 7.3 TL04 works on AIX 7.3 TL04 systems. This is inherent to how WPARs work, not a limitation of podman-aix.

What it **is**: a way to manage isolated workloads on AIX with the ergonomics of modern container tooling, backed by technology IBM has shipped and supported for almost 20 years.

---

## Highlights

- **podman-compatible CLI**: `run`, `stop`, `rm`, `exec`, `ps`, `logs`, `inspect`
- **Image management**: `build`, `commit`, `push`, `pull`, `images`
- **Built-in registry**: HTTP registry with Bearer token auth
- **Automatic networking**: IP alias on host interface (.200-.253 range)
- **Fast cloning**: Images via `savewpar`/`restwpar` (~48s container create)
- **Multi-container demo**: Caddy + Go + MariaDB with blue/green deployment and live dashboard
- **Interactive mode**: `podman demo --interactive` — control the blue/green switch yourself

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
# Deploy a full-stack application (~6 minutes)
# Creates 4 containers: Caddy reverse proxy + Go app (blue/green) + MariaDB
podman demo

# Interactive mode — you control the blue/green switch
podman demo --interactive

# Open the live dashboard in your browser (URL shown during demo)
# Watch the deployment switch in real time

# Cleanup
podman demo --cleanup
```

The demo deploys a real microservices stack:

```
Client
  |
  v
[ demo-web ]         Caddy 2.9 reverse proxy (:80)
  |
  +----> [ demo-app-blue ]    Go app v1 (:8090)
  |
  +----> [ demo-app-green ]   Go app v2 (:8090)
            |
            v
      [ demo-db ]         MariaDB 11.8.5 (:3306)
```

Each box is an isolated System WPAR with its own IP address, filesystem, and process space.

---

## Architecture

### How It Works

```
podman run -d --name myapp aix73-minimal
       |
       v
  1. Find image: /var/lib/podman-aix/images/aix73-minimal.img
  2. Restore WPAR: restwpar -f image.img (fast clone, ~48s)
  3. Assign IP: ifconfig en0 alias 192.168.128.20x (auto-detected)
  4. Start WPAR: startwpar myapp
  5. Track state: /var/lib/podman-aix/state.json
       |
       v
  podman exec myapp hostname  -->  clogin wpar_name hostname
```

### Key Concepts

| Concept | podman-aix | Traditional Podman |
|---------|-----------|-------------------|
| **Container** | AIX System WPAR | OCI container (cgroups/namespaces) |
| **Image** | `savewpar` snapshot | OCI image (layers, registry) |
| **Image format** | AIX backup format | OCI image spec |
| **Registry** | Built-in HTTP + token auth | OCI Distribution spec |
| **Networking** | IP alias on host interface | CNI/netavark |
| **Isolation** | WPAR (shared kernel) | namespaces + cgroups |
| **Portability** | Same AIX oslevel | Any OCI runtime |

### Image Portability

WPAR images are snapshots of a running AIX system. They work across machines with the **same AIX version and TL/SP level**. This is not a bug — it's how WPARs are designed. If your fleet runs AIX 7.3 TL04, one image serves all machines.

For heterogeneous environments, build one image per oslevel and use the registry to distribute them.

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
| `podman demo [--cleanup] [--interactive]` | Run/clean multi-container demo |
| `podman version` | Show version info |

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
| ---      | Supported      | Supported      |

> Requires AIX 7.2+ with System WPAR support. The binary runs on all versions; WPAR operations require 7.2+.

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
