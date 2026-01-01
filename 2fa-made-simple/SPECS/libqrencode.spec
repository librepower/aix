Name:           libqrencode
Version:        4.1.1
Release:        4.aix7.3.sixe
Summary:        QR Code encoding library
License:        LGPL-2.1
Group:          System Environment/Libraries
URL:            https://fukuchi.org/works/qrencode/
Vendor:         SIXE - IBM Business Partner
Packager:       Hugo Blanco <hugo.blanco@sixe.eu>

%description
Libqrencode is a library for encoding data in a QR Code symbol.
Required for QR code display in google-authenticator on AIX.

Compiled for AIX by SIXE - IBM Business Partner
https://sixe.eu

%install
mkdir -p %{buildroot}/opt/freeware/lib
mkdir -p %{buildroot}/opt/freeware/include

cp /opt/freeware/lib/libqrencode.a %{buildroot}/opt/freeware/lib/
cp /opt/freeware/lib/libqrencode.so.4 %{buildroot}/opt/freeware/lib/
cp /opt/freeware/include/qrencode.h %{buildroot}/opt/freeware/include/

cd %{buildroot}/opt/freeware/lib
ln -sf libqrencode.so.4 libqrencode.so

%files
%defattr(-,root,system,-)
/opt/freeware/lib/libqrencode.a
/opt/freeware/lib/libqrencode.so.4
/opt/freeware/lib/libqrencode.so
/opt/freeware/include/qrencode.h

%post
echo "libqrencode %{version} installed - SIXE (https://sixe.eu)"
