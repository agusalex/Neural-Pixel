# Multi-stage: build sd.cpp (Vulkan and CPU) and serve FastAPI UI

FROM ubuntu:22.04 AS sd-builder
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git ca-certificates \
    libvulkan1 libvulkan-dev vulkan-tools glslang-tools && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /src

# Clone stable-diffusion.cpp with submodules
RUN git clone --depth 1 https://github.com/leejet/stable-diffusion.cpp . && \
    git submodule update --init --recursive

# Build Vulkan-enabled sd binary
RUN cmake -S . -B build-vk -DGGML_VULKAN=ON -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build-vk -j && \
    install -m 0755 $(find build-vk -type f -name sd -print -quit) /tmp/sd

# Build CPU-only sd binary (as sd-cpu)
RUN cmake -S . -B build-cpu -DGGML_VULKAN=OFF -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build-cpu -j && \
    install -m 0755 $(find build-cpu -type f -name sd -print -quit) /tmp/sd-cpu


FROM ubuntu:22.04 AS base
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    python3 python3-pip python3-venv \
    libvulkan1 vulkan-tools \
    ca-certificates curl && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Install web runtime
COPY web/requirements.txt /app/web/requirements.txt
RUN python3 -m venv /venv && /venv/bin/pip install --no-cache-dir -r /app/web/requirements.txt

# Copy app
COPY web /app/web

# Include sd binaries built in previous stage (available on PATH and at /app/*)
COPY --from=sd-builder /tmp/sd /usr/local/bin/sd
COPY --from=sd-builder /tmp/sd /app/sd
COPY --from=sd-builder /tmp/sd-cpu /usr/local/bin/sd-cpu
COPY --from=sd-builder /tmp/sd-cpu /app/sd-cpu

# Expose models/outputs folders mount points
RUN mkdir -p /app/models /app/outputs
VOLUME ["/app/models", "/app/outputs"]

ENV PYTHONUNBUFFERED=1 \
    MODELS_DIR=/app/models \
    OUTPUTS_DIR=/app/outputs

WORKDIR /app

CMD ["/venv/bin/uvicorn", "web.app.main:app", "--host", "0.0.0.0", "--port", "7860"]
