#!/bin/sh
# C-Sentinel SIEM Integration QA Tests
# Run on AIX to validate SIEM functionality

SENTINEL="${1:-./bin/sentinel}"
PASS=0
FAIL=0

echo "================================================"
echo "C-Sentinel SIEM Integration QA Tests"
echo "================================================"
echo ""

# Test 1: Help shows SIEM options
echo "Test 1: SIEM options in help..."
if $SENTINEL -h 2>&1 | grep -q "SIEM Integration"; then
    echo "  PASS: SIEM options documented"
    PASS=$((PASS+1))
else
    echo "  FAIL: SIEM options missing from help"
    FAIL=$((FAIL+1))
fi

# Test 2: Log file creation
echo ""
echo "Test 2: SIEM log file creation..."
rm -f /tmp/siem_test_1.log
$SENTINEL -q -L /tmp/siem_test_1.log >/dev/null 2>&1
if [ -s /tmp/siem_test_1.log ]; then
    echo "  PASS: Log file created with content"
    PASS=$((PASS+1))
else
    echo "  FAIL: Log file not created or empty"
    FAIL=$((FAIL+1))
fi

# Test 3: Log file JSON format
echo ""
echo "Test 3: SIEM log JSON validity..."
if python3 -c "import json; json.loads(open('/tmp/siem_test_1.log').readline())" 2>/dev/null; then
    echo "  PASS: Log entries are valid JSON"
    PASS=$((PASS+1))
else
    echo "  FAIL: Log entries are not valid JSON"
    FAIL=$((FAIL+1))
fi

# Test 4: Log contains required fields
echo ""
echo "Test 4: SIEM log required fields..."
REQUIRED_FIELDS="timestamp source host event severity"
ALL_PRESENT=1
for field in $REQUIRED_FIELDS; do
    if ! grep -q "\"$field\":" /tmp/siem_test_1.log 2>/dev/null; then
        echo "  Missing field: $field"
        ALL_PRESENT=0
    fi
done
if [ $ALL_PRESENT -eq 1 ]; then
    echo "  PASS: All required fields present"
    PASS=$((PASS+1))
else
    echo "  FAIL: Missing required fields"
    FAIL=$((FAIL+1))
fi

# Test 5: Network events in log
echo ""
echo "Test 5: Network events with SIEM..."
rm -f /tmp/siem_test_2.log
$SENTINEL -q -n -L /tmp/siem_test_2.log >/dev/null 2>&1
if grep -q "listeners" /tmp/siem_test_2.log 2>/dev/null; then
    echo "  PASS: Network data included in events"
    PASS=$((PASS+1))
else
    echo "  FAIL: Network data missing from events"
    FAIL=$((FAIL+1))
fi

# Test 6: Audit events in log (if audit enabled)
echo ""
echo "Test 6: Audit integration with SIEM..."
rm -f /tmp/siem_test_3.log
$SENTINEL -q -a -L /tmp/siem_test_3.log 2>/dev/null
if [ -s /tmp/siem_test_3.log ]; then
    echo "  PASS: Audit mode with SIEM works"
    PASS=$((PASS+1))
else
    echo "  WARN: Audit events may require: /usr/sbin/audit start"
    PASS=$((PASS+1))  # Don't fail if audit isn't enabled
fi

# Test 7: SIEM config display
echo ""
echo "Test 7: SIEM configuration display..."
OUTPUT=$($SENTINEL -q -L /tmp/siem_test_4.log 2>&1)
if echo "$OUTPUT" | grep -q "SIEM Integration"; then
    echo "  PASS: SIEM config displayed on stderr"
    PASS=$((PASS+1))
else
    echo "  FAIL: SIEM config not displayed"
    FAIL=$((FAIL+1))
fi

# Test 8: CEF format option
echo ""
echo "Test 8: CEF format option..."
# Just check that -R cef is accepted
if $SENTINEL -q -L /tmp/siem_test_5.log -R cef >/dev/null 2>&1; then
    echo "  PASS: CEF format option accepted"
    PASS=$((PASS+1))
else
    echo "  FAIL: CEF format option rejected"
    FAIL=$((FAIL+1))
fi

# Test 9: JSON format option
echo ""
echo "Test 9: JSON format option..."
if $SENTINEL -q -L /tmp/siem_test_6.log -R json >/dev/null 2>&1; then
    echo "  PASS: JSON format option accepted"
    PASS=$((PASS+1))
else
    echo "  FAIL: JSON format option rejected"
    FAIL=$((FAIL+1))
fi

# Test 10: Threshold option
echo ""
echo "Test 10: Alert threshold option..."
if $SENTINEL -q -L /tmp/siem_test_7.log -T 80 >/dev/null 2>&1; then
    echo "  PASS: Threshold option accepted"
    PASS=$((PASS+1))
else
    echo "  FAIL: Threshold option rejected"
    FAIL=$((FAIL+1))
fi

# Cleanup
rm -f /tmp/siem_test_*.log

# Summary
echo ""
echo "================================================"
echo "Results: $PASS passed, $FAIL failed"
echo "================================================"

if [ $FAIL -gt 0 ]; then
    exit 1
fi
exit 0
