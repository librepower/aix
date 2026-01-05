# C-Sentinel Dockerfile
#
# Multi-stage build for minimal final image size.
# Stage 1: Build the C binaries
# Stage 2: Runtime with Python for orchestration

# Build stage
FROM gcc:12 AS builder

WORKDIR /build

# Copy source files
COPY include/ include/
COPY src/ src/
COPY Makefile .

# Build with static linking for maximum portability
RUN make STATIC=1

# Verify binaries
RUN ./bin/sentinel --quick && \
    echo "Build successful"

# Runtime stage
FROM python:3.11-slim

LABEL maintainer="C-Sentinel Project"
LABEL description="Semantic Observability for UNIX Systems"
LABEL version="0.1.0"

WORKDIR /app

# Install runtime dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        curl \
        procps \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages for LLM integration
RUN pip install --no-cache-dir \
    anthropic \
    openai

# Copy binaries from builder
COPY --from=builder /build/bin/sentinel /app/bin/sentinel
COPY --from=builder /build/bin/sentinel-diff /app/bin/sentinel-diff

# Copy Python wrapper and examples
COPY sentinel_analyze.py .
COPY examples/ examples/

# Make binaries executable
RUN chmod +x /app/bin/* /app/sentinel_analyze.py

# Add bin to PATH
ENV PATH="/app/bin:${PATH}"

# Default command shows help
CMD ["./sentinel_analyze.py", "--help"]
