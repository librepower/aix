Name:           libunwind
Version:        17.1.3
Release:        1.librepower
Summary:        Stack unwinding library for AIX (from IBM Open XL Runtime)
License:        Apache-2.0
URL:            https://public.dhe.ibm.com/aix/products/ccpp/
Vendor:         LibrePower
Packager:       LibrePower <hello@librepower.org>

# Provides the shared object that Rust binaries need
Provides:       libunwind.a(libunwind.so.1)
Provides:       libunwind.a(libunwind.so.1)(64bit)

%description
Stack unwinding library required by Rust-compiled binaries on AIX.
This is a repackaging of libunwind.a from IBM Open XL C++ 17.1.3 Runtime.

Required by: ripgrep, fd, delta, eza, gping, starship, and other Rust tools.

Original source: IBM_OPEN_XL_CPP_UTILITIES_17.1.3.0_AIX.tar.Z
IBM provides this freely at https://public.dhe.ibm.com/aix/products/ccpp/

%install
mkdir -p %{buildroot}/usr/lpp/xlC/lib
mkdir -p %{buildroot}/usr/lib
cp /tmp/libunwind.a %{buildroot}/usr/lpp/xlC/lib/
ln -sf /usr/lpp/xlC/lib/libunwind.a %{buildroot}/usr/lib/libunwind.a

%files
%dir /usr/lpp/xlC
%dir /usr/lpp/xlC/lib
/usr/lpp/xlC/lib/libunwind.a
/usr/lib/libunwind.a

%changelog
* Tue Feb 04 2026 LibrePower <hello@librepower.org> - 17.1.3-1
- Initial package for LibrePower AIX repo
- Enables Rust packages on AIX 7.1 and 7.2
