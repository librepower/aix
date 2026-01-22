#!/bin/sh
# C-Sentinel Audit Trail Rotation
# Installed by csentinel4aix - safe to remove if not needed
#
# Rotates AIX audit trail to prevent disk fill
# Keeps last 4 rotated files, max 100MB before rotation

TRAIL=/audit/trail
MAX_SIZE=104857600  # 100MB
KEEP_FILES=4

# Only run if audit is active and trail exists
[ -f "$TRAIL" ] || exit 0

size=$(ls -l "$TRAIL" 2>/dev/null | awk '{print $5}')
[ -z "$size" ] && exit 0

if [ "$size" -gt "$MAX_SIZE" ]; then
    # Rotate
    cp "$TRAIL" "${TRAIL}.$(date +%Y%m%d-%H%M%S)"
    > "$TRAIL"
    
    # Cleanup old rotations
    ls -t ${TRAIL}.* 2>/dev/null | tail -n +$((KEEP_FILES + 1)) | xargs rm -f 2>/dev/null
    
    logger -t sentinel-audit "Rotated audit trail (was ${size} bytes)"
fi
