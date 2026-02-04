# age - Simple, modern file encryption

![AIX 7.3](https://img.shields.io/badge/AIX-7.3+-blue)
![Go](https://img.shields.io/badge/Go-1.24-00ADD8)
![License](https://img.shields.io/badge/license-BSD--3-green)

age is a simple, modern, and secure file encryption tool with small explicit keys, no config options, and UNIX-style composability.

## Installation

```bash
dnf install age
```

## Quick Start

```bash
# Generate a key pair
age-keygen -o key.txt

# Encrypt a file
age -r age1... file.txt > file.txt.age

# Decrypt a file
age -d -i key.txt file.txt.age > file.txt

# Encrypt with passphrase
age -p file.txt > file.txt.age
```

## Links

- [age GitHub](https://github.com/FiloSottile/age)
- [LibrePower](https://librepower.org)

## License

BSD-3-Clause

---

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Beta live now!

---

*Part of [LibrePower](https://librepower.org) - Unlocking Power Systems through open source. Unmatched RAS and TCO. Minimal footprint 🌍*
