%define php_version 8.3.16
%define php_major 8.3
%define _prefix /opt/freeware

Name:           php83
Version:        %{php_version}
Release:        1%{?dist}
Summary:        PHP 8.3 scripting language for AIX

License:        PHP-3.01
URL:            https://www.php.net/
Vendor:         LibrePower
Packager:       LibrePower <hello@librepower.org>

BuildRequires:  gcc >= 10
BuildRequires:  make
BuildRequires:  autoconf
BuildRequires:  libxml2-devel
BuildRequires:  openssl-devel
BuildRequires:  curl-devel
BuildRequires:  libsodium-devel
BuildRequires:  sqlite-devel
BuildRequires:  libzip-devel
BuildRequires:  gmp-devel
BuildRequires:  libicu-devel
BuildRequires:  oniguruma-devel
BuildRequires:  libgd-devel
BuildRequires:  freetype-devel
BuildRequires:  libjpeg-turbo-devel
BuildRequires:  libpng-devel
BuildRequires:  bzip2-devel
BuildRequires:  zlib-devel
BuildRequires:  gettext-devel
BuildRequires:  postgresql18-devel

Requires:       libxml2
Requires:       openssl
Requires:       curl
Requires:       libsodium
Requires:       sqlite
Requires:       libzip
Requires:       gmp
Requires:       libicu
Requires:       oniguruma
Requires:       libgd
Requires:       freetype
Requires:       libjpeg-turbo
Requires:       libpng
Requires:       bzip2
Requires:       zlib
Requires:       gettext

Provides:       php = %{php_version}
Provides:       php-cli = %{php_version}
Provides:       php-common = %{php_version}

%description
PHP 8.3 for IBM AIX 7.3+ (ppc64) compiled by LibrePower.
Includes 53 extensions: bcmath, bz2, curl, gd, intl, mbstring, mysqli,
openssl, pdo, pgsql, soap, sodium, zip, and more.

Tested with WordPress, Nextcloud, Flarum, Lychee, Kanboard.

%package fpm
Summary:        PHP 8.3 FastCGI Process Manager
Requires:       %{name} = %{version}-%{release}
Provides:       php-fpm = %{php_version}

%description fpm
PHP-FPM (FastCGI Process Manager) for PHP 8.3 on AIX.

%package devel
Summary:        PHP 8.3 development files
Requires:       %{name} = %{version}-%{release}
Provides:       php-devel = %{php_version}

%description devel
Development files for PHP 8.3 (headers, phpize, php-config).

%package pgsql
Summary:        PHP 8.3 PostgreSQL extension
Requires:       %{name} = %{version}-%{release}
Requires:       postgresql18-libs
Provides:       php-pgsql = %{php_version}

%description pgsql
PostgreSQL database extensions for PHP 8.3 (pgsql, pdo_pgsql).

%install
# Install from pre-built files
mkdir -p %{buildroot}%{_prefix}/bin
mkdir -p %{buildroot}%{_prefix}/sbin
mkdir -p %{buildroot}%{_prefix}/etc
mkdir -p %{buildroot}%{_prefix}/etc/php.d
mkdir -p %{buildroot}%{_prefix}/etc/php-fpm.d
mkdir -p %{buildroot}%{_prefix}/lib/php/extensions/no-debug-non-zts-20230831
mkdir -p %{buildroot}%{_prefix}/include/php
mkdir -p %{buildroot}%{_prefix}/lib/php/build
mkdir -p %{buildroot}%{_prefix}/php/man/man1
mkdir -p %{buildroot}%{_prefix}/php/man/man8
mkdir -p %{buildroot}%{_prefix}/php/php/fpm

# Copy binaries
cp -p %{_prefix}/bin/php %{buildroot}%{_prefix}/bin/
cp -p %{_prefix}/bin/php-cgi %{buildroot}%{_prefix}/bin/
cp -p %{_prefix}/bin/phpize %{buildroot}%{_prefix}/bin/
cp -p %{_prefix}/bin/php-config %{buildroot}%{_prefix}/bin/
cp -p %{_prefix}/sbin/php-fpm %{buildroot}%{_prefix}/sbin/

# Copy config
cp -p %{_prefix}/etc/php.ini %{buildroot}%{_prefix}/etc/
cp -p %{_prefix}/etc/php-fpm.conf %{buildroot}%{_prefix}/etc/
cp -rp %{_prefix}/etc/php-fpm.d/* %{buildroot}%{_prefix}/etc/php-fpm.d/

# Copy extensions
cp -rp %{_prefix}/lib/php/extensions/no-debug-non-zts-20230831/* %{buildroot}%{_prefix}/lib/php/extensions/no-debug-non-zts-20230831/

# Copy headers and build files
cp -rp %{_prefix}/include/php/* %{buildroot}%{_prefix}/include/php/
cp -rp %{_prefix}/lib/php/build/* %{buildroot}%{_prefix}/lib/php/build/

%files
%{_prefix}/bin/php
%{_prefix}/bin/php-cgi
%config(noreplace) %{_prefix}/etc/php.ini
%dir %{_prefix}/etc/php.d
%dir %{_prefix}/lib/php/extensions/no-debug-non-zts-20230831
%{_prefix}/lib/php/extensions/no-debug-non-zts-20230831/opcache.a

%files fpm
%{_prefix}/sbin/php-fpm
%config(noreplace) %{_prefix}/etc/php-fpm.conf
%dir %{_prefix}/etc/php-fpm.d
%config(noreplace) %{_prefix}/etc/php-fpm.d/www.conf

%files devel
%{_prefix}/bin/phpize
%{_prefix}/bin/php-config
%{_prefix}/include/php/
%{_prefix}/lib/php/build/

%files pgsql
# pgsql is built-in, this is a meta-package

%changelog
* Mon Jan 20 2026 LibrePower <hello@librepower.org> - 8.3.16-1
- Initial package for AIX 7.3 ppc64
- 53 extensions included
- Fixes for fiber assembly, OBJECT_MODE, pthread
