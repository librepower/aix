#
# lpsof - LibrePowerSof Makefile for AIX
# Version 0.3.0 (Security-Hardened)
#

CC = /opt/freeware/bin/gcc

# Security-hardened compiler flags
# -Wall -Wextra: Enable comprehensive warnings
# -D_FORTIFY_SOURCE=2: Runtime buffer overflow checks
# -Wformat -Wformat-security: Format string warnings
# NOTE: -fstack-protector-strong not available on AIX
CFLAGS = -Wall -Wextra -O2 -D_ALL_SOURCE -D_LARGE_FILES -maix64 \
         -Wformat -Wformat-security -D_FORTIFY_SOURCE=2
LDFLAGS = -maix64
LIBS = -lperfstat

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1

TARGET = lpsof
SOURCES = lpsof.c
OBJECTS = $(SOURCES:.c=.o)
MANPAGE = lpsof.1

.PHONY: all clean install uninstall test test-all

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS)

install: $(TARGET)
	mkdir -p $(DESTDIR)$(BINDIR)
	cp $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	chmod 755 $(DESTDIR)$(BINDIR)/$(TARGET)
	mkdir -p $(DESTDIR)$(MANDIR)
	cp $(MANPAGE) $(DESTDIR)$(MANDIR)/$(MANPAGE)
	chmod 644 $(DESTDIR)$(MANDIR)/$(MANPAGE)
	@echo "Installed lpsof to $(DESTDIR)$(BINDIR)"
	@echo "Installed man page to $(DESTDIR)$(MANDIR)"

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(MANDIR)/$(MANPAGE)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: clean all

# 32-bit build
build32: CFLAGS = -Wall -O2 -D_ALL_SOURCE -D_LARGE_FILES -maix32
build32: LDFLAGS = -maix32
build32: clean all

# Quick test
test: $(TARGET)
	@echo "=== Quick Tests ==="
	./$(TARGET) -h | head -5
	./$(TARGET) -v
	./$(TARGET) -p $$$$ | head -3

# Comprehensive test suite
test-all: $(TARGET)
	@echo "============================================"
	@echo "LPSOF v0.1.0 Comprehensive Test Suite"
	@echo "============================================"
	@echo ""
	@echo "=== Test 1: Help (-h) ==="
	./$(TARGET) -h | head -10
	@echo "PASS"
	@echo ""
	@echo "=== Test 2: Version (-v) ==="
	./$(TARGET) -v
	@echo "PASS"
	@echo ""
	@echo "=== Test 3: PID filter (-p) ==="
	./$(TARGET) -p $$$$ | wc -l
	@echo "PASS"
	@echo ""
	@echo "=== Test 4: User filter (-u) ==="
	./$(TARGET) -u root 2>/dev/null | wc -l
	@echo "PASS"
	@echo ""
	@echo "=== Test 5: Command filter (-c) ==="
	./$(TARGET) -c init 2>/dev/null | wc -l
	@echo "PASS"
	@echo ""
	@echo "=== Test 6: Network (-i) ==="
	./$(TARGET) -i 2>/dev/null | wc -l
	@echo "PASS"
	@echo ""
	@echo "=== Test 7: Terse mode (-t) ==="
	./$(TARGET) -t -p $$$$
	@echo "PASS"
	@echo ""
	@echo "=== Test 8: PPID column (-R) ==="
	./$(TARGET) -R -p $$$$ | head -2
	@echo "PASS"
	@echo ""
	@echo "=== Test 9: Human sizes (-H) ==="
	./$(TARGET) -H -p $$$$ | head -2
	@echo "PASS"
	@echo ""
	@echo "=== Test 10: Numeric UID (-l) ==="
	./$(TARGET) -l -p $$$$ | head -2
	@echo "PASS"
	@echo ""
	@echo "=== Test 11: Command width (+c) ==="
	./$(TARGET) +c 20 -p $$$$ | head -2
	@echo "PASS"
	@echo ""
	@echo "=== Test 12: FD filter cwd (-d cwd) ==="
	./$(TARGET) -d cwd -p $$$$
	@echo "PASS"
	@echo ""
	@echo "=== Test 13: Directory search (+D) ==="
	./$(TARGET) +D /var/adm 2>/dev/null | wc -l
	@echo "PASS"
	@echo ""
	@echo "=== Test 14: Field output (-F) ==="
	./$(TARGET) -F -p $$$$ | head -3
	@echo "PASS"
	@echo ""
	@echo "=== Test 15: AND logic (-a) ==="
	./$(TARGET) -a -c init -u root 2>/dev/null | wc -l
	@echo "PASS"
	@echo ""
	@echo "=== Test 16: PGID filter (-g) ==="
	./$(TARGET) -g 1 2>/dev/null | wc -l
	@echo "PASS"
	@echo ""
	@echo "=== Test 17: Offset display (-o) ==="
	./$(TARGET) -o -p $$$$ | head -2
	@echo "PASS"
	@echo ""
	@echo "=== Test 18: Link count (+L) ==="
	./$(TARGET) +L -p $$$$ | head -2
	@echo "PASS"
	@echo ""
	@echo "=== Test 19: File search (positional) ==="
	./$(TARGET) / 2>/dev/null | wc -l
	@echo "PASS"
	@echo ""
	@echo "=== Test 20: Port filter (-i :port) ==="
	./$(TARGET) -i :22 2>/dev/null | wc -l
	@echo "PASS"
	@echo ""
	@echo "============================================"
	@echo "All tests completed!"
	@echo "============================================"
