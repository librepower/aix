# 2FA Made Simple for AIX/VIOS

Google Authenticator two-factor authentication for AIX/VIOS, with working QR codes and safer configuration.

## Download

### Option 1: curl (Recommended)

```bash
cd /tmp

curl -L -o libqrencode-4.1.1-4.aix7.3.sixe.ppc.rpm \
  https://github.com/librepower/aix/releases/download/2fa-v1.0/libqrencode-4.1.1-4.aix7.3.sixe.ppc.rpm

curl -L -o google-authenticator-1.10-1.aix7.1.ppc.rpm \
  https://github.com/librepower/aix/releases/download/2fa-v1.0/google-authenticator-1.10-1.aix7.1.ppc.rpm

# Verify downloads
file *.rpm
# Should show: RPM v3.0 bin...
```

> ⚠️ **Important**: Use `-L` flag to follow redirects. Do NOT download from `/blob/` URLs.

### Option 2: GitHub Releases Page

Download from [Releases](https://github.com/librepower/aix/releases/tag/2fa-v1.0)

## Quick Start

```bash
# 1. Configure NTP first (critical for TOTP!)
cat > /etc/ntp.conf << 'NTPEOF'
# NTP Configuration
driftfile /etc/ntp.drift

# Public NTP servers
server 0.pool.ntp.org iburst
server 1.pool.ntp.org iburst
server 2.pool.ntp.org iburst
server time.google.com iburst

# Restrictions
restrict default limited kod nomodify notrap nopeer noquery
restrict 127.0.0.1
NTPEOF

ntpdate -u pool.ntp.org
startsrc -s xntpd

# 2. Install packages
rpm -ivh libqrencode-4.1.1-4.aix7.3.sixe.ppc.rpm
rpm -ivh google-authenticator-1.10-1.aix7.1.ppc.rpm

# 3. Configure PAM - add to /etc/pam.conf:
sshd    auth       required   pam_aix
sshd    auth       required   /usr/lib/security/pam_google_authenticator.so nullok no_increment_hotp
sshd    account    required   pam_aix
sshd    password   required   pam_aix
sshd    session    required   pam_aix

# 4. Configure SSH - add to /etc/ssh/sshd_config:
UsePAM yes
KbdInteractiveAuthentication yes

# 5. Restart SSH
stopsrc -s sshd && startsrc -s sshd

# 6. Setup 2FA for a user
google-authenticator -t -i "YourCompany"
```

## Why This Package?

IBM released google-authenticator for AIX but their [official guide](https://community.ibm.com/community/user/discussion/google-authenticator-libpam-is-now-available-on-aix-toolbox) has issues:

| Problem | IBM's Approach | Our Solution |
|---------|---------------|--------------|
| No QR code | "Failed to use libqrencode" | ✅ QR works (libqrencode included) |
| Breaks sudo | Changes auth_type to PAM_AUTH | ✅ Keeps STD_AUTH |
| No NTP warning | TOTP fails silently | ✅ NTP documented as Step 0 |
| No rollback | Users get locked out | ✅ Full rollback instructions |
| No emergency access | Panic if 2FA fails | ✅ HMC console documented |

## Documentation

- **[INSTALL_2FA.txt](INSTALL_2FA.txt)** - Full English documentation
- **[INSTALL_2FA_ES.txt](INSTALL_2FA_ES.txt)** - Documentación completa en español

## Package Contents

```
2fa-made-simple/
├── RPMS/                           # Ready-to-install binaries
│   ├── google-authenticator-*.rpm  # Official IBM package
│   └── libqrencode-*.rpm           # QR library (SIXE build)
├── SPECS/                          # For rebuilding RPMs
│   └── libqrencode.spec
├── SOURCES/                        # Scripts
│   └── 2fa-check                   # Optional login prompt
├── INSTALL_2FA.txt                 # English docs
├── INSTALL_2FA_ES.txt              # Spanish docs
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

## Optional: Login Prompt

To prompt users without 2FA to enable it at login:

```bash
# Copy script
cp SOURCES/2fa-check /opt/freeware/bin/
chmod +x /opt/freeware/bin/2fa-check

# Add to /etc/profile
echo '[ -x /opt/freeware/bin/2fa-check ] && . /opt/freeware/bin/2fa-check' >> /etc/profile
```

## License

- libqrencode: LGPL-2.1 (compiled for AIX by SIXE)
- google-authenticator: Apache-2.0 (official IBM package)
- Documentation and scripts: Apache-2.0

## Credits

- Google Authenticator PAM module by Google
- libqrencode by Kentaro Fukuchi
- AIX packaging and documentation by [SIXE](https://sixe.eu)
- Part of the [LibrePower](https://librepower.org) initiative
