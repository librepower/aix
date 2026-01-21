Name:           librepower-web-src
Version:        1.0
Release:        1.librepower.aix7.3
Summary:        SRC integration for web services (Apache, nginx, PHP-FPM)
License:        MIT
URL:            https://aix.librepower.org
Group:          System Environment/Daemons
BuildArch:      noarch

Requires:       httpd
Requires:       php83-fpm

%description
Integrates Apache httpd, nginx, and PHP-FPM with AIX System Resource
Controller (SRC) for unified service management using startsrc/stopsrc/lssrc.

After installation, manage web services with:
  startsrc -s httpd      # Start Apache
  startsrc -s nginx      # Start nginx
  startsrc -s php-fpm    # Start PHP-FPM
  stopsrc -s <service>   # Stop service
  lssrc -s <service>     # Check status

%install
mkdir -p %{buildroot}/opt/freeware/libexec/librepower
mkdir -p %{buildroot}/opt/freeware/share/doc/librepower-web-src

cat > %{buildroot}/opt/freeware/libexec/librepower/web-src-install.sh << 'SRCEOF'
#!/bin/bash
# Web Services SRC Integration for AIX
# LibrePower - https://librepower.org

install_httpd() {
    echo "Installing httpd SRC subsystem..."
    stopsrc -s httpd 2>/dev/null
    rmssys -s httpd 2>/dev/null
    mkssys -s httpd \
           -p /opt/freeware/sbin/httpd \
           -a "-DFOREGROUND" \
           -u 0 \
           -S \
           -n 15 \
           -f 9
    [ $? -eq 0 ] && echo "  httpd: OK" || echo "  httpd: FAILED"
}

install_nginx() {
    echo "Installing nginx SRC subsystem..."
    stopsrc -s nginx 2>/dev/null
    rmssys -s nginx 2>/dev/null
    mkssys -s nginx \
           -p /opt/freeware/sbin/nginx \
           -a "-g 'daemon off;'" \
           -u 0 \
           -S \
           -n 15 \
           -f 9
    [ $? -eq 0 ] && echo "  nginx: OK" || echo "  nginx: FAILED"
}

install_phpfpm() {
    echo "Installing php-fpm SRC subsystem..."
    stopsrc -s php-fpm 2>/dev/null
    rmssys -s php-fpm 2>/dev/null
    mkssys -s php-fpm \
           -p /opt/freeware/sbin/php-fpm \
           -a "-F" \
           -u 0 \
           -S \
           -n 15 \
           -f 9
    [ $? -eq 0 ] && echo "  php-fpm: OK" || echo "  php-fpm: FAILED"
}

remove_all() {
    echo "Removing SRC subsystems..."
    for svc in httpd nginx php-fpm; do
        stopsrc -s $svc 2>/dev/null
        rmssys -s $svc 2>/dev/null
        rmitab $svc 2>/dev/null
    done
    echo "Done."
}

show_status() {
    echo "=== Web Services Status ==="
    for svc in httpd nginx php-fpm mariadb11; do
        status=$(lssrc -s $svc 2>/dev/null | tail -1)
        if [ -n "$status" ]; then
            echo "$status"
        else
            echo "$svc: not registered"
        fi
    done
}

enable_boot() {
    echo "Enabling auto-start at boot..."
    rmitab httpd 2>/dev/null
    rmitab php-fpm 2>/dev/null
    mkitab 'httpd:2:once:/usr/bin/startsrc -s httpd'
    mkitab 'php-fpm:2:once:/usr/bin/startsrc -s php-fpm'
    echo "Added to /etc/inittab: httpd, php-fpm"
}

case "$1" in
    remove|uninstall)
        remove_all
        ;;
    status)
        show_status
        ;;
    boot)
        enable_boot
        ;;
    httpd)
        install_httpd
        ;;
    nginx)
        install_nginx
        ;;
    php-fpm)
        install_phpfpm
        ;;
    *)
        echo "LibrePower Web Services SRC Installer"
        echo "======================================"
        install_httpd
        install_nginx
        install_phpfpm
        echo ""
        echo "Commands:"
        echo "  startsrc -s httpd      # Start Apache"
        echo "  startsrc -s nginx      # Start nginx"
        echo "  startsrc -s php-fpm    # Start PHP-FPM"
        echo "  stopsrc -s <service>   # Stop service"
        echo "  lssrc -s <service>     # Check status"
        echo ""
        echo "Enable auto-start: $0 boot"
        ;;
esac
SRCEOF
chmod 755 %{buildroot}/opt/freeware/libexec/librepower/web-src-install.sh

cat > %{buildroot}/opt/freeware/share/doc/librepower-web-src/README.md << 'DOCEOF'
# LibrePower Web SRC Integration

Unified SRC management for web services on AIX.

## Quick Start

```bash
# Register all services with SRC
/opt/freeware/libexec/librepower/web-src-install.sh

# Start services
startsrc -s httpd
startsrc -s php-fpm
startsrc -s mariadb11

# Check status
lssrc -s httpd
lssrc -s php-fpm

# Enable auto-start at boot
/opt/freeware/libexec/librepower/web-src-install.sh boot
```

## Services

| Service | Command | Foreground Flag |
|---------|---------|-----------------|
| Apache  | startsrc -s httpd | -DFOREGROUND |
| nginx   | startsrc -s nginx | -g 'daemon off;' |
| PHP-FPM | startsrc -s php-fpm | -F |

## More Info

https://aix.librepower.org
DOCEOF

%post
echo "Registering web services with SRC..."
/opt/freeware/libexec/librepower/web-src-install.sh

%preun
if [ $1 -eq 0 ]; then
    /opt/freeware/libexec/librepower/web-src-install.sh remove
fi

%files
%dir /opt/freeware/libexec/librepower
/opt/freeware/libexec/librepower/web-src-install.sh
%doc /opt/freeware/share/doc/librepower-web-src/README.md

%changelog
* Mon Jan 20 2026 LibrePower <hello@librepower.org> - 1.0-1
- Initial release
- SRC integration for httpd, nginx, php-fpm
- Auto-registration on install
