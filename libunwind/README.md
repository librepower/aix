# libunwind for AIX

Stack unwinding library required by Rust-compiled binaries on AIX.

## Source

This is a repackaging of `libunwind.a` from IBM Open XL C++ 17.1.3 Runtime.

**Original source**: [IBM Open XL C++ Utilities](https://public.dhe.ibm.com/aix/products/ccpp/xlc.rte.aix.17.1.3/)

IBM provides this freely for redistribution.

## Why is this needed?

Rust programs compiled for AIX require `libunwind.a(libunwind.so.1)` for stack unwinding (panic handling, backtraces, etc.).

On AIX 7.3, this library is typically available if IBM Open XL is installed. On AIX 7.1 and 7.2, it may not be present.

This package provides the library for all AIX versions.

## Packages that require libunwind

- ripgrep
- fd
- delta
- eza
- gping
- starship

## Installation

```bash
dnf install libunwind
```

Or the Rust packages will automatically install it as a dependency:

```bash
dnf install ripgrep fd delta eza gping starship
```

## License

Apache-2.0 (IBM Open XL C++ Runtime)

---

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Beta live now!

---

*Part of [LibrePower](https://librepower.org) - Unlocking Power Systems through open source. Unmatched RAS and TCO. Minimal footprint 🌍*
