FROM debian:bookworm-slim AS builder

RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    libpng-dev \
    curl \
    git \
    cmake \
    build-essential \
    make \
    && ln -s /usr/bin/python3 /usr/bin/python \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

WORKDIR /badge2025
COPY . .

RUN bash run_cmake_wasm.sh
RUN cd web && make build-user-docs

FROM golang:alpine AS server

WORKDIR /app

COPY --from=builder /badge2025/tools/wasm_serve.go .
COPY --from=builder /badge2025/web/docs/user_docs/.book/ ./static/

RUN go build -ldflags="-s -w" -o wasm_serve wasm_serve.go

FROM alpine:latest

WORKDIR /app

COPY --from=server /app/wasm_serve .
COPY --from=server /app/static ./static

EXPOSE 8080
CMD ["./wasm_serve", "--dir=./static", "--port=8080"]
