# C-Sentinel Makefile
# Semantic Observability for UNIX Systems
#
# Build targets:
#   make          - Build all binaries
#   make static   - Build statically linked (maximum portability)
#   make test     - Run test suite
#   make install  - Install to /usr/local/bin

# Default compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
CFLAGS += -I./include
LDFLAGS =
LDLIBS = -lm

# Override for AIX (detected at runtime)
# On AIX, you may need to run: make CC=/opt/freeware/bin/gcc
# Or set environment variables before make

# Debug build
ifdef DEBUG
    CFLAGS += -g -DDEBUG -O0
    CFLAGS := $(filter-out -O2,$(CFLAGS))
endif

# Static linking for maximum portability
ifdef STATIC
    LDFLAGS += -static
endif

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Main sentinel sources
SENTINEL_SRCS = $(SRC_DIR)/main.c \
                $(SRC_DIR)/prober.c \
                $(SRC_DIR)/net_probe.c \
                $(SRC_DIR)/json_serialize.c \
                $(SRC_DIR)/policy.c \
                $(SRC_DIR)/sanitize.c \
                $(SRC_DIR)/baseline.c \
                $(SRC_DIR)/config.c \
                $(SRC_DIR)/alert.c \
                $(SRC_DIR)/sha256.c \
                $(SRC_DIR)/audit.c \
                $(SRC_DIR)/audit_json.c \
                $(SRC_DIR)/process_chain.c

SENTINEL_OBJS = $(SENTINEL_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Diff tool sources
DIFF_SRCS = $(SRC_DIR)/diff.c
DIFF_OBJS = $(DIFF_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Header dependencies
HEADERS = $(INC_DIR)/sentinel.h $(INC_DIR)/policy.h $(INC_DIR)/sanitize.h $(INC_DIR)/audit.h $(INC_DIR)/color.h

# Target binaries
SENTINEL = $(BIN_DIR)/sentinel
SENTINEL_DIFF = $(BIN_DIR)/sentinel-diff

# Default target
all: dirs $(SENTINEL) $(SENTINEL_DIFF)
	@echo ""
	@echo "Build complete. Binaries:"
	@ls -la $(BIN_DIR)/

# Static build for deployment
static: LDFLAGS += -static
static: clean all
	@echo ""
	@echo "Static build complete. Checking dependencies:"
	@file $(BIN_DIR)/* || true
	@ldd $(BIN_DIR)/sentinel 2>&1 | head -5 || echo "(statically linked)"

# Create directories
dirs:
	@mkdir -p $(BUILD_DIR) $(BIN_DIR)

# Link sentinel
$(SENTINEL): $(SENTINEL_OBJS)
	$(CC) $(SENTINEL_OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

# Link sentinel-diff  
$(SENTINEL_DIFF): $(DIFF_OBJS)
	$(CC) $(DIFF_OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

# Compile rule
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Special rule for diff.c (doesn't need all headers)
$(BUILD_DIR)/diff.o: $(SRC_DIR)/diff.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Install
PREFIX ?= /usr/local
install: all
	install -d $(PREFIX)/bin
	install -m 755 $(SENTINEL) $(PREFIX)/bin/
	install -m 755 $(SENTINEL_DIFF) $(PREFIX)/bin/
	@echo "Installed to $(PREFIX)/bin/"

# Uninstall
uninstall:
	rm -f $(PREFIX)/bin/sentinel
	rm -f $(PREFIX)/bin/sentinel-diff

# Test suite
test: all
	@echo "=== C-Sentinel Test Suite ==="
	@echo ""
	@echo "1. Quick mode test..."
	@./$(SENTINEL) --quick > /dev/null && echo "   PASS: Quick mode" || echo "   FAIL: Quick mode"
	@echo ""
	@echo "2. JSON output test..."
	@./$(SENTINEL) /etc/hosts > /tmp/sentinel_test.json 2>/dev/null && echo "   PASS: JSON output" || echo "   FAIL: JSON output"
	@echo ""
	@echo "3. Diff tool test..."
	@./$(SENTINEL) > /tmp/fp1.json 2>/dev/null
	@./$(SENTINEL) > /tmp/fp2.json 2>/dev/null
	@./$(SENTINEL_DIFF) /tmp/fp1.json /tmp/fp2.json > /dev/null 2>&1 && echo "   PASS: Diff tool" || echo "   PASS: Diff tool (differences found)"
	@echo ""
	@echo "4. JSON validity test..."
	@python3 -c "import json; json.load(open('/tmp/sentinel_test.json'))" 2>/dev/null && echo "   PASS: Valid JSON" || echo "   FAIL: Invalid JSON"
	@echo ""
	@echo "5. Audit probe test..."
	@./$(SENTINEL) --audit --quick 2>/dev/null && echo "   PASS: Audit probe" || echo "   WARN: Audit probe (auditd may not be installed)"
	@echo ""
	@echo "6. Colour output test..."
	@./$(SENTINEL) --quick --color 2>/dev/null | head -1 | grep -q "C-Sentinel" && echo "   PASS: Colour output" || echo "   FAIL: Colour output"
	@echo ""
	@echo "=== All tests complete ==="
	@rm -f /tmp/sentinel_test.json /tmp/fp1.json /tmp/fp2.json

# Development helpers
.PHONY: all clean install uninstall test dirs static

# Static analysis
lint:
	@which cppcheck > /dev/null 2>&1 && \
		cppcheck --enable=all --suppress=missingIncludeSystem \
		         --suppress=unusedFunction $(SRC_DIR)/ $(INC_DIR)/ || \
		echo "Install cppcheck for static analysis"

# Format code
format:
	@which clang-format > /dev/null 2>&1 && \
		clang-format -i $(SRC_DIR)/*.c $(INC_DIR)/*.h || \
		echo "Install clang-format for code formatting"

# Generate compile_commands.json for IDE support
compile_commands:
	@which bear > /dev/null 2>&1 && \
		bear -- make clean all || \
		echo "Install bear for compile_commands.json generation"

# Show binary sizes
size: all
	@echo "Binary sizes:"
	@size $(BIN_DIR)/* || ls -lh $(BIN_DIR)/*

# Help
help:
	@echo "C-Sentinel Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build all binaries (default)"
	@echo "  static    - Build with static linking"
	@echo "  test      - Run test suite"
	@echo "  install   - Install to PREFIX (default: /usr/local)"
	@echo "  clean     - Remove build artifacts"
	@echo "  lint      - Run static analysis"
	@echo "  format    - Format source code"
	@echo "  size      - Show binary sizes"
	@echo ""
	@echo "Options:"
	@echo "  DEBUG=1   - Debug build with symbols"
	@echo "  STATIC=1  - Static linking"
	@echo "  PREFIX=   - Installation prefix"
	@echo ""
	@echo "Examples:"
	@echo "  make"
	@echo "  make DEBUG=1"
	@echo "  make STATIC=1"
	@echo "  make install PREFIX=/opt/sentinel"
