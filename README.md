# LibrePower AIX

Open source tools and packages for AIX/VIOS on IBM Power systems.

Part of the [LibrePower](https://librepower.org) initiative - extending the life of IBM Power hardware through open source.

## Available Packages

### 🔐 2FA Made Simple

Google Authenticator two-factor authentication for AIX/VIOS, done right.

**LibrePower Edition improvements:**
- ✅ TOTP (time-based) is the default - no confusing "time-based?" question
- ✅ Clearer "save configuration" message instead of confusing "update file"
- ✅ QR codes work (libqrencode included)
- ✅ Keeps STD_AUTH - sudo works normally
- ✅ NTP documented as critical step
- ✅ Full rollback and emergency access instructions

📁 **[Download 2FA Made Simple](2fa-made-simple/)**

## Quick Start

```bash
# Download RPMs using curl (requires -L for redirects)
cd /tmp

curl -L -o libqrencode-4.1.1-4.librepower.aix7.3.ppc.rpm \
  https://github.com/librepower/aix/releases/download/2fa-v1.0/libqrencode-4.1.1-4.librepower.aix7.3.ppc.rpm

curl -L -o google-authenticator-1.10-1.librepower.ppc.rpm \
  https://github.com/librepower/aix/releases/download/2fa-v1.0/google-authenticator-1.10-1.librepower.aix7.3.aix7.3.ppc.rpm

# Install
rpm -ivh libqrencode-4.1.1-4.librepower.aix7.3.ppc.rpm
rpm -ivh google-authenticator-1.10-1.librepower.ppc.rpm

# Then follow INSTALL_2FA.txt for NTP, PAM and SSH configuration
```

> ⚠️ **Important**: Always use `curl -L` to follow GitHub redirects. Do NOT download from `/blob/` URLs - those return HTML pages, not binaries.

## Contributing

Contributions welcome! Open an issue or submit a PR.

- **SPECS/** - RPM spec files for rebuilding
- **SOURCES/** - Scripts and patches

## License

- Documentation and scripts: Apache-2.0
- libqrencode: LGPL-2.1
- google-authenticator: Apache-2.0

## About LibrePower

LibrePower promotes open source on IBM Power systems and extends the useful life of Power8/Power9 hardware through community-driven software.

🌍 Sustainability through longevity - hardware that lasts 10-15 years is good for the planet.

---

*Maintained by [SIXE](https://sixe.eu) - IBM Business Partner*
