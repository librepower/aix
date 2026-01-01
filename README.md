# LibrePower AIX

Open source tools and packages for AIX/VIOS on IBM Power systems.

Part of the [LibrePower](https://librepower.org) initiative - extending the life of IBM Power hardware through open source. 🚀 Launching February 2026

## Available Packages

### 🔐 2FA Made Simple

Google Authenticator two-factor authentication for AIX/VIOS, done right.

**Why this instead of [IBM's guide](https://community.ibm.com/community/user/discussion/google-authenticator-libpam-is-now-available-on-aix-toolbox)?**

| Issue | IBM Guide | LibrePower |
|-------|-----------|------------|
| QR Code | ❌ "Failed to use libqrencode" | ✅ Works (libqrencode included) |
| auth_type | Changes to PAM_AUTH | Keeps STD_AUTH |
| sudo | ⚠️ Breaks ([confirmed](https://community.ibm.com/community/user/discussion/google-authenticator-libpam-is-now-available-on-aix-toolbox)) | ✅ Works normally |
| NTP | Not mentioned | Documented as critical step |
| Rollback | Not documented | Full rollback instructions |
| Emergency access | Not mentioned | HMC console always works |

📁 **[Download 2FA Made Simple](2fa-made-simple/)**

## Quick Start

```bash
# Download RPMs from 2fa-made-simple/RPMS/
rpm -ivh libqrencode-4.1.1-3.aix7.3.sixe.aix7.3.ppc.rpm
rpm -ivh google-authenticator-1.10-1.aix7.1.ppc.rpm

# Then follow INSTALL_2FA.txt for PAM and SSH configuration
```

## Contributing

Contributions welcome! Open an issue or submit a PR.

- **SPECS/** - RPM spec files for rebuilding
- **SOURCES/** - Scripts and patches

## License

- Documentation and scripts: Apache-2.0
- libqrencode: LGPL-2.1
- google-authenticator: Apache-2.0 (official IBM package)

## About LibrePower

Power hardware offers exceptional longevity. LibrePower brings more and better open source to IBM Power - extending hardware lifespan while expanding what's possible.

🌍 Longer life. Endless possibilities. Unmatched TCO. Minimal footprint.

---

*Maintained by [SIXE](https://sixe.eu) - IBM Business Partner*
