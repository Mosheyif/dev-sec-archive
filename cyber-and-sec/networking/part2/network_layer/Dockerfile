FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Unified layer for all nodes. Docker's cache makes deploying 3 containers instantaneous.
RUN apt-get update && apt-get install -y \
    iproute2 iptables iputils-ping net-tools python3 python3-scapy \
    wireshark tshark curl wget netcat-openbsd \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt/scripts
