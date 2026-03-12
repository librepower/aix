Name:           caddy
Version:        2.9.1
Release:        1.librepower
Summary:        Fast, multi-platform web server with automatic HTTPS
License:        Apache-2.0
URL:            https://caddyserver.com
Group:          Applications/Internet

%description
Caddy is a powerful, extensible platform to serve your sites, services,
and apps, written in Go. Automatic HTTPS, HTTP/2, HTTP/3 support.

Compiled for AIX ppc64 with fcntl locking patches for badger dependency.

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/bin
mkdir -p %{buildroot}/opt/freeware/etc/caddy
mkdir -p %{buildroot}/opt/freeware/var/lib/caddy
mkdir -p %{buildroot}/opt/freeware/var/log/caddy

# Binary
cp /tmp/caddy %{buildroot}/opt/freeware/bin/caddy
chmod 755 %{buildroot}/opt/freeware/bin/caddy

# Default Caddyfile
cat > %{buildroot}/opt/freeware/etc/caddy/Caddyfile << 'CADDYEOF'
# Caddy default configuration for AIX
# See https://caddyserver.com/docs/caddyfile

:80 {
    root * /opt/freeware/var/lib/caddy/www
    file_server

    log {
        output file /opt/freeware/var/log/caddy/access.log
    }
}
CADDYEOF

# Default index
mkdir -p %{buildroot}/opt/freeware/var/lib/caddy/www
cat > %{buildroot}/opt/freeware/var/lib/caddy/www/index.html << 'HTMLEOF'
<!DOCTYPE html>
<html><head><title>Caddy on AIX</title></head>
<body><h1>Caddy is running on AIX</h1>
<p>Served by <a href="https://caddyserver.com">Caddy</a> via
<a href="https://aix.librepower.org">LibrePower</a>.</p>
</body></html>
HTMLEOF

%files
%defattr(-,root,system,-)
/opt/freeware/bin/caddy
%config(noreplace) /opt/freeware/etc/caddy/Caddyfile
%dir /opt/freeware/var/lib/caddy
%dir /opt/freeware/var/lib/caddy/www
/opt/freeware/var/lib/caddy/www/index.html
%dir /opt/freeware/var/log/caddy

%changelog
* Wed Mar 12 2026 LibrePower <hello@librepower.org> - 2.9.1-1.librepower
- Initial AIX port
- Patched dgraph-io/badger v1/v2 for AIX fcntl locking
- Compiled with Go 1.24.11 (CGO_ENABLED=0)
