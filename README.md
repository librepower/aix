# LibrePower AIX

**LibrePower - Unlocking Power Systems through open source. Unmatched RAS and TCO. Minimal footprint 🌍**

Open source tools and packages for AIX/VIOS on IBM Power systems.

## Available Packages

### 🔍 fzf - Fuzzy Finder
*NEW - First Go-based tool compiled for AIX*

The incredibly popular command-line fuzzy finder, now on AIX. Search through anything: files, processes, packages, command history.

- ✅ Blazing fast (500,000 items in < 1 second)
- ✅ Zero dependencies (single static binary)
- ✅ AIX-specific helper scripts (fzf-rpm, fzf-proc, fzf-svc)
- ✅ Shell integration (Ctrl-R history search, Ctrl-T file picker)
- ✅ First proof that modern Go tools can run on AIX

📁 **[Documentation & Downloads](fzf-fuzzy-finder/)**

---

### 🔐 2FA Made Simple

Google Authenticator two-factor authentication for AIX/VIOS, done right.

- ✅ QR codes work (libqrencode included)
- ✅ Bilingual setup wizards (EN/ES)
- ✅ NTP verification before setup
- ✅ Secure defaults
- ✅ Full rollback and emergency access instructions

📁 **[Documentation & Downloads](2fa-made-simple/)**

## Quick Install

### fzf
```bash
curl -L -o fzf.rpm https://github.com/librepower/aix/releases/download/fzf-v0.46.1/fzf-0.46.1-1.aix7.3.sixe.aix7.3.ppc.rpm
rpm -ivh fzf.rpm
```

### 2FA Made Simple
```bash
curl -L -o libqrencode.rpm https://github.com/librepower/aix/releases/download/2fa-v1.0/libqrencode-4.1.1-4.librepower.aix7.3.ppc.rpm
curl -L -o google-auth.rpm https://github.com/librepower/aix/releases/download/2fa-v1.0/google-authenticator-1.10-1.aix7.1.ppc.rpm
rpm -ivh libqrencode.rpm google-auth.rpm
```

## Contribute

**We welcome contributions from the community!**

Have you compiled open source software for AIX? Built something useful for Power Systems? Share it here.

### What we're looking for

- 📦 RPM packages compiled for AIX/VIOS
- 🛠️ Tools and utilities for Power Systems
- 📚 Documentation and guides
- 🔧 Patches for AIX compatibility

### How to contribute

1. **Fork** this repository
2. **Add** your package:
   ```
   your-package/
   ├── RPMS/           # Compiled RPMs
   ├── SPECS/          # Spec files (for reproducibility)
   ├── SOURCES/        # Scripts, patches, configs
   └── README.md       # Documentation
   ```
3. **Submit** a Pull Request

### Guidelines

- Include license information
- Document tested AIX/VIOS versions
- Provide build instructions when possible
- Documentation in any language welcome. We maintain English and Spanish; other languages supported if volunteers step up

### Ideas welcome too

No code yet? Open an [Issue](https://github.com/librepower/aix/issues) with your ideas, requests, or questions.

---

Let's build something great for Power Systems together.

---

*Maintained by [SIXE](https://sixe.eu) - IBM Business Partner*
