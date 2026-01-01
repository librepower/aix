# 2FA Made Simple for AIX/VIOS

Google Authenticator two-factor authentication for AIX/VIOS, with working QR codes, safe configuration, and easy bilingual setup wizards.

## Download

### Option 1: curl (Recommended)

```bash
cd /tmp

# Required packages
curl -L -o libqrencode-4.1.1-4.librepower.aix7.3.ppc.rpm \
  https://github.com/librepower/aix/releases/download/2fa-v1.0/libqrencode-4.1.1-4.librepower.aix7.3.ppc.rpm

curl -L -o google-authenticator-1.10-1.aix7.1.ppc.rpm \
  https://github.com/librepower/aix/releases/download/2fa-v1.0/google-authenticator-1.10-1.aix7.1.ppc.rpm

# Optional: Easy setup wizards (English & Spanish)
curl -L -o google-authenticator-setup-1.0-4.librepower.aix7.3.ppc.rpm \
  https://github.com/librepower/aix/releases/download/2fa-v1.0/google-authenticator-setup-1.0-4.librepower.aix7.3.ppc.rpm

# Verify downloads
file *.rpm
```

> ⚠️ **Important**: Use `-L` flag to follow redirects. Do NOT download from `/blob/` URLs.

### Option 2: GitHub Releases Page

Download from [Releases](https://github.com/librepower/aix/releases/tag/2fa-v1.0)

## Easy Setup Wizards

After installing the packages, use our setup wizards:

### English
```bash
google-authenticator-setup
```

### Español
```bash
google-authenticator-configura
```

Features:
- ✅ Verifies NTP synchronization before setup
- ✅ Secure defaults (TOTP, disallow reuse, rate limiting)
- ✅ Step-by-step guidance with colored output
- ✅ Clear emergency access information (HMC console)

## Quick Start

```bash
# 1. Configure NTP first (critical for TOTP!)
cat > /etc/ntp.conf << 'NTPEOF'
driftfile /etc/ntp.drift
server 0.pool.ntp.org iburst
server 1.pool.ntp.org iburst
server time.google.com iburst
restrict default limited kod nomodify notrap nopeer noquery
restrict 127.0.0.1
NTPEOF

ntpdate -u pool.ntp.org
startsrc -s xntpd
# Enable at boot: edit /etc/rc.tcpip and uncomment the xntpd entry

# 2. Install packages
rpm -ivh libqrencode-4.1.1-4.librepower.aix7.3.ppc.rpm
rpm -ivh google-authenticator-1.10-1.aix7.1.ppc.rpm
rpm -ivh google-authenticator-setup-1.0-4.librepower.aix7.3.ppc.rpm

# 3. Configure PAM - add to /etc/pam.conf:
# SSH 2FA for AIX made SIMPLE
sshd    auth       required   pam_aix
sshd    auth       required   /usr/lib/security/pam_google_authenticator.so nullok no_increment_hotp
sshd    account    required   pam_aix
sshd    password   required   pam_aix
sshd    session    required   pam_aix

# 4. Configure SSH - add to /etc/ssh/sshd_config:
# SSH 2FA for AIX made SIMPLE
UsePAM yes
KbdInteractiveAuthentication yes

# 5. Restart SSH
stopsrc -s sshd
startsrc -s sshd

# NOTE: After this step, 2FA is ACTIVE. Users with ~/.google_authenticator
# need password + code. Users without it only need password (nullok).

# 6. Setup 2FA for a user
google-authenticator-setup           # English wizard
google-authenticator-configura       # Spanish wizard
```

## Why This Package?

IBM released google-authenticator for AIX but their [official guide](https://community.ibm.com/community/user/discussion/google-authenticator-libpam-is-now-available-on-aix-toolbox) has issues:

| Problem | IBM's Approach | Our Solution |
|---------|---------------|--------------|
| No QR code | "Failed to use libqrencode" | ✅ QR works (libqrencode included) |
| Breaks sudo | Changes auth_type to PAM_AUTH | ✅ Keeps STD_AUTH |
| No NTP warning | TOTP fails silently | ✅ NTP checked before setup |
| Confusing prompts | "update file?" (misleading) | ✅ Clear bilingual wizards |
| No rollback | Users get locked out | ✅ Full rollback instructions |
| No emergency access | Panic if 2FA fails | ✅ HMC console documented |

## Documentation

- **[INSTALL_2FA.txt](INSTALL_2FA.txt)** - Full English documentation
- **[INSTALL_2FA_ES.txt](INSTALL_2FA_ES.txt)** - Documentación completa en español

## Package Contents

```
2fa-made-simple/
├── RPMS/
│   ├── google-authenticator-1.10-1.aix7.1.ppc.rpm       # IBM official
│   ├── libqrencode-4.1.1-4.librepower.aix7.3.ppc.rpm          # QR library
│   └── google-authenticator-setup-1.0-4.librepower.aix7.3.ppc.rpm
├── SPECS/
│   └── libqrencode.spec
├── SOURCES/
│   ├── 2fa-check                        # Optional login prompt
│   ├── google-authenticator-setup       # English wizard
│   └── google-authenticator-configura   # Spanish wizard
├── INSTALL_2FA.txt
├── INSTALL_2FA_ES.txt
└── README.md
```

## Requirements

- AIX 7.1+ or VIOS 3.x
- NTP configured (critical for TOTP)
- Root access

## Important Notes

⚠️ **Safe installation**: RPMs only install binaries. 2FA is NOT activated until you manually configure PAM and SSH.

🔓 **Emergency access**: Serial console (HMC) never asks for 2FA - you can always recover.

👥 **Gradual rollout**: `nullok` option allows users without 2FA to still login normally.

## License

- libqrencode: LGPL-2.1 (compiled for AIX by SIXE)
- google-authenticator: Apache-2.0 (official IBM package)
- google-authenticator-setup/configura: Apache-2.0 (LibrePower)
- Documentation and scripts: Apache-2.0

## Credits

- Google Authenticator PAM module by Google
- libqrencode by Kentaro Fukuchi
- AIX packaging and documentation by [SIXE](https://sixe.eu)
- Part of the [LibrePower](https://librepower.org) initiative
