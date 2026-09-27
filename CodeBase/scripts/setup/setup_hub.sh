#!/bin/bash
# ═══════════════════════════════════════════════════════════
# Project Ability — Raspberry Pi Hub Setup Script
# Run this on the Raspberry Pi 5
# Usage: chmod +x setup_hub.sh && sudo ./setup_hub.sh
# ═══════════════════════════════════════════════════════════

set -e

echo "═══════════════════════════════════════════════"
echo "  Project Ability — Hub Setup"
echo "  Setting up WiFi AP + MQTT Broker"
echo "═══════════════════════════════════════════════"
echo ""

AP_SSID="Ability"
AP_PASS="ability123"
AP_IP="192.168.4.1"
DHCP_START="192.168.4.10"
DHCP_END="192.168.4.50"

# ── Step 1: Update & Install ────────────────────────────
echo "[1/5] Installing packages..."
apt update -y
apt install -y mosquitto mosquitto-clients hostapd dnsmasq python3-pip

# ── Step 2: Configure Mosquitto MQTT Broker ─────────────
echo "[2/5] Configuring Mosquitto MQTT broker..."

cat > /etc/mosquitto/conf.d/ability.conf << EOF
# Project Ability — MQTT Broker Config
listener 1883
allow_anonymous true
max_connections 20
EOF

systemctl enable mosquitto
systemctl restart mosquitto
echo "  Mosquitto running on port 1883"

# ── Step 3: Configure WiFi Access Point ─────────────────
echo "[3/5] Configuring WiFi Access Point..."

# Stop services while configuring
systemctl stop hostapd 2>/dev/null || true
systemctl stop dnsmasq 2>/dev/null || true

# Static IP for wlan0
cat >> /etc/dhcpcd.conf << EOF

# Project Ability — WiFi AP
interface wlan0
    static ip_address=${AP_IP}/24
    nohook wpa_supplicant
EOF

# hostapd config
cat > /etc/hostapd/hostapd.conf << EOF
interface=wlan0
driver=nl80211
ssid=${AP_SSID}
hw_mode=g
channel=7
wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=${AP_PASS}
wpa_key_mgmt=WPA-PSK
rsn_pairwise=CCMP
EOF

# Point hostapd to config
sed -i 's|#DAEMON_CONF=""|DAEMON_CONF="/etc/hostapd/hostapd.conf"|' /etc/default/hostapd 2>/dev/null || true

# ── Step 4: Configure DHCP Server ──────────────────────
echo "[4/5] Configuring DHCP (dnsmasq)..."

# Backup original
mv /etc/dnsmasq.conf /etc/dnsmasq.conf.bak 2>/dev/null || true

cat > /etc/dnsmasq.conf << EOF
# Project Ability — DHCP Server
interface=wlan0
dhcp-range=${DHCP_START},${DHCP_END},255.255.255.0,24h
domain=ability.local
address=/ability.local/${AP_IP}

# Static IP assignments for modules
dhcp-host=braille_01,${DHCP_START}1
dhcp-host=glove_01,${DHCP_START}0
dhcp-host=voice_01,${DHCP_START}2
EOF

# ── Step 5: Enable & Start Services ────────────────────
echo "[5/5] Starting services..."

systemctl unmask hostapd
systemctl enable hostapd
systemctl enable dnsmasq

systemctl restart dhcpcd
sleep 2
systemctl restart hostapd
systemctl restart dnsmasq

# ── Install Python dependencies ────────────────────────
echo ""
echo "Installing Python dependencies..."
pip3 install paho-mqtt pyyaml --break-system-packages 2>/dev/null || pip3 install paho-mqtt pyyaml

# ── Done ───────────────────────────────────────────────
echo ""
echo "═══════════════════════════════════════════════"
echo "  SETUP COMPLETE!"
echo "═══════════════════════════════════════════════"
echo ""
echo "  WiFi AP:     ${AP_SSID}"
echo "  Password:    ${AP_PASS}"
echo "  Pi IP:       ${AP_IP}"
echo "  MQTT Port:   1883"
echo "  DHCP Range:  ${DHCP_START} - ${DHCP_END}"
echo ""
echo "  Next steps:"
echo "  1. Reboot: sudo reboot"
echo "  2. Connect phone to '${AP_SSID}' WiFi"
echo "  3. Test MQTT:"
echo "     Terminal 1: mosquitto_sub -t 'ability/#' -v"
echo "     Terminal 2: mosquitto_pub -t 'ability/test' -m 'hello'"
echo "  4. Run Hub Engine:"
echo "     python3 hub/core/main.py"
echo ""
