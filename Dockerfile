# Multi-stage: build sd.cpp (optional) and serve FastAPI UI

FROM ubuntu:22.04 AS base
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    python3 python3-pip python3-venv \
    libvulkan1 vulkan-tools \
    ca-certificates curl git && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Install web runtime
COPY web/requirements.txt /app/web/requirements.txt
RUN python3 -m venv /venv && /venv/bin/pip install --no-cache-dir -r /app/web/requirements.txt

# Copy app
COPY web /app/web

# Expose models/outputs folders mount points
RUN mkdir -p /app/models /app/outputs
VOLUME ["/app/models", "/app/outputs"]

# Copy sd binaries if provided at build-time (optional). Otherwise expect to mount or use PATH.
# Placeholders for sd / sd-cpu binaries
COPY --chmod=755 resources/neural_pixel.bat /dev/null || true

ENV PYTHONUNBUFFERED=1 \
    MODELS_DIR=/app/models \
    OUTPUTS_DIR=/app/outputs

WORKDIR /app

CMD ["/venv/bin/uvicorn", "web.app.main:app", "--host", "0.0.0.0", "--port", "7860"]
