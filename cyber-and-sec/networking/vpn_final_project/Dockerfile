FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential libssl-dev iproute2 iptables iputils-ping tcpdump \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN make clean || true
RUN make
