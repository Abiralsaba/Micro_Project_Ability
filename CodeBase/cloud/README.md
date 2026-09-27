# Cloud Relay — Global Message Routing

> **Infrastructure:** Mosquitto MQTT Broker on VPS
> **Cost:** ~$5–10/month

## What the Cloud Relay Does

Routes **encrypted** text messages between Central Hubs worldwide. It **never reads, processes, or stores** message content (end-to-end encryption — only the sender and receiver hubs can decrypt).

## Setup

1. Provision a VPS (1 vCPU, 1GB RAM — DigitalOcean, AWS, Vultr)
2. Install Mosquitto MQTT Broker v5.0
3. Configure TLS 1.3 with Let's Encrypt certificates
4. Configure authentication and ACLs
5. Open port 8883 (MQTT over TLS)

## Security Stack

| Layer | Technology | Purpose |
|---|---|---|
| Transport | TLS 1.3 | Encrypts the MQTT connection itself |
| Payload | AES-256-GCM (E2EE) | Encrypts message content — broker never sees plaintext |
| Authentication | Username/password + client certificates | Only authorized hubs can connect |
| Authorization | ACLs | Hubs can only publish/subscribe to their own topics |
| Delivery | QoS Level 2 | Exactly-once message delivery guarantee |
