# LibrePower AIX

Open source tools and packages for AIX/VIOS on IBM Power systems.

Part of the [LibrePower](https://librepower.org) initiative - extending the life of IBM Power hardware through open source.

## Available Packages

### 🔐 2FA Made Simple

Google Authenticator two-factor authentication for AIX/VIOS, done right.

**Features:**
- ✅ QR codes work (libqrencode included)
- ✅ Bilingual setup wizards (English/Spanish)
- ✅ NTP verification before setup
- ✅ Secure defaults (TOTP, rate limiting)
- ✅ Keeps STD_AUTH - sudo works normally
- ✅ Full rollback and emergency access instructions

📁 **[Documentation & Downloads](2fa-made-simple/)**

## Quick Start

```bash
cd /tmp

# Download
curl -L -O https://github.com/librepower/aix/releases/download/2fa-v1.0/libqrencode-4.1.1-4.librepower.aix7.3.ppc.rpm
curl -L -O https://github.com/librepower/aix/releases/download/2fa-v1.0/google-authenticator-1.10-1.aix7.1.ppc.rpm
curl -L -O https://github.com/librepower/aix/releases/download/2fa-v1.0/google-authenticator-setup-1.0-4.librepower.aix7.3.ppc.rpm

# Install
rpm -ivh libqrencode-4.1.1-4.librepower.aix7.3.ppc.rpm
rpm -ivh google-authenticator-1.10-1.aix7.1.ppc.rpm
rpm -ivh google-authenticator-setup-1.0-4.librepower.aix7.3.ppc.rpm

# Configure 2FA
google-authenticator-setup      # English
google-authenticator-configura  # Español
```

> ⚠️ **Important**: Use `curl -L` to follow GitHub redirects.

## Contributing

Contributions welcome! Open an issue or submit a PR.

## License

- Documentation and scripts: Apache-2.0
- libqrencode: LGPL-2.1
- google-authenticator: Apache-2.0

## About LibrePower

LibrePower promotes open source on IBM Power systems and extends the useful life of Power hardware through community-driven software.

🌍 Sustainability through longevity.

---

*Maintained by [SIXE](https://sixe.eu) - IBM Business Partner*
