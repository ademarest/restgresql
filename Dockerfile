# Build Stage
FROM alpine:latest AS build

# Install build dependencies
RUN apk add --no-cache \
    build-base \
    cmake \
    git \
    openssl-dev \
    linux-headers \
    postgresql-dev \
    python3

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build the project and run unit tests
RUN mkdir build && \
    cd build && \
    cmake .. && \
    make -j$(nproc) && \
    ctest --output-on-failure

# Runtime Stage
FROM alpine:latest

# Install runtime dependencies
RUN apk add --no-cache \
    libssl3 \
    libstdc++ \
    postgresql-libs

# Set working directory
WORKDIR /app

# Copy the binary from the build stage
COPY --from=build /app/build/restgresql .

# Create a default config directory
RUN mkdir -p /etc/restgresql

# Expose the port
EXPOSE 8000

# Run the server
ENTRYPOINT ["./restgresql"]
