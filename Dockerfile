# Multi-stage build for Network Traffic Shaping and QoS Simulator
FROM gcc:13.2 AS builder

# Set working directory
WORKDIR /app

# Copy source files
COPY include/ ./include/
COPY src/ ./src/

# Compile the C++ application
RUN g++ -std=c++17 -pthread -O3 \
    -o network_simulator \
    src/main.cpp \
    -Iinclude

# Runtime stage with Python for visualization
FROM python:3.11-slim

# Install required system packages
RUN apt-get update && apt-get install -y \
    libgomp1 \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy compiled binary from builder
COPY --from=builder /app/network_simulator .

# Copy Python visualization script
COPY scripts/ ./scripts/
COPY requirements.txt .

# Install Python dependencies
RUN pip install --no-cache-dir --upgrade pip && \
    pip install --no-cache-dir -r requirements.txt

# Create results directory
RUN mkdir -p results

# Default command: run simulator then visualize
CMD ["sh", "-c", "./network_simulator && python scripts/visualize.py"]
