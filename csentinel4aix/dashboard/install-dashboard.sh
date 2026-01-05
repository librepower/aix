#!/bin/bash
#
# C-Sentinel Dashboard Installation Script
#

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}C-Sentinel Dashboard Installer${NC}"
echo "================================="
echo

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}Please run as root (sudo ./install-dashboard.sh)${NC}"
    exit 1
fi

# Configuration
INSTALL_DIR="/opt/sentinel-dashboard"
DB_NAME="${DB_NAME:-sentinel}"
DB_USER="${DB_USER:-sentinel}"
DB_PASSWORD="${DB_PASSWORD:-$(openssl rand -base64 24)}"
API_KEY="${API_KEY:-$(openssl rand -hex 16)}"
FLASK_SECRET="${FLASK_SECRET:-$(openssl rand -hex 32)}"

echo -e "${YELLOW}Installing to: ${INSTALL_DIR}${NC}"
echo -e "${YELLOW}Database: ${DB_NAME}${NC}"
echo
echo "Save the credentials below - you'll need them!"
echo

# Create installation directory
echo -e "${YELLOW}Creating installation directory...${NC}"
mkdir -p "$INSTALL_DIR"
cp -r . "$INSTALL_DIR/"
cd "$INSTALL_DIR"

# Create virtual environment
echo -e "${YELLOW}Creating Python virtual environment...${NC}"
python3 -m venv venv
./venv/bin/pip install --upgrade pip
./venv/bin/pip install -r requirements.txt

# Create .env file
echo -e "${YELLOW}Creating environment configuration...${NC}"
cat > "$INSTALL_DIR/.env" << EOF
# C-Sentinel Dashboard Configuration
# Generated on $(date)

# Database
DB_HOST=localhost
DB_PORT=5432
DB_NAME=${DB_NAME}
DB_USER=${DB_USER}
DB_PASSWORD=${DB_PASSWORD}

# API Authentication (for sentinel agents)
SENTINEL_API_KEY=${API_KEY}

# Flask Session Security
FLASK_SECRET_KEY=${FLASK_SECRET}

# Dashboard Authentication
# Generate hash with: echo -n 'yourpassword' | sha256sum | cut -d' ' -f1
# Then uncomment and set:
# SENTINEL_ADMIN_PASSWORD_HASH=your-sha256-hash-here

# Email Alerts (optional - set ALERT_EMAIL_ENABLED=true to enable)
ALERT_EMAIL_ENABLED=false
# ALERT_SMTP_HOST=smtp.gmail.com
# ALERT_SMTP_PORT=587
# ALERT_SMTP_USER=your-email@gmail.com
# ALERT_SMTP_PASS=your-app-password
# ALERT_FROM=your-email@gmail.com
# ALERT_TO=alerts@your-domain.com
# ALERT_COOLDOWN_MINS=60
EOF

chmod 600 "$INSTALL_DIR/.env"

# Update service file with generated secrets
echo -e "${YELLOW}Configuring systemd service...${NC}"
sed -i "s/CHANGE_ME_generate_with_secrets_token_hex_32/${FLASK_SECRET}/g" sentinel-dashboard.service

# Install systemd service
cp sentinel-dashboard.service /etc/systemd/system/
systemctl daemon-reload

# Install nginx config if present
echo -e "${YELLOW}Checking for nginx configuration...${NC}"
if [ -f nginx-sentinel.conf ]; then
    echo "  - Installing nginx config..."
    cp nginx-sentinel.conf /etc/nginx/sites-available/sentinel
    ln -sf /etc/nginx/sites-available/sentinel /etc/nginx/sites-enabled/
    nginx -t && echo -e "${GREEN}  - nginx config valid${NC}"
else
    echo "  - No nginx config found (nginx-sentinel.conf)"
    echo "  - You'll need to configure your web server manually"
fi

# Initialize database schema
echo -e "${YELLOW}Initializing database schema...${NC}"
cd "$INSTALL_DIR"
DB_HOST=localhost DB_PORT=5432 DB_NAME="$DB_NAME" DB_USER="$DB_USER" DB_PASSWORD="$DB_PASSWORD" \
    ./venv/bin/python -c "from app import init_db; init_db()"

# Run database migrations
echo -e "${YELLOW}Running database migrations...${NC}"

if [ -f migrate_audit_totals.sql ]; then
    echo "  - Applying audit totals migration..."
    sudo -u postgres psql -d "$DB_NAME" -f "$INSTALL_DIR/migrate_audit_totals.sql" 2>/dev/null || true
fi

if [ -f migrate_phase2.sql ]; then
    echo "  - Applying phase 2 migration (event history, sessions)..."
    sudo -u postgres psql -d "$DB_NAME" -f "$INSTALL_DIR/migrate_phase2.sql" 2>/dev/null || true
fi

echo -e "${GREEN}Database migrations complete!${NC}"

echo
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Installation complete!${NC}"
echo -e "${GREEN}========================================${NC}"
echo
echo -e "${YELLOW}IMPORTANT - Save these credentials:${NC}"
echo
echo "  API Key:      ${API_KEY}"
echo "  Flask Secret: ${FLASK_SECRET}"
echo "  DB Password:  ${DB_PASSWORD}"
echo
echo -e "${YELLOW}Next steps:${NC}"
echo
echo "  1. Set dashboard password:"
echo "     echo -n 'yourpassword' | sha256sum | cut -d' ' -f1"
echo "     Edit /etc/systemd/system/sentinel-dashboard.service"
echo "     Set SENTINEL_ADMIN_PASSWORD_HASH to the hash"
echo
echo "  2. Start the dashboard:"
echo "     sudo systemctl daemon-reload"
echo "     sudo systemctl enable sentinel-dashboard"
echo "     sudo systemctl start sentinel-dashboard"
echo
echo "  3. Configure web server (nginx/apache) with SSL"
echo "     sudo certbot --nginx -d your-domain.com"
echo
echo "  4. Configure sentinel agent to report:"
echo "     Add to crontab (crontab -e):"
echo
echo "     */5 * * * * sudo /usr/local/bin/sentinel --json --network --audit | \\"
echo "       curl -s -X POST -H 'Content-Type: application/json' \\"
echo "       -H 'X-API-Key: ${API_KEY}' \\"
echo "       -d @- https://your-domain.com/api/ingest"
echo
echo "  5. (Optional) Configure email alerts:"
echo "     Edit /etc/systemd/system/sentinel-dashboard.service"
echo "     Set ALERT_EMAIL_ENABLED=true and configure SMTP settings"
echo "     sudo systemctl daemon-reload && sudo systemctl restart sentinel-dashboard"
echo
echo -e "${GREEN}Documentation: https://github.com/williamofai/c-sentinel${NC}"
echo
