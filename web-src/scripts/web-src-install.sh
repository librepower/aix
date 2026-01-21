#!/bin/bash
# Web Services SRC Integration for AIX
# LibrePower - https://librepower.org
#
# Registers Apache, nginx, and PHP-FPM with AIX SRC
# Usage: ./web-src-install.sh [install|remove]

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
    echo "httpd: $(lssrc -s httpd | tail -1)"
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
    echo "nginx: $(lssrc -s nginx | tail -1)"
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
    echo "php-fpm: $(lssrc -s php-fpm | tail -1)"
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
    for svc in httpd nginx php-fpm; do
        lssrc -s $svc 2>/dev/null || echo "$svc: not registered"
    done
}

case "$1" in
    remove|uninstall)
        remove_all
        ;;
    status)
        show_status
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
        echo ""
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
        echo "Auto-start at boot (add to /etc/inittab):"
        echo "  mkitab 'httpd:2:once:/usr/bin/startsrc -s httpd'"
        echo "  mkitab 'php-fpm:2:once:/usr/bin/startsrc -s php-fpm'"
        ;;
esac
