# C-Sentinel Dashboard for AIX

**Web Dashboard for C-Sentinel System Monitoring**

![AIX 7.2+](https://img.shields.io/badge/AIX-7.2%2B-blue)
![Version](https://img.shields.io/badge/version-1.0.0-green)
![License](https://img.shields.io/badge/license-MIT-blue)

A Flask-based web dashboard for viewing system fingerprints across multiple AIX hosts.

## Join the Community

LibrePower is more than AIX—we're building open source support across the entire IBM Power ecosystem: AIX, IBM i, and Linux on Power (ppc64le).

📬 **[Subscribe to our newsletter](https://librepower.substack.com/subscribe)** for releases, technical articles, and community updates.

🌐 **[librepower.org](https://librepower.org)** — Launching February 2026

---

## Features

- **Real-time host monitoring** - See all hosts at a glance
- **Historical charts** - Memory and load trends over 24 hours
- **Network view** - All listening ports and connections
- **Config tracking** - SHA256 checksums of monitored files
- **Multi-host fleet support** - Monitor your entire AIX infrastructure
- **2FA authentication** - Secure admin access with TOTP
- **API key management** - Control agent access

## Installation

### Option 1: DNF (Recommended)

```bash
# Add LibrePower repository (one-time setup)
curl -fsSL https://aix.librepower.org/install.sh | sh

# Install dashboard and dependencies
dnf install csentinel-dashboard postgresql18-server nginx python3
```

### Option 2: Direct RPM

```bash
rpm -ivh csentinel-dashboard-1.0.0-1.librepower.aix7.3.noarch.rpm
```

## Setup Guide

### 1. Initialize PostgreSQL

```bash
# Initialize database cluster (first time only)
/opt/freeware/bin/initdb -D /var/lib/pgsql/data

# Start PostgreSQL
startsrc -s postgresql

# Create database and user
sudo -u postgres psql <<EOF
CREATE DATABASE sentinel;
CREATE USER sentinel WITH ENCRYPTED PASSWORD 'your-secure-password';
GRANT ALL PRIVILEGES ON DATABASE sentinel TO sentinel;
\c sentinel
GRANT ALL ON SCHEMA public TO sentinel;
EOF

# Initialize schema
sudo -u postgres psql -d sentinel -f /opt/sentinel-dashboard/migrate.sql
```

### 2. Install Python Dependencies

```bash
/opt/freeware/bin/pip3 install flask psycopg2-binary gunicorn
```

### 3. Configure Environment

Create `/opt/sentinel-dashboard/env.sh`:

```bash
#!/bin/bash
export DB_HOST=localhost
export DB_PORT=5432
export DB_NAME=sentinel
export DB_USER=sentinel
export DB_PASSWORD="your-secure-password"
export SENTINEL_API_KEY="$(openssl rand -hex 32)"

echo "API Key: $SENTINEL_API_KEY"
echo "Save this key - you'll need it for agent configuration"
```

```bash
chmod 600 /opt/sentinel-dashboard/env.sh
source /opt/sentinel-dashboard/env.sh
```

### 4. Start the Dashboard

```bash
cd /opt/sentinel-dashboard
source env.sh
gunicorn -b 127.0.0.1:5000 -w 2 app:app &
```

To run as a background service, add to `/etc/inittab`:

```
sentinel:2:respawn:/opt/sentinel-dashboard/start.sh
```

Or create a simple start script:

```bash
cat > /opt/sentinel-dashboard/start.sh << 'EOF'
#!/bin/bash
cd /opt/sentinel-dashboard
source env.sh
exec /opt/freeware/bin/gunicorn -b 127.0.0.1:5000 -w 2 app:app
EOF
chmod 755 /opt/sentinel-dashboard/start.sh
```

### 5. Configure Nginx Reverse Proxy

The package installs a config at `/opt/freeware/etc/nginx/conf.d/sentinel.conf`.

Edit it to match your hostname:

```bash
vi /opt/freeware/etc/nginx/conf.d/sentinel.conf
```

Then start/reload nginx:

```bash
startsrc -s nginx
# Or if already running:
refresh -s nginx
```

### 6. Access the Dashboard

Open in your browser:
- **Local**: http://localhost:5000
- **Via nginx**: http://your-aix-host/

## Configure Agents

On each monitored AIX host, add to crontab:

```bash
crontab -e
```

```cron
# Send fingerprint every 5 minutes
*/5 * * * * /opt/freeware/bin/sentinel -j -n 2>/dev/null | curl -s -X POST \
  -H "Content-Type: application/json" \
  -H "X-API-Key: YOUR_API_KEY" \
  -d @- http://dashboard-host:5000/api/ingest >/dev/null 2>&1
```

> **Note:** AIX does not support GNU-style long options. Use `-j -n` instead of `--json --network`.

## API Reference

| Endpoint | Method | Auth | Description |
|----------|--------|------|-------------|
| `/api/ingest` | POST | API Key | Receive fingerprint from agents |
| `/api/hosts` | GET | - | List all hosts with latest stats |
| `/api/hosts/<hostname>` | GET | - | Get host details and history |
| `/api/hosts/<hostname>/latest` | GET | - | Get latest full fingerprint |
| `/api/stats` | GET | - | Overall fleet statistics |
| `/health` | GET | - | Health check endpoint |

## Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `DB_HOST` | localhost | PostgreSQL host |
| `DB_PORT` | 5432 | PostgreSQL port |
| `DB_NAME` | sentinel | Database name |
| `DB_USER` | sentinel | Database user |
| `DB_PASSWORD` | (required) | Database password |
| `SENTINEL_API_KEY` | (required) | API key for agent ingestion |

## Package Contents

```
/opt/sentinel-dashboard/
├── app.py                 # Flask application
├── migrate.sql            # Database schema
├── requirements.txt       # Python dependencies
└── templates/
    ├── base.html          # Base template
    ├── index.html         # Dashboard home
    ├── host.html          # Host detail view
    ├── login.html         # Authentication
    ├── profile.html       # User profile
    ├── 2fa.html           # 2FA setup
    ├── api_keys.html      # API key management
    └── admin/
        ├── users.html     # User management
        ├── sessions.html  # Active sessions
        └── audit.html     # Audit log

/opt/freeware/etc/nginx/conf.d/
└── sentinel.conf          # Nginx reverse proxy config

/opt/freeware/libexec/sentinel/
└── install-dashboard.sh   # Setup helper script
```

## Architecture

```
┌─────────────────────────────────────────┐
│           AIX Dashboard Host            │
│                                         │
│  ┌─────────────┐    ┌───────────────┐   │
│  │   Nginx     │───▶│   Gunicorn    │   │
│  │  (port 80)  │    │  (port 5000)  │   │
│  └─────────────┘    └───────┬───────┘   │
│                             │           │
│                     ┌───────▼───────┐   │
│                     │  PostgreSQL   │   │
│                     │   (JSONB)     │   │
│                     └───────────────┘   │
└─────────────────────────────────────────┘
                      ▲
        ┌─────────────┼─────────────┐
        │             │             │
   ┌────┴────┐  ┌────┴────┐  ┌────┴────┐
   │  AIX 1  │  │  AIX 2  │  │  AIX N  │
   │sentinel │  │sentinel │  │sentinel │
   └─────────┘  └─────────┘  └─────────┘
```

## Troubleshooting

### Dashboard won't start

```bash
# Check if port 5000 is in use
netstat -an | grep 5000

# Check PostgreSQL connection
/opt/freeware/bin/psql -h localhost -U sentinel -d sentinel -c "SELECT 1"

# Check logs
tail -f /var/log/sentinel-dashboard.log
```

### Agents not reporting

```bash
# Test API endpoint
/opt/freeware/bin/curl -X POST -H "Content-Type: application/json" \
  -H "X-API-Key: YOUR_KEY" \
  -d '{"hostname":"test"}' \
  http://dashboard:5000/api/ingest

# Check cron is running
ps -ef | grep cron
```

### Database connection errors

```bash
# Verify PostgreSQL is running
lssrc -s postgresql

# Check pg_hba.conf allows local connections
cat /var/lib/pgsql/data/pg_hba.conf
```

## Related Packages

- **csentinel4aix** - The C-Sentinel agent that collects system fingerprints
- **postgresql18-server** - PostgreSQL database (IBM AIX Toolbox)
- **nginx** - Web server/reverse proxy (IBM AIX Toolbox)
- **python3** - Python runtime (IBM AIX Toolbox)

## License

MIT License

## Credits

- Part of the [C-Sentinel](../README.md) monitoring suite
- Developed by [LibrePower](https://librepower.org)
- Built for AIX sysadmins who need fleet visibility
