# C-Sentinel for AIX

**AIX Port of C-Sentinel - Semantic Observability for UNIX Systems**

> 🔗 **Original Project:** [github.com/williamofai/c-sentinel](https://github.com/williamofai/c-sentinel)
> 👤 **Original Author:** William ([@williamofai](https://github.com/williamofai))
> 📦 **This Repository:** AIX 7.1/7.2/7.3 port by [LibrePower](https://librepower.org)

This is a port of the excellent C-Sentinel system monitoring tool to IBM AIX. All core functionality and design credit goes to the original author. We are only responsible for the AIX-specific adaptations and maintaining compatibility with AIX 7.x systems.

**About C-Sentinel:** A lightweight, portable system prober written in C that captures "system fingerprints" for AI-assisted analysis of non-obvious risks.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
![Version](https://img.shields.io/badge/version-0.6.0--aix-blue)
![Platform](https://img.shields.io/badge/platform-AIX%207.1%2B-blue)

---

## Demo

![C-Sentinel Demo](demo/csentinel-demo.gif)

*Quick analysis, baseline learning, and drift detection on AIX 7.3*

---

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Launching February 2026

---

## Installation

> ℹ️ **What's included**: The RPM package installs the **command-line tool** (`/opt/freeware/bin/sentinel`). The web dashboard is **optional** and requires separate installation (PostgreSQL, Python 3.11+, nginx). See [Dashboard README](dashboard/README.md) for details.

### Option 1: dnf (Recommended)

Add the LibrePower repository and install with one command:

```bash
# Add repository (one-time setup)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install C-Sentinel
dnf install csentinel4aix
```

📦 Repository details: https://aix.librepower.org/

### Option 2: Direct RPM Install

```bash
cd /tmp

# Download RPM from GitLab releases
curl -L -o csentinel4aix-1.0.0-3.librepower.aix7.3.ppc.rpm \
  https://gitlab.com/librepower/aix/-/raw/main/csentinel4aix/RPMS/csentinel4aix-1.0.0-3.librepower.aix7.3.ppc.rpm

# Install
rpm -ivh csentinel4aix-1.0.0-3.librepower.aix7.3.ppc.rpm
```

> ⚠️ **Note**: Use `-L` flag with curl to follow redirects.

### Option 3: Build from Source

See [RPM-BUILD.md](RPM-BUILD.md) for detailed build instructions.

### Quick Start (After Installation)

```bash
# Quick system analysis
sentinel -q -n

# Include security audit events (AIX)
sentinel -q -n -a

# Full file integrity check (~171 critical files, AIX only)
sentinel -F -q

# Learn baseline (first time)
sentinel -l -n

# Compare against baseline
sentinel -b -q -n

# Generate JSON for AI analysis
sentinel -j -n -a > system-fingerprint.json
```

For the optional web dashboard installation, see [dashboard/README.md](dashboard/README.md).

---

## AIX Port Status & Features

**Status:** Active development (~95% feature parity) - Suitable for testing and pilot deployments.

### ✅ Working Features

- **System Monitoring** - libperfstat integration (uptime, load, memory)
- **Process Monitoring** - Full /proc/psinfo support (408+ processes tracked)
- **Network Monitoring** - TCP/UDP listeners with **PID attribution for 70+ ports** (SSH, databases, IBM middleware, SAP, etc.)
- **Config Drift Detection** - SHA256 checksums with baseline comparison
- **Process Chain Analysis** - Detect suspicious parent-child relationships
- **Baseline Learning** - Anomaly detection and deviation alerts
- **AIX Audit Integration** - Native audit subsystem support with brute-force detection
- **Full File Integrity Mode** - 171 critical files (PowerSC RTC-comparable) with `-F` flag

### ⏳ Planned

- Production hardening
- Extensive enterprise testing

**Documentation:** [AIX Installation Guide](README.AIX.md) | [Complete Port Status](AIX_PORT_STATUS.md)

**Feedback:** Testing welcome! Report issues on [GitLab](https://gitlab.com/librepower/aix/-/issues)

---

## Usage Examples

See [README.AIX.md](README.AIX.md) for complete AIX usage documentation and examples.

## Dashboard (Optional)

The web dashboard provides multi-host monitoring with authentication, alerts, and historical tracking.

**Requirements:** PostgreSQL, Python 3.11+, nginx

See [dashboard/README.md](dashboard/README.md) for installation instructions.

## Platform Support Matrix

| Feature | Linux | AIX 7.1/7.2/7.3 | Notes |
|---------|-------|-----------------|-------|
| System Info | ✅ | ✅ | Full support |
| Process Monitoring | ✅ | ✅ | Full support |
| Config File Monitoring | ✅ | ✅ | Full support |
| Network Monitoring | ✅ | ✅ | AIX: **PID attribution (70+ ports)** |
| Audit Integration | ✅ | ✅ | AIX: Native audit via `-a` flag |
| Full File Integrity | ⚠️ | ✅ | AIX: 171 files via `-F` flag |
| Baseline Learning | ✅ | ✅ | Full support |
| Web Dashboard | ✅ | ✅ | Full support (requires PostgreSQL) |
| Long Options (--xxx) | ✅ | ❌ | AIX: Use short (-x) |

**Legend:** ✅ Fully supported | ⚠️ Limited | ❌ Not available

---

## Support

- **Issues**: https://gitlab.com/librepower/aix/-/issues
- **Documentation**: [README.AIX.md](README.AIX.md) | [AIX_PORT_STATUS.md](AIX_PORT_STATUS.md)

## License

MIT License - see [LICENSE](LICENSE) file for details

## Acknowledgments

- Original project: [williamofai/c-sentinel](https://github.com/williamofai/c-sentinel)
- AIX port: LibrePower team
- Contributors: See [CONTRIBUTORS.md](CONTRIBUTORS.md)

## Citation

If you use C-Sentinel in your research or production environment, please cite:

```bibtex
@software{c_sentinel,
  title = {C-Sentinel: Semantic Observability for UNIX Systems},
  author = {William Murray and LibrePower Team},
  year = {2025},
  url = {https://github.com/librepower/c-sentinel4aix},
  version = {0.6.0-aix}
}
```

---

**Version:** 0.6.0-aix
**Last Updated:** 2026-01-22
**Supported Platforms:** Linux (all), AIX 7.1, AIX 7.2, AIX 7.3
**License:** MIT
