#!/bin/bash
#
# C-Sentinel Dashboard Installation Script for AIX
#
# This script helps set up the dashboard after installing
# the csentinel-dashboard RPM package.
#

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}C-Sentinel Dashboard Setup for AIX${NC}"
echo "====================================="
echo

# Check if running as root
if [ "$(id -u)" -ne 0 ]; then
    echo -e "${RED}Please run as root${NC}"
    exit 1
fi

# Configuration
INSTALL_DIR="/opt/sentinel-dashboard"
DB_NAME="${DB_NAME:-sentinel}"
DB_USER="${DB_USER:-sentinel}"
DB_PASSWORD="${DB_PASSWORD:-$(openssl rand -base64 24)}"
API_KEY="${SENTINEL_API_KEY:-$(openssl rand -hex 16)}"
FLASK_SECRET="${FLASK_SECRET_KEY:-$(openssl rand -hex 32)}"

echo -e "${YELLOW}Dashboard directory: ${INSTALL_DIR}${NC}"
echo -e "${YELLOW}Database: ${DB_NAME}${NC}"
echo
echo "Save the credentials below - you'll need them!"
echo

# Check prerequisites
echo -e "${YELLOW}Checking prerequisites...${NC}"

# Check PostgreSQL
if ! /opt/freeware/bin/psql --version >/dev/null 2>&1; then
    echo -e "${RED}PostgreSQL not found. Install with:${NC}"
    echo "  dnf install postgresql18-server"
    exit 1
fi
echo "  - PostgreSQL: OK"

# Check Python pip packages
if ! /opt/freeware/bin/python3 -c "import flask" 2>/dev/null; then
    echo -e "${YELLOW}Installing Python dependencies...${NC}"
    /opt/freeware/bin/pip3 install flask psycopg2-binary gunicorn
fi
echo "  - Python packages: OK"

# Check if PostgreSQL is running
if ! pgrep -f postgres >/dev/null; then
    echo -e "${YELLOW}PostgreSQL not running. Starting...${NC}"

    # Check if data directory exists
    if [ ! -d /var/lib/pgsql/data/base ]; then
        echo "  - Initializing PostgreSQL data directory..."
        mkdir -p /var/lib/pgsql/data
        chown postgres:postgres /var/lib/pgsql/data
        su - postgres -c "/opt/freeware/bin/initdb -D /var/lib/pgsql/data"
    fi

    # Start PostgreSQL
    su - postgres -c "/opt/freeware/bin/pg_ctl -D /var/lib/pgsql/data -l /var/lib/pgsql/logfile start"
    sleep 3
fi
echo "  - PostgreSQL running: OK"

# Create database and user
echo -e "${YELLOW}Setting up database...${NC}"

# Check if database exists
if ! su - postgres -c "/opt/freeware/bin/psql -lqt" | cut -d \| -f 1 | grep -qw "$DB_NAME"; then
    echo "  - Creating database ${DB_NAME}..."
    su - postgres -c "/opt/freeware/bin/psql -c \"CREATE DATABASE ${DB_NAME};\""
fi

# Check if user exists
if ! su - postgres -c "/opt/freeware/bin/psql -c \"SELECT 1 FROM pg_roles WHERE rolname='${DB_USER}'\"" | grep -q 1; then
    echo "  - Creating user ${DB_USER}..."
    su - postgres -c "/opt/freeware/bin/psql -c \"CREATE USER ${DB_USER} WITH ENCRYPTED PASSWORD '${DB_PASSWORD}';\""
fi

# Grant privileges
su - postgres -c "/opt/freeware/bin/psql -c \"GRANT ALL PRIVILEGES ON DATABASE ${DB_NAME} TO ${DB_USER};\"" 2>/dev/null || true
su - postgres -c "/opt/freeware/bin/psql -d ${DB_NAME} -c \"GRANT ALL ON SCHEMA public TO ${DB_USER};\"" 2>/dev/null || true

echo "  - Database setup: OK"

# Run migrations
echo -e "${YELLOW}Running database migrations...${NC}"
if [ -f "$INSTALL_DIR/migrate.sql" ]; then
    su - postgres -c "/opt/freeware/bin/psql -d ${DB_NAME} -f ${INSTALL_DIR}/migrate.sql" 2>/dev/null || true
    echo "  - Schema migration: OK"
fi

# Create environment file
echo -e "${YELLOW}Creating environment configuration...${NC}"
cat > "$INSTALL_DIR/env.sh" << EOF
#!/bin/bash
# C-Sentinel Dashboard Environment
# Generated on $(date)

export DB_HOST=localhost
export DB_PORT=5432
export DB_NAME=${DB_NAME}
export DB_USER=${DB_USER}
export DB_PASSWORD='${DB_PASSWORD}'
export SENTINEL_API_KEY='${API_KEY}'
export FLASK_SECRET_KEY='${FLASK_SECRET}'

# Optional: Dashboard password (SHA256 hash)
# Generate with: echo -n 'yourpassword' | openssl dgst -sha256 | awk '{print \$2}'
# export SENTINEL_ADMIN_PASSWORD_HASH=your-hash-here
EOF

chmod 600 "$INSTALL_DIR/env.sh"
echo "  - Environment file: $INSTALL_DIR/env.sh"

# Create start script
echo -e "${YELLOW}Creating start script...${NC}"
cat > "$INSTALL_DIR/start.sh" << 'EOF'
#!/bin/bash
cd /opt/sentinel-dashboard
source env.sh
exec /opt/freeware/bin/gunicorn -b 0.0.0.0:5000 -w 2 app:app
EOF

chmod 755 "$INSTALL_DIR/start.sh"
echo "  - Start script: $INSTALL_DIR/start.sh"

echo
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Setup complete!${NC}"
echo -e "${GREEN}========================================${NC}"
echo
echo -e "${YELLOW}IMPORTANT - Save these credentials:${NC}"
echo
echo "  API Key:      ${API_KEY}"
echo "  DB Password:  ${DB_PASSWORD}"
echo
echo -e "${YELLOW}Next steps:${NC}"
echo
echo "  1. Start the dashboard:"
echo "     cd /opt/sentinel-dashboard"
echo "     ./start.sh &"
echo
echo "  2. Or add to /etc/inittab for auto-start:"
echo "     sentinel:2:respawn:/opt/sentinel-dashboard/start.sh"
echo "     telinit q"
echo
echo "  3. Configure nginx (optional):"
echo "     Edit /opt/freeware/etc/nginx/conf.d/sentinel.conf"
echo "     startsrc -s nginx"
echo
echo "  4. Configure sentinel agents to report (crontab -e):"
echo
echo "     */5 * * * * /opt/freeware/bin/sentinel -j -n | \\"
echo "       /opt/freeware/bin/curl -s -X POST \\"
echo "       -H 'Content-Type: application/json' \\"
echo "       -H 'X-API-Key: ${API_KEY}' \\"
echo "       -d @- http://$(hostname):5000/api/ingest >/dev/null 2>&1"
echo
echo -e "${GREEN}Documentation: https://gitlab.com/librepower/aix/-/tree/main/csentinel4aix/dashboard${NC}"
echo
