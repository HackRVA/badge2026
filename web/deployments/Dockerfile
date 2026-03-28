FROM golang:1.26-trixie AS builder

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
RUN cd web && go run ./cmd/generate/

FROM golang:alpine AS server

WORKDIR /app

COPY --from=builder /badge2025/web/cmd/server/ .
COPY --from=builder /badge2025/web/.dist/ ./static/

RUN go build -ldflags="-s -w" -o wasm_serve main.go 

FROM alpine:latest

WORKDIR /app

COPY --from=server /app/wasm_serve .
COPY --from=server /app/static ./static

EXPOSE 8080
CMD ["./wasm_serve", "--dir=./static", "--port=8080"]
