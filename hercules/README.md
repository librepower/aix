# Hercules - IBM Mainframe Emulator for AIX

![AIX 7.2+](https://img.shields.io/badge/AIX-7.2+-blue)
![C](https://img.shields.io/badge/C-GCC%2013.3-yellow)
![License](https://img.shields.io/badge/license-QPL-green)

Hercules is an open-source IBM mainframe emulator (System/370, ESA/390, z/Architecture). Run MVS, VM/370, z/OS, z/VSE, and z/VM on AIX POWER systems. Native 64-bit ppc64 build with big-endian advantage — POWER hardware shares the same byte order as z/Architecture.

![Hercules Demo](demo/hercules-demo.gif)

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)**

---

## Why Hercules on AIX?

| Feature | Description |
|---------|-------------|
| **Native big-endian** | POWER and z/Architecture share byte order — zero byte-swap overhead |
| **64-bit XCOFF** | Native AIX binary, not a Linux port |
| **3 architectures** | S/370, ESA/390, z/Architecture in one binary |
| **128 CPUs** | Up to 128 emulated processors |
| **Full DASD** | CKD (3390, 3380) and FBA DASD support |
| **Tape** | 3420/3480/3490 tape emulation |
| **Console** | 3270 terminal via tn3270 |

## Features

- **S/370, ESA/390, z/Architecture** emulation — run MVS 3.8j through z/OS
- **DASD utilities** — dasdinit, dasdload, dasdcopy, dasdls for disk image management
- **Tape utilities** — hetinit, hetget, hetmap, tapecopy for tape image management
- **HTTP console** — web-based operator interface
- **CCKD/CCKD64** — compressed DASD images with BZIP2 and ZLIB
- **Crypto support** — via /dev/urandom on AIX

## Installation

### Option 1: dnf (Recommended)

```bash
# Add LibrePower repo (if not already added)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install Hercules
dnf install hercules
```

### Option 2: Manual RPM

```bash
curl -L -o hercules-4.9.1-1.librepower.aix7.1.ppc.rpm \
  https://aix.librepower.org/packages/hercules-4.9.1-1.librepower.aix7.1.ppc.rpm

rpm -ivh hercules-4.9.1-1.librepower.aix7.1.ppc.rpm
```

## Quick Start

```bash
# Check version
hercules --version

# Create a DASD volume
dasdinit -a myvol.3390 3390-1 VOL001

# Start Hercules with a config file
hercules -f myconfig.cnf
```

### Minimal Configuration

Create a file `myconfig.cnf`:

```
CPUSERIAL 000001
CPUMODEL  3906
MAINSIZE  64
ARCHMODE  z/Arch
CNSLPORT  3270
NUMCPU    1
```

Then connect a 3270 terminal emulator to `localhost:3270`.

## Included Utilities

| Utility | Description |
|---------|-------------|
| `hercules` | Main emulator |
| `herclin` | Line-mode emulator |
| `dasdinit` | Create empty DASD images |
| `dasdload` | Load DASD from card deck |
| `dasdcopy` | Copy/convert DASD images |
| `dasdls` | List DASD contents |
| `dasdcat` | Extract PDS members |
| `dasdseq` | Sequential DASD extract |
| `hetinit` | Create tape images |
| `hetget` | Extract from tape images |
| `hetmap` | Map tape contents |
| `tapecopy` | Copy tape images |
| `tapemap` | Display tape layout |
| `cckdcdsk` | Check CCKD image integrity |
| `cckdcomp` | Compress CCKD images |
| `convto64` | Convert CCKD to CCKD64 |

## Known Limitations

- **No guest networking** — AIX does not support TUN/TAP devices, so CTC/LCS/QETH network adapters are not available. Guest operating systems run without network connectivity.
- **Console access only** — Connect via 3270 terminal emulator (tn3270).
- **PCLMULQDQ warning** — This x86-specific instruction is not available on POWER. The warning is harmless and does not affect operation.

## Build Notes

- SDL Hercules Hyperion 4.9.1 (actively maintained fork)
- Compiled with GCC 13.3.0, `-maix64`
- External packages: crypto, decNumber, SoftFloat, telnet (all 64-bit)
- AIX-specific patches: symbol renames for AIX header conflicts, TUN/TAP stubs, crypto /dev/urandom fix

## Links

- [SDL Hercules Hyperion](https://github.com/SDL-Hercules-390/hyperion)
- [Hercules Documentation](https://sdl-hercules-390.github.io/html/)
- [MVS/TK5](https://www.prince-webdesign.nl/tk5) — Free MVS 3.8j turnkey system
- [LibrePower](https://librepower.org)

## License

- Hercules: Q Public License 1.0 (QPL)
- AIX port: QPL (LibrePower)

## Credits

- Hercules by Roger Bowler, Jan Jaeger, and the SDL Hyperion team
- AIX port and packaging by [LibrePower](https://librepower.org)
