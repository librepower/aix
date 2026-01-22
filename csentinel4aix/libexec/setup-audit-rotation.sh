#!/bin/sh
# Setup audit rotation - run interactively after install
CRONTAB=/var/spool/cron/crontabs/root
SCRIPT=/opt/freeware/libexec/sentinel/audit-rotate.sh

if grep -q "sentinel/audit-rotate" "$CRONTAB" 2>/dev/null; then
    echo "Audit rotation already configured."
    exit 0
fi

echo ""
echo "=== C-Sentinel Audit Rotation Setup ==="
echo ""
echo "AIX audit trail (/audit/trail) can grow indefinitely and fill disk."
echo "This will add a daily cron job to rotate when trail exceeds 100MB."
echo ""
printf "Enable audit trail rotation? [y/N] "
read answer
case "$answer" in
    [Yy]*)
        echo "# C-Sentinel audit trail rotation (daily at 3am)" >> "$CRONTAB"
        echo "0 3 * * * $SCRIPT >/dev/null 2>&1" >> "$CRONTAB"
        echo "Done. Rotation enabled (daily at 3am, max 100MB, keeps 4 backups)"
        ;;
    *)
        echo "Skipped. Run this script later if needed."
        ;;
esac
