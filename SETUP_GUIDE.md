# Project Ability — Complete Setup Guide

> **Goal:** Get Glove → Pi → Braille working end-to-end.
> A deaf user signs letters with the glove → Pi routes the text → Braille module displays it.

---

## What You Need

| # | Device | Role | You Have It? |
|---|--------|------|---|
| 1 | **Raspberry Pi 5** (4GB or 8GB) | Central Hub — WiFi AP + MQTT broker + routing | |
| 2 | **ESP32-WROOM-32** | Braille Module — 8 servos, 4 braille cells | |
| 3 | **ESP32-S3 Super Mini** | Glove Module — 5 flex sensors + BNO055 IMU | |
| 4 | **MicroSD card** (32GB+) | Pi OS | |
| 5 | **USB-C power supply** for Pi | 5V 3A+ | |
| 6 | **USB cables** | For flashing ESP32s from your laptop | |

### Software on Your Laptop
- **Arduino IDE 2.x** — for flashing ESP32 firmware
- **Raspberry Pi Imager** — for installing Pi OS
- (Optional) **VS Code** — for editing Pi Python code

---

## Phase 1: Set Up the Raspberry Pi

### Step 1.1 — Flash Raspberry Pi OS

1. Download [Raspberry Pi Imager](https://www.raspberrypi.com/software/)
2. Insert your MicroSD card
3. In Imager:
   - **OS:** Raspberry Pi OS (64-bit) — the full desktop version
   - **Storage:** Your MicroSD card
   - Click the **gear icon** and set:
     - Enable SSH
     - Set username: `pi`
     - Set password: `ability123` (or whatever you want)
     - Set WiFi (your home WiFi for initial setup — we'll change this later)
4. Click **Write** and wait
5. Insert the MicroSD into the Pi and boot it

### Step 1.2 — Connect to Pi

**Option A: Monitor + Keyboard** (easy)
- Plug in HDMI, keyboard, mouse, use the desktop

**Option B: SSH** (headless)
```bash
# From your laptop (same WiFi network)
ssh pi@raspberrypi.local
# Password: ability123
```

### Step 1.3 — Install Required Packages

Run these commands on the Pi:

```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install MQTT broker
sudo apt install -y mosquitto mosquitto-clients

# Install Python dependencies
sudo apt install -y python3-pip
pip3 install paho-mqtt --break-system-packages

# Install WiFi AP tools
sudo apt install -y hostapd dnsmasq
```

### Step 1.4 — Configure WiFi Access Point

The Pi will create its own WiFi network that all ESP32s connect to.

**WARNING: After this step, the Pi will stop connecting to your home WiFi on wlan0. If you need internet on the Pi, connect it via Ethernet cable.**

#### 1.4a — Set static IP for wlan0

```bash
sudo nano /etc/dhcpcd.conf
```

Add these lines at the **bottom** of the file:

```
interface wlan0
    static ip_address=192.168.4.1/24
    nohook wpa_supplicant
```

Save: `Ctrl+O`, Enter, `Ctrl+X`

#### 1.4b — Configure hostapd (WiFi AP)

```bash
sudo nano /etc/hostapd/hostapd.conf
```

Paste this entire content:

```
interface=wlan0
driver=nl80211
ssid=Ability
hw_mode=g
channel=7
wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=ability123
wpa_key_mgmt=WPA-PSK
rsn_pairwise=CCMP
```

Save and exit.

#### 1.4c — Point hostapd to the config

```bash
sudo nano /etc/default/hostapd
```

Find the line `#DAEMON_CONF=""` and change it to:

```
DAEMON_CONF="/etc/hostapd/hostapd.conf"
```

#### 1.4d — Configure dnsmasq (DHCP)

```bash
# Backup original config
sudo mv /etc/dnsmasq.conf /etc/dnsmasq.conf.bak

sudo nano /etc/dnsmasq.conf
```

Paste:

```
interface=wlan0
dhcp-range=192.168.4.10,192.168.4.50,255.255.255.0,24h
```

Save and exit.

#### 1.4e — Enable and start services

```bash
sudo systemctl unmask hostapd
sudo systemctl enable hostapd
sudo systemctl enable dnsmasq
```

#### 1.4f — Reboot

```bash
sudo reboot
```

### Step 1.5 — Verify WiFi AP + MQTT

After reboot:

1. **On your phone** — go to WiFi settings, you should see **"Ability"** network
2. Connect with password: **ability123**

3. **On the Pi** (connect via Ethernet SSH or local keyboard):

```bash
# Check WiFi AP is running
sudo systemctl status hostapd
# Should show: active (running)

# Check MQTT broker is running
sudo systemctl status mosquitto
# Should show: active (running)

# Test MQTT — open 2 terminals:

# Terminal 1: Subscribe to all topics
mosquitto_sub -t "ability/#" -v

# Terminal 2: Publish a test message
mosquitto_pub -t "ability/test" -m "hello from pi"

# Terminal 1 should show:
# ability/test hello from pi
```

**IMPORTANT: If mosquitto_sub shows the test message, your MQTT broker is working. This is the foundation — everything else builds on this.**

### Step 1.6 — Configure Mosquitto for External Access

```bash
sudo nano /etc/mosquitto/conf.d/ability.conf
```

Paste:

```
listener 1883
allow_anonymous true
```

```bash
sudo systemctl restart mosquitto
```

---

## Phase 2: Set Up the Braille ESP32

### Step 2.1 — Install Arduino Libraries

Open Arduino IDE → **Sketch > Include Library > Manage Libraries** → Install:

| Library | Author | Purpose |
|---------|--------|---------|
| **ESP32Servo** | Kevin Harrington | Servo control |
| **PubSubClient** | Nick O'Leary | MQTT client |
| **ArduinoJson** | Benoit Blanchon | JSON parsing |

### Step 2.2 — Select Board

In Arduino IDE:
- **Tools > Board:** `ESP32 Dev Module`
- **Tools > Port:** Select your ESP32's COM port
- **Tools > Upload Speed:** `115200`

### Step 2.3 — Upload Firmware

1. Open the file: `modules/braille/firmware/braille_module/braille_mqtt.ino`
2. **Verify** the WiFi credentials match (lines 7-8):
   ```cpp
   const char* WIFI_SSID = "Ability";
   const char* WIFI_PASS = "ability123";
   ```
3. **Verify** the MQTT server IP (line 11):
   ```cpp
   const char* MQTT_SERVER = "192.168.4.1";
   ```
4. Click **Upload**
5. Open **Serial Monitor** (115200 baud)

### Step 2.4 — Verify Connection

Serial Monitor should show:

```
BRAILLE MODULE (MQTT)

M1: OK
M2: OK
M3: OK
M4: OK
Servos at home.
WiFi: Connecting to Ability............
WiFi connected! IP: 192.168.4.XX
MQTT: Connecting to 192.168.4.1... connected!
  Subscribed: ability/braille/display

READY — waiting for MQTT messages...
```

### Step 2.5 — Test from Pi

On the Pi terminal:

```bash
# Send a word to the braille module
mosquitto_pub -t "ability/braille/display" -m '{"text":"help"}'
```

**Expected:** The 4 braille modules should display H, E, L, P, hold 5 seconds, return home.

```bash
# Try more commands:
mosquitto_pub -t "ability/braille/display" -m '{"text":"abcd"}'
mosquitto_pub -t "ability/braille/display" -m '{"text":"home"}'
mosquitto_pub -t "ability/braille/display" -m '{"text":"test"}'
```

### Wiring Reference — Braille Module

```
ESP32 Pin    →    Servo
─────────────────────────
GPIO 13      →    Module 1 Left  (Dots 1,2,3)
GPIO 12      →    Module 1 Right (Dots 4,5,6)
GPIO 14      →    Module 2 Left
GPIO 27      →    Module 2 Right
GPIO 26      →    Module 3 Left
GPIO 25      →    Module 3 Right
GPIO 33      →    Module 4 Left
GPIO 32      →    Module 4 Right

Servo wires:
  Brown/Black → GND (expansion board)
  Red         → 5V  (expansion board)
  Orange      → Signal (GPIO pin)
```

---

## Phase 3: Set Up the Glove ESP32-S3

### Step 3.1 — Install Board Support

In Arduino IDE:
- **File > Preferences > Additional Board Manager URLs:** add:
  ```
  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  ```
- **Tools > Board > Board Manager:** Install `esp32` by Espressif

### Step 3.2 — Select Board

- **Tools > Board:** `ESP32S3 Dev Module`
- **Tools > USB CDC On Boot:** `Enabled`
- **Tools > Port:** Select the ESP32-S3 COM port

### Step 3.3 — Upload Firmware

1. Open: `modules/glove/firmware/glove_flex_imu/glove_mqtt.ino`
2. Verify WiFi credentials (same as braille)
3. Click **Upload**
4. Open **Serial Monitor** (115200 baud)

### Step 3.4 — Verify Connection

```
--- Glove Module (MQTT) ---
Th:1234mV  Ix:2345mV  Md:3456mV  Rg:2100mV  Pk:1800mV
BNO055 OK
WiFi: Connecting to Ability............
WiFi connected! IP: 192.168.4.XX
MQTT: Connecting... connected!
Calibrate: move fingers flat<->bent
```

### Step 3.5 — Test Glove to MQTT

1. On the Pi, start a subscriber:
   ```bash
   mosquitto_sub -t "ability/glove/text" -v
   ```
2. On the glove:
   - Sign letters (e.g., make the ASL "A" sign, hold 400ms)
   - The sentence builds: `[A]` then `[AB]` then `[ABC]`
   - To **send** the sentence: **palm-down, all fingers open** (hold 400ms)
   - Or type `send` in Serial Monitor
3. The Pi terminal should show:
   ```
   ability/glove/text {"text":"ABC","user":"glove_01","timestamp":12345}
   ```

### How to Send with the Glove

| Gesture | Action |
|---------|--------|
| ASL letters (A-Z) | Adds letter to sentence buffer |
| **Palm UP, all fingers open** | Adds a **space** |
| **Palm DOWN, all fingers open** | **SENDS** the sentence via MQTT |
| Type `send` in Serial Monitor | Also sends the sentence |
| Type `clear` in Serial Monitor | Clears the sentence buffer |

### Wiring Reference — Glove Module

```
ESP32-S3 Pin  →  Component
───────────────────────────────
GPIO 1        →  Flex Sensor: Thumb
GPIO 2        →  Flex Sensor: Index
GPIO 3        →  Flex Sensor: Middle
GPIO 4        →  Flex Sensor: Ring
GPIO 5        →  Flex Sensor: Pinky
GPIO 8 (SDA)  →  BNO055 SDA
GPIO 9 (SCL)  →  BNO055 SCL

Each flex sensor needs a 10k voltage divider:
  Flex sensor → 10k → GND
  Junction point → GPIO pin
  Flex sensor other end → 3.3V
```

---

## Phase 4: Start the Hub Engine

### Step 4.1 — Copy Project to Pi

From your laptop:

```bash
# Option A: USB drive
# Copy the Micro_Project_Ability folder to a USB drive, plug into Pi

# Option B: SCP (if Pi is on Ethernet)
scp -r /path/to/Micro_Project_Ability pi@<pi-ip>:~/

# Option C: Git
# On the Pi:
git clone <your-repo-url> ~/Micro_Project_Ability
```

### Step 4.2 — Run the Hub Engine

On the Pi:

```bash
cd ~/Micro_Project_Ability/CodeBase
python3 hub/core/main.py
```

You should see:

```
16:30:00 [INFO] ══════════════════════════════════════════════════
16:30:00 [INFO]   Project Ability — Central Hub Engine
16:30:00 [INFO]   MQTT: localhost:1883
16:30:00 [INFO] ══════════════════════════════════════════════════
16:30:00 [INFO] Connected to MQTT broker at localhost:1883
16:30:00 [INFO]   Subscribed: ability/glove/text
16:30:00 [INFO]   Subscribed: ability/voice/text
16:30:00 [INFO]   Subscribed: ability/braille/status
16:30:00 [INFO]   Subscribed: ability/glove/status
16:30:00 [INFO]   Subscribed: ability/hub/command
16:30:00 [INFO] Hub engine running. Press Ctrl+C to stop.
```

### Step 4.3 — Run Hub in Background (Optional)

To keep the hub running even after you close the terminal:

```bash
# Option A: nohup
nohup python3 hub/core/main.py &

# Option B: systemd service (permanent)
sudo nano /etc/systemd/system/ability-hub.service
```

Paste:

```ini
[Unit]
Description=Project Ability Hub Engine
After=mosquitto.service network.target

[Service]
Type=simple
User=pi
WorkingDirectory=/home/pi/Micro_Project_Ability/CodeBase
ExecStart=/usr/bin/python3 hub/core/main.py
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

```bash
sudo systemctl enable ability-hub
sudo systemctl start ability-hub

# Check status
sudo systemctl status ability-hub

# View logs
journalctl -u ability-hub -f
```

---

## Phase 5: End-to-End Test

### The Full Flow: Glove → Pi → Braille

1. **Pi:** Hub engine running (`python3 hub/core/main.py`)
2. **Braille ESP32:** Connected to WiFi + MQTT (Serial Monitor open)
3. **Glove ESP32-S3:** Connected to WiFi + MQTT (Serial Monitor open)

**Test:**
1. On the Glove — sign: **A, B, C, D**
2. You see on Glove Serial: `Sentence: [ABCD]`
3. Do the **SEND gesture** (palm down, all open)
4. **Pi Hub** logs: `Received [ability/glove/text]: {"text":"ABCD"}`
5. **Pi Hub** logs: `Routing to Braille: "ABCD"`
6. **Braille Serial** logs: `MQTT [ability/braille/display]: {"text":"ABCD"}`
7. **Braille servos** display A on M1, B on M2, C on M3, D on M4, hold 5s, home

**If you see all 7 steps above, your system is working!**

### Quick Test Without Glove

From the Pi terminal (no glove needed):

```bash
# Simulate a glove message
mosquitto_pub -t "ability/glove/text" -m '{"text":"hello","user":"test"}'
```

The hub receives it and forwards to braille automatically.

---

## Troubleshooting

### WiFi Issues

| Problem | Fix |
|---------|-----|
| ESP32 can't find "Ability" WiFi | Check `sudo systemctl status hostapd` on Pi |
| ESP32 connects but no IP | Check `sudo systemctl status dnsmasq` on Pi |
| ESP32 gets IP but can't reach MQTT | Ping test: does 192.168.4.1 respond? |

```bash
# On Pi — check who's connected to WiFi
arp -a

# Check if Mosquitto is listening
sudo netstat -tlnp | grep 1883
```

### MQTT Issues

| Problem | Fix |
|---------|-----|
| ESP32: "MQTT failed (rc=-2)" | Mosquitto not accepting external connections. Check `/etc/mosquitto/conf.d/ability.conf` has `listener 1883` |
| ESP32: "MQTT failed (rc=-4)" | Network issue — ESP32 can't reach Pi IP |
| Hub: "Cannot connect to MQTT broker" | Run `sudo systemctl restart mosquitto` |
| Messages not arriving | Check topic spelling — MQTT topics are case-sensitive |

```bash
# Debug: see ALL MQTT traffic
mosquitto_sub -t "#" -v
```

### Servo Issues

| Problem | Fix |
|---------|-----|
| Only Module 1 works | Check pin connections for M2-M4 |
| Wrong patterns on modules | Type `home` then single letters to test each |
| Servos jittering | Power supply issue — use expansion board 5V, not ESP32 pins |

---

## Network Summary

```
WiFi SSID:     Ability
WiFi Password: ability123
Pi IP:         192.168.4.1
MQTT Port:     1883
```

```
Your Phone ─── WiFi "Ability" ───┐
                                  │
Glove ESP32-S3 ── WiFi ─────────┤
                                  │
                            ┌─────┴──────┐
                            │   Pi 5      │
                            │ 192.168.4.1 │
                            │  Mosquitto  │
                            │  main.py    │
                            └─────┬───────┘
                                  │
Braille ESP32 ─── WiFi ─────────┘
```

---

## File Reference

| File | Location | Runs On |
|------|----------|---------|
| Hub Engine | `hub/core/main.py` | Pi |
| Braille Firmware | `modules/braille/firmware/braille_module/braille_mqtt.ino` | Braille ESP32 |
| Glove Firmware | `modules/glove/firmware/glove_flex_imu/glove_mqtt.ino` | Glove ESP32-S3 |
| Pi Setup Script | `scripts/setup/setup_hub.sh` | Pi |
| Original Braille (standalone WiFi) | `modules/braille/firmware/braille_module/demomodule.ino` | Braille ESP32 |
| Original Glove (no MQTT) | `modules/glove/firmware/glove_flex_imu/glove_flex_imu.ino` | Glove ESP32-S3 |

To change WiFi name/password, edit `/etc/hostapd/hostapd.conf` on the Pi AND update the `WIFI_SSID`/`WIFI_PASS` in both `.ino` files.
