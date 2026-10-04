/*
 * ══════════════════════════════════════════════════════════════
 * BRAILLE MODULE v3 — ESP32-S3 N16R8
 * Combined: 4-Cell Servo Display (Receiver) + 6-Button Keyboard (Input)
 * Project Ability
 * ══════════════════════════════════════════════════════════════
 *
 * TARGET: ESP32-S3-WROOM-1 N16R8 (DevKitC-1)
 *         16MB Flash + 8MB PSRAM (Octal SPI)
 *
 * ⚠️  N16R8 IMPORTANT: GPIOs 26-37 are NOT available!
 *     They are used internally by Octal SPI flash + PSRAM.
 *     This firmware only uses safe, available GPIOs.
 *
 * WHAT THIS DOES:
 *   1. RECEIVES text via WiFi web UI or Serial Monitor
 *      → drives 8 servos across 4 Braille cells to display it
 *   2. READS 6 push buttons as a Braille keyboard
 *      → detects which dots are pressed, decodes to A-Z,
 *        accumulates into a buffer, and can display on servos
 *
 * ── SERVO PINS (All on LEFT side header J1!) ──────────────
 *   M1: GPIO 13 (left)  + GPIO 12 (right)  [J1 Pins 19, 18]
 *   M2: GPIO 11 (left)  + GPIO 10 (right)  [J1 Pins 17, 16]
 *   M3: GPIO 9  (left)  + GPIO 8  (right)  [J1 Pins 15, 12]
 *   M4: GPIO 18 (left)  + GPIO 17 (right)  [J1 Pins 11, 10]
 *   Servo Power 5V: J1 Pin 21
 *   Servo GND     : J1 Pin 22
 *
 * ── KEYBOARD PINS (All 7 buttons on RIGHT side header J3!) ─
 *   Button 1 (Dot 1): GPIO 1   [J3 Pin 5]
 *   Button 2 (Dot 2): GPIO 2   [J3 Pin 6]
 *   Button 3 (Dot 3): GPIO 42  [J3 Pin 7]
 *   Button 4 (Dot 4): GPIO 41  [J3 Pin 8]
 *   Button 5 (Dot 5): GPIO 40  [J3 Pin 9]
 *   Button 6 (Dot 6): GPIO 39  [J3 Pin 10]
 *   Button 7 (ACTION): GPIO 47 [J3 Pin 17]
 *   Common GND      : J3 Pin 21 or Pin 22
 *
 *   TYPING LOGIC (7th Button Multi-Click):
 *   - Press Buttons 1-6 (Dots 1-6): accumulates the dot pattern.
 *   - Button 7 (1 Click)  → Merge dot clicks & commit letter to word/buffer!
 *   - Button 7 (2 Clicks) → Insert SPACE (' ') into buffer!
 *   - Button 7 (3 Clicks) → SEND FULL DATA to display on servos!
 *   - Button 7 (Long Press >1.2s) → Clear buffer and pending dots!
 *
 *   Wire each button between its GPIO pin and J3 GND.
 *   Uses internal pullups (INPUT_PULLUP) — NO external resistors needed!
 *
 * ── AVAILABLE GPIOs ON N16R8 ────────────────────────────────
 *   SAFE: 1,2,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,21,
 *         38,39,40,41,42,47,48
 *   AVOID: 0(boot), 3(JTAG), 19-20(USB), 43-44(UART),
 *          45-46(strapping), 26-37(flash/PSRAM)
 *
 * ── BRAILLE DOT LAYOUT ─────────────────────────────────────
 *     [Dot 1]  [Dot 4]
 *     [Dot 2]  [Dot 5]
 *     [Dot 3]  [Dot 6]
 *
 * ── LIBRARIES REQUIRED ─────────────────────────────────────
 *   - ESP32Servo  (Arduino Library Manager)
 *
 * ── SERIAL COMMANDS ─────────────────────────────────────────
 *   test    → display all 26 letters a-z on servos
 *   home    → reset all servos to home position
 *   sweep   → sweep all servos 0°→180°→0°
 *   diag    → test each module individually
 *   m1..m4  → test single module (e.g. 'm4')
 *   clear   → clear the keyboard input buffer
 *   send_kb → display keyboard buffer on servos, then clear it
 *   <text>  → display that text on the Braille cells
 *
 * ══════════════════════════════════════════════════════════════
 */

#include <ESP32Servo.h>
#include <WebServer.h>
#include <WiFi.h>

// ══════════════════════════════════════════════════════════════
// CONFIGURATION
// ══════════════════════════════════════════════════════════════

// ── WiFi Access Point ────────────────────────────────────────
const char *AP_SSID = "BrailleModule";
const char *AP_PASS = "braille123";

// ── Servo Pins (4 modules × 2 servos) ───────────────────────
// All on safe N16R8 GPIOs (no conflict with flash/PSRAM)
#define M1_LEFT_PIN 13
#define M1_RIGHT_PIN 12
#define M2_LEFT_PIN 11
#define M2_RIGHT_PIN 10
#define M3_LEFT_PIN 9
#define M3_RIGHT_PIN 8
#define M4_LEFT_PIN 18
#define M4_RIGHT_PIN 17

#define NUM_MODULES 4

// ── Display Timing ───────────────────────────────────────────
#define CHAR_HOLD_TIME 5000 // Hold each letter for 5 seconds
#define BATCH_GAP_TIME 3000 // Gap between batches of 4 letters
#define SERVO_MOVE_TIME 300 // Time for servo to reach position
#define SERVO_SETTLE_MS 30  // Settle time after each servo write

// ── Keyboard Button Pins (7 buttons → GND on Right Header J3) ──
#define BTN_DOT1   1    // Header J3, Pin 5  (Button 1: Braille Dot 1)
#define BTN_DOT2   2    // Header J3, Pin 6  (Button 2: Braille Dot 2)
#define BTN_DOT3   42   // Header J3, Pin 7  (Button 3: Braille Dot 3)
#define BTN_DOT4   41   // Header J3, Pin 8  (Button 4: Braille Dot 4)
#define BTN_DOT5   40   // Header J3, Pin 9  (Button 5: Braille Dot 5)
#define BTN_DOT6   39   // Header J3, Pin 10 (Button 6: Braille Dot 6)
#define BTN_ACTION 47   // Header J3, Pin 17 (Button 7: 1-click=Commit, 2-click=Space, 3-click=Send)

#define NUM_BUTTONS 7
const int BTN_PINS[NUM_BUTTONS] = {
  BTN_DOT1, BTN_DOT2, BTN_DOT3,
  BTN_DOT4, BTN_DOT5, BTN_DOT6,
  BTN_ACTION
};

// ── Keyboard Timing ──────────────────────────────────────────
#define BTN_DEBOUNCE_MS        25  // 25ms debounce delay
#define MULTI_CLICK_WINDOW_MS  400 // 400ms window for double/triple click detection

// ══════════════════════════════════════════════════════════════
// SERVO CAM ANGLE LOOKUP TABLES
// ══════════════════════════════════════════════════════════════
//
// Each 3D-printed rack has 8 stepped cam positions.
// Index = 3-bit binary of dots in that column.
//
//  Index  Binary  Dots Raised    Servo Angle
//  ─────  ──────  ────────────   ───────────
//    0     000     None           32°
//    1     001     Dot 1 only     66°
//    2     010     Dot 2 only     180°
//    3     011     Dots 1,2       88°
//    4     100     Dot 3 only     154°
//    5     101     Dots 1,3       152°
//    6     110     Dots 2,3       132°
//    7     111     All three      105°

const int CAM_ANGLES[8] = {32, 66, 180, 88, 154, 152, 132, 105};

const int RIGHT_CAM_ANGLES[8] = {22, 44, 170, 66, 148, 152, 132, 105};

// ══════════════════════════════════════════════════════════════
// BRAILLE ENCODING / DECODING
// ══════════════════════════════════════════════════════════════

// Encode: character → 6-bit Braille pattern (for servo display)
// Bit layout: [dot6][dot5][dot4][dot3][dot2][dot1]
uint8_t charToBraille(char c) {
  c = tolower(c);
  switch (c) {
  case 'a':
    return 0b000001; // dot 1
  case 'b':
    return 0b000011; // dots 1,2
  case 'c':
    return 0b001001; // dots 1,4
  case 'd':
    return 0b011001; // dots 1,4,5
  case 'e':
    return 0b010001; // dots 1,5
  case 'f':
    return 0b001011; // dots 1,2,4
  case 'g':
    return 0b011011; // dots 1,2,4,5
  case 'h':
    return 0b010011; // dots 1,2,5
  case 'i':
    return 0b001010; // dots 2,4
  case 'j':
    return 0b011010; // dots 2,4,5
  case 'k':
    return 0b000101; // dots 1,3
  case 'l':
    return 0b000111; // dots 1,2,3
  case 'm':
    return 0b001101; // dots 1,3,4
  case 'n':
    return 0b011101; // dots 1,3,4,5
  case 'o':
    return 0b010101; // dots 1,3,5
  case 'p':
    return 0b001111; // dots 1,2,3,4
  case 'q':
    return 0b011111; // dots 1,2,3,4,5
  case 'r':
    return 0b010111; // dots 1,2,3,5
  case 's':
    return 0b001110; // dots 2,3,4
  case 't':
    return 0b011110; // dots 2,3,4,5
  case 'u':
    return 0b100101; // dots 1,3,6
  case 'v':
    return 0b100111; // dots 1,2,3,6
  case 'w':
    return 0b111010; // dots 2,4,5,6
  case 'x':
    return 0b101101; // dots 1,3,4,6
  case 'y':
    return 0b111101; // dots 1,3,4,5,6
  case 'z':
    return 0b110101; // dots 1,3,5,6
  case ' ':
    return 0b000000; // space = no dots
  default:
    return 0xFF; // unsupported
  }
}

// Decode: 6-bit Braille pattern → character (for keyboard input)
char brailleTable[64];

void buildBrailleTable() {
  for (int i = 0; i < 64; i++) {
    brailleTable[i] = '\0';
  }
  brailleTable[0b000001] = 'A';
  brailleTable[0b000011] = 'B';
  brailleTable[0b001001] = 'C';
  brailleTable[0b011001] = 'D';
  brailleTable[0b010001] = 'E';
  brailleTable[0b001011] = 'F';
  brailleTable[0b011011] = 'G';
  brailleTable[0b010011] = 'H';
  brailleTable[0b001010] = 'I';
  brailleTable[0b011010] = 'J';
  brailleTable[0b000101] = 'K';
  brailleTable[0b000111] = 'L';
  brailleTable[0b001101] = 'M';
  brailleTable[0b011101] = 'N';
  brailleTable[0b010101] = 'O';
  brailleTable[0b001111] = 'P';
  brailleTable[0b011111] = 'Q';
  brailleTable[0b010111] = 'R';
  brailleTable[0b001110] = 'S';
  brailleTable[0b011110] = 'T';
  brailleTable[0b100101] = 'U';
  brailleTable[0b100111] = 'V';
  brailleTable[0b111010] = 'W';
  brailleTable[0b101101] = 'X';
  brailleTable[0b111101] = 'Y';
  brailleTable[0b110101] = 'Z';
}

// ══════════════════════════════════════════════════════════════
// GLOBAL STATE
// ══════════════════════════════════════════════════════════════

// ── Servo Pulse Width (MG90S) ────────────────────────────────
#define SERVO_MIN_US 500
#define SERVO_MAX_US 2400

// Servo objects — 8 servos for all 4 modules (M1-M4) via ESP32Servo library
Servo servos[8];

// Pin order: M1L, M1R, M2L, M2R, M3L, M3R, M4L, M4R
const int SERVO_PINS[8] = {M1_LEFT_PIN,  M1_RIGHT_PIN, M2_LEFT_PIN,
                           M2_RIGHT_PIN, M3_LEFT_PIN,  M3_RIGHT_PIN,
                           M4_LEFT_PIN,  M4_RIGHT_PIN};

bool moduleOK[NUM_MODULES] = {false, false, false, false};

// Web server
WebServer server(80);
String currentStatus = "Ready";
String lastWord = "";
bool isBusy = false;
String pendingWord = "";
bool hasPending = false;

// Keyboard state (7 buttons: Dots 1-6, Action/Enter)
bool btnLastState[NUM_BUTTONS] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
bool btnStableState[NUM_BUTTONS] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
unsigned long btnDebounceTimer[NUM_BUTTONS] = {0, 0, 0, 0, 0, 0, 0};

int actionClickCount = 0;
unsigned long actionLastClickTime = 0;
unsigned long actionPressStartTime = 0;

uint8_t pendingPattern = 0; // Bits 0..5 for active dots staged for next letter
String kbInputBuffer = "";  // Accumulated typed letters / words

// ══════════════════════════════════════════════════════════════
// WEB UI — HTML PAGE
// ══════════════════════════════════════════════════════════════

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>Braille Module — ESP32-S3</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{
  font-family:'Segoe UI',system-ui,-apple-system,sans-serif;
  background:#0a0a0f;
  color:#e0e0e0;
  min-height:100vh;
  display:flex;
  flex-direction:column;
  align-items:center;
  padding:20px;
}
.container{width:100%;max-width:420px}
.header{text-align:center;padding:30px 0 20px}
.header h1{
  font-size:22px;font-weight:700;
  background:linear-gradient(135deg,#60a5fa,#a78bfa);
  -webkit-background-clip:text;-webkit-text-fill-color:transparent;
  letter-spacing:1px;
}
.header p{font-size:13px;color:#666;margin-top:6px}
.card{
  background:rgba(255,255,255,0.04);
  border:1px solid rgba(255,255,255,0.08);
  border-radius:16px;padding:24px;margin-bottom:16px;
  backdrop-filter:blur(10px);
}
.status-bar{
  display:flex;align-items:center;gap:10px;
  padding:12px 16px;border-radius:12px;
  background:rgba(255,255,255,0.03);
  border:1px solid rgba(255,255,255,0.06);
  margin-bottom:16px;
}
.status-dot{
  width:10px;height:10px;border-radius:50%;
  background:#22c55e;box-shadow:0 0 8px #22c55e80;flex-shrink:0;
}
.status-dot.busy{
  background:#f59e0b;box-shadow:0 0 8px #f59e0b80;
  animation:pulse 1s infinite;
}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:0.4}}
.status-text{font-size:14px;color:#aaa}
.status-text span{color:#e0e0e0;font-weight:500}
.input-group{display:flex;gap:10px;margin-bottom:16px}
input[type="text"]{
  flex:1;padding:14px 18px;border-radius:12px;
  border:1px solid rgba(255,255,255,0.1);
  background:rgba(255,255,255,0.06);color:#fff;
  font-size:16px;outline:none;transition:border-color 0.2s;
}
input[type="text"]:focus{border-color:#60a5fa}
input[type="text"]::placeholder{color:#555}
.btn{
  padding:14px 24px;border-radius:12px;border:none;
  font-size:15px;font-weight:600;cursor:pointer;
  transition:all 0.2s;letter-spacing:0.5px;
}
.btn-send{background:linear-gradient(135deg,#3b82f6,#8b5cf6);color:#fff}
.btn-send:hover{transform:translateY(-1px);box-shadow:0 4px 15px rgba(59,130,246,0.4)}
.btn-send:active{transform:translateY(0)}
.btn-send:disabled{opacity:0.5;cursor:not-allowed;transform:none;box-shadow:none}
.quick-btns{display:grid;grid-template-columns:1fr 1fr;gap:8px}
.btn-quick{
  padding:12px;border-radius:10px;
  border:1px solid rgba(255,255,255,0.08);
  background:rgba(255,255,255,0.04);color:#aaa;
  font-size:13px;font-weight:500;cursor:pointer;transition:all 0.2s;
}
.btn-quick:hover{background:rgba(255,255,255,0.08);color:#fff;border-color:rgba(255,255,255,0.15)}
.btn-quick:active{transform:scale(0.97)}
.modules{display:grid;grid-template-columns:repeat(4,1fr);gap:8px;margin-top:16px}
.module-box{
  text-align:center;padding:12px 4px;border-radius:10px;
  background:rgba(255,255,255,0.03);border:1px solid rgba(255,255,255,0.06);
}
.module-box .num{font-size:11px;color:#666;margin-bottom:4px}
.module-box .letter{font-size:28px;font-weight:700;color:#60a5fa;min-height:36px;line-height:36px}
.module-box .letter.empty{color:#333}
.braille-cell{display:inline-grid;grid-template-columns:1fr 1fr;gap:3px;margin-top:6px}
.braille-cell .dot{width:8px;height:8px;border-radius:50%;background:#222;border:1px solid #333}
.braille-cell .dot.active{background:#60a5fa;border-color:#60a5fa;box-shadow:0 0 4px #60a5fa80}
.kb-section{
  margin-top:16px;padding:16px;border-radius:12px;
  background:rgba(96,165,250,0.05);border:1px solid rgba(96,165,250,0.15);
}
.kb-section h3{font-size:14px;color:#60a5fa;margin-bottom:8px}
.kb-typed{font-size:20px;font-weight:600;color:#fff;min-height:30px;letter-spacing:2px;word-break:break-all}
.kb-typed.empty{color:#444;font-style:italic;font-weight:400;font-size:14px}
.kb-actions{display:flex;gap:8px;margin-top:10px}
.btn-kb{
  flex:1;padding:10px;border-radius:8px;border:1px solid rgba(96,165,250,0.2);
  background:rgba(96,165,250,0.08);color:#60a5fa;font-size:12px;
  font-weight:600;cursor:pointer;transition:all 0.2s;
}
.btn-kb:hover{background:rgba(96,165,250,0.15);color:#fff}
.log{
  margin-top:16px;padding:12px;border-radius:10px;
  background:rgba(0,0,0,0.3);font-family:'Courier New',monospace;
  font-size:12px;color:#666;max-height:120px;overflow-y:auto;line-height:1.6;
}
.footer{text-align:center;padding:20px;font-size:11px;color:#333}
</style>
</head>
<body>
<div class="container">
  <div class="header">
    <h1>BRAILLE MODULE</h1>
    <p>ESP32-S3 — 4-Cell Display + Keyboard</p>
  </div>

  <div class="status-bar">
    <div class="status-dot" id="statusDot"></div>
    <div class="status-text">Status: <span id="statusText">Ready</span></div>
  </div>

  <div class="card">
    <div class="input-group">
      <input type="text" id="textInput" placeholder="Type text to display..." maxlength="50" autocomplete="off">
      <button class="btn btn-send" id="sendBtn" onclick="sendText()">Send</button>
    </div>
    <div class="quick-btns">
      <button class="btn-quick" onclick="sendQuick('home')">Home</button>
      <button class="btn-quick" onclick="sendQuick('test')">Test A-Z</button>
      <button class="btn-quick" onclick="sendQuick('diag')">Diagnostics</button>
      <button class="btn-quick" onclick="sendQuick('sweep')">Sweep</button>
    </div>
  </div>

  <div class="card">
    <div class="modules">
      <div class="module-box">
        <div class="num">M1</div>
        <div class="letter empty" id="m1letter">-</div>
        <div class="braille-cell" id="m1dots">
          <div class="dot"></div><div class="dot"></div>
          <div class="dot"></div><div class="dot"></div>
          <div class="dot"></div><div class="dot"></div>
        </div>
      </div>
      <div class="module-box">
        <div class="num">M2</div>
        <div class="letter empty" id="m2letter">-</div>
        <div class="braille-cell" id="m2dots">
          <div class="dot"></div><div class="dot"></div>
          <div class="dot"></div><div class="dot"></div>
          <div class="dot"></div><div class="dot"></div>
        </div>
      </div>
      <div class="module-box">
        <div class="num">M3</div>
        <div class="letter empty" id="m3letter">-</div>
        <div class="braille-cell" id="m3dots">
          <div class="dot"></div><div class="dot"></div>
          <div class="dot"></div><div class="dot"></div>
          <div class="dot"></div><div class="dot"></div>
        </div>
      </div>
      <div class="module-box">
        <div class="num">M4</div>
        <div class="letter empty" id="m4letter">-</div>
        <div class="braille-cell" id="m4dots">
          <div class="dot"></div><div class="dot"></div>
          <div class="dot"></div><div class="dot"></div>
          <div class="dot"></div><div class="dot"></div>
        </div>
      </div>
    </div>

    <div class="kb-section">
      <h3>⌨ Keyboard Input (7 Buttons)</h3>
      <div class="kb-typed empty" id="kbTyped">Waiting for button press...</div>
      <div id="kbDots" style="font-size:12px;color:#00ffcc;margin-top:6px;min-height:16px;"></div>
      <div class="kb-actions">
        <button class="btn-kb" onclick="sendQuick('send_kb')">Display on Servos</button>
        <button class="btn-kb" onclick="sendQuick('clear')">Clear Buffer</button>
      </div>
    </div>
  </div>

  <div class="log" id="log">System ready...</div>

  <div class="footer">
    Project Ability — Braille Module v3 (ESP32-S3)
  </div>
</div>

<script>
const brailleMap={
  'a':0b000001,'b':0b000011,'c':0b001001,'d':0b011001,'e':0b010001,
  'f':0b001011,'g':0b011011,'h':0b010011,'i':0b001010,'j':0b011010,
  'k':0b000101,'l':0b000111,'m':0b001101,'n':0b011101,'o':0b010101,
  'p':0b001111,'q':0b011111,'r':0b010111,'s':0b001110,'t':0b011110,
  'u':0b100101,'v':0b100111,'w':0b111010,'x':0b101101,'y':0b111101,
  'z':0b110101,' ':0b000000
};

function sendText(){
  const input=document.getElementById('textInput');
  const text=input.value.trim();
  if(!text)return;
  fetch('/send?text='+encodeURIComponent(text))
    .then(r=>r.json())
    .then(d=>{
      if(d.ok){addLog('Sent: "'+text+'"');input.value='';updateModuleDisplay(text)}
      else{addLog('Busy! Try again.')}
      pollStatus();
    })
    .catch(()=>addLog('Error sending'));
}

function sendQuick(cmd){
  fetch('/send?text='+cmd)
    .then(r=>r.json())
    .then(d=>{addLog('Command: '+cmd);pollStatus()})
    .catch(()=>addLog('Error'));
}

function updateModuleDisplay(text){
  text=text.toLowerCase();
  for(let i=0;i<4;i++){
    const letter=document.getElementById('m'+(i+1)+'letter');
    const dotsEl=document.getElementById('m'+(i+1)+'dots');
    const dots=dotsEl.querySelectorAll('.dot');
    if(i<text.length){
      const ch=text[i];
      letter.textContent=ch.toUpperCase();
      letter.classList.remove('empty');
      const pattern=brailleMap[ch]||0;
      const dotOrder=[0,3,1,4,2,5];
      for(let d=0;d<6;d++){dots[d].classList.toggle('active',(pattern>>dotOrder[d])&1)}
    }else{
      letter.textContent='-';letter.classList.add('empty');
      for(let d=0;d<6;d++){dots[d].classList.remove('active')}
    }
  }
}

function pollStatus(){
  fetch('/status')
    .then(r=>r.json())
    .then(d=>{
      document.getElementById('statusText').textContent=d.status;
      const dot=document.getElementById('statusDot');
      const btn=document.getElementById('sendBtn');
      if(d.busy){dot.classList.add('busy');btn.disabled=true;setTimeout(pollStatus,1000)}
      else{dot.classList.remove('busy');btn.disabled=false}
      const kbEl=document.getElementById('kbTyped');
      if(d.kb&&d.kb.length>0){kbEl.textContent=d.kb;kbEl.classList.remove('empty')}
      else{kbEl.textContent='Waiting for button press...';kbEl.classList.add('empty')}
      const kbDots=document.getElementById('kbDots');
      if(d.dots&&d.dots!=='none'){kbDots.textContent='Active Dots: '+d.dots+' (Click Enter to commit)'}
      else{kbDots.textContent=''}
    })
    .catch(()=>{});
}

function addLog(msg){
  const log=document.getElementById('log');
  const time=new Date().toLocaleTimeString();
  log.innerHTML+='<br>['+time+'] '+msg;
  log.scrollTop=log.scrollHeight;
}

document.getElementById('textInput').addEventListener('keydown',function(e){
  if(e.key==='Enter')sendText();
});

setInterval(pollStatus,2000);
</script>
</body>
</html>
)rawliteral";

// ══════════════════════════════════════════════════════════════
// WEB SERVER HANDLERS
// ══════════════════════════════════════════════════════════════

void handleRoot() { server.send(200, "text/html", HTML_PAGE); }

void handleSend() {
  if (!server.hasArg("text")) {
    server.send(400, "application/json",
                "{\"ok\":false,\"error\":\"no text\"}");
    return;
  }

  String text = server.arg("text");
  text.trim();

  if (text.length() == 0) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"empty\"}");
    return;
  }

  if (isBusy) {
    server.send(200, "application/json", "{\"ok\":false,\"error\":\"busy\"}");
    return;
  }

  pendingWord = text;
  hasPending = true;
  server.send(200, "application/json", "{\"ok\":true}");

  Serial.print("[Web] Input: ");
  Serial.println(text);
}

String getDotsString(uint8_t pattern);

void handleStatus() {
  String json = "{\"status\":\"" + currentStatus +
                "\",\"busy\":" + (isBusy ? "true" : "false") + ",\"last\":\"" +
                lastWord + "\",\"kb\":\"" + kbInputBuffer +
                "\",\"dots\":\"" + getDotsString(pendingPattern) + "\"}";
  server.send(200, "application/json", json);
}

// ══════════════════════════════════════════════════════════════
// SERVO CONTROL
// ══════════════════════════════════════════════════════════════

void writeLeftServo(int module, int angle) {
  if (module >= 0 && module < NUM_MODULES) {
    int idx = module * 2;
    if (servos[idx].attached())
      servos[idx].write(angle);
  }
}

void writeRightServo(int module, int angle) {
  if (module >= 0 && module < NUM_MODULES) {
    int idx = module * 2 + 1;
    if (servos[idx].attached())
      servos[idx].write(angle);
  }
}

void driveModule(int module, uint8_t pattern) {
  // Split 6-bit pattern into left (dots 1,2,3) and right (dots 4,5,6)
  uint8_t leftPattern = pattern & 0x07;
  uint8_t rightPattern = (pattern >> 3) & 0x07;

  // Right slider is flipped — swap bits 0 and 2
  rightPattern = ((rightPattern & 0x01) << 2) | (rightPattern & 0x02) |
                 ((rightPattern & 0x04) >> 2);

  int leftAngle = CAM_ANGLES[leftPattern];
  int rightAngle = RIGHT_CAM_ANGLES[rightPattern];

  writeLeftServo(module, leftAngle);
  delay(SERVO_SETTLE_MS);
  writeRightServo(module, rightAngle);
  delay(SERVO_SETTLE_MS);

  Serial.print("    M");
  Serial.print(module + 1);
  Serial.print(": L=");
  Serial.print(leftAngle);
  Serial.print("° R=");
  Serial.print(rightAngle);
  Serial.println("°");
}

void homeModule(int module) {
  writeLeftServo(module, 22);
  delay(SERVO_SETTLE_MS);
  writeRightServo(module, 22);
  delay(SERVO_SETTLE_MS);
}

void homeAllServos() {
  for (int m = 0; m < NUM_MODULES; m++) {
    if (moduleOK[m]) {
      homeModule(m);
    }
  }
  delay(SERVO_MOVE_TIME);
}

// ══════════════════════════════════════════════════════════════
// KEYBOARD INPUT — 7 Buttons on Right Header J3
// Buttons 1-6 = Braille Dots 1-6
// Button 7    = Action: 1-click=Commit, 2-click=Space, 3-click=Send
// ══════════════════════════════════════════════════════════════

// Print which dots are active (e.g., "1,2,5")
void printDots(uint8_t pattern) {
  bool first = true;
  for (int i = 0; i < 6; i++) {
    if (pattern & (1 << i)) {
      if (!first)
        Serial.print(",");
      Serial.print(i + 1);
      first = false;
    }
  }
  if (first)
    Serial.print("none");
}

String getDotsString(uint8_t pattern) {
  if (pattern == 0) return "none";
  String s = "";
  for (int i = 0; i < 6; i++) {
    if (pattern & (1 << i)) {
      if (s.length() > 0) s += ",";
      s += String(i + 1);
    }
  }
  return s;
}

// Button 7 — 1 Click: Merge active dots & commit as letter to word
void commitPendingPattern() {
  if (pendingPattern == 0) {
    Serial.print("[KB] Button 7 (1 Click) → No dots active. Current buffer: \"");
    Serial.print(kbInputBuffer);
    Serial.println("\"");
    return;
  }
  char letter = brailleTable[pendingPattern];
  if (letter != '\0') {
    kbInputBuffer += letter;
    Serial.print("[KB] Button 7 (1 Click) → Merged dots [");
    printDots(pendingPattern);
    Serial.print("] → Committed '");
    Serial.print(letter);
    Serial.print("' | Buffer: \"");
    Serial.print(kbInputBuffer);
    Serial.println("\"");
  } else {
    Serial.print("[KB] Button 7 (1 Click) → Unknown Braille pattern (dots ");
    printDots(pendingPattern);
    Serial.println("). Not added.");
  }
  pendingPattern = 0;
}

// Button 7 — 2 Clicks: Insert Space
void insertSpace() {
  // Commit any staged dots first
  if (pendingPattern != 0) {
    char letter = brailleTable[pendingPattern];
    if (letter != '\0') {
      kbInputBuffer += letter;
    }
    pendingPattern = 0;
  }
  kbInputBuffer += ' ';
  Serial.print("[KB] Button 7 (2 Clicks) → SPACE inserted | Buffer: \"");
  Serial.print(kbInputBuffer);
  Serial.println("\"");
}

// Button 7 — 3 Clicks: Send Full Data to Servos
void sendFullBufferToServos() {
  // If dots are pending, commit them first
  if (pendingPattern != 0) {
    char letter = brailleTable[pendingPattern];
    if (letter != '\0') {
      kbInputBuffer += letter;
    }
    pendingPattern = 0;
  }

  if (kbInputBuffer.length() > 0) {
    Serial.print("[KB] Button 7 (3 Clicks) → SEND FULL DATA to servos: \"");
    Serial.print(kbInputBuffer);
    Serial.println("\"");
    currentStatus = "KB → Display: " + kbInputBuffer;
    displayWord(kbInputBuffer);
    kbInputBuffer = "";
  } else {
    Serial.println("[KB] Button 7 (3 Clicks) → Buffer is empty. Nothing to display.");
  }
}

void readKeyboard() {
  // Don't read keyboard while servos are actively moving
  if (isBusy)
    return;

  unsigned long now = millis();

  // 1. Debounce and read all 7 buttons
  for (int i = 0; i < NUM_BUTTONS; i++) {
    int reading = digitalRead(BTN_PINS[i]);

    if (reading != btnLastState[i]) {
      btnDebounceTimer[i] = now;
    }

    if ((now - btnDebounceTimer[i]) > BTN_DEBOUNCE_MS) {
      if (reading != btnStableState[i]) {
        btnStableState[i] = reading;

        // Button Pressed (Falling edge to GND)
        if (btnStableState[i] == LOW) {
          if (i < 6) {
            // Buttons 1..6: Braille Dots 1..6 — Accumulate dots into pattern
            pendingPattern |= (1 << i);
            Serial.print("[KB] Button ");
            Serial.print(i + 1);
            Serial.print(" (Dot ");
            Serial.print(i + 1);
            Serial.print(") pressed → Active dots: ");
            printDots(pendingPattern);
            char preview = brailleTable[pendingPattern];
            if (preview != '\0') {
              Serial.print(" [Preview: '");
              Serial.print(preview);
              Serial.print("']");
            }
            Serial.println();
          }
          else if (i == 6) {
            // Button 7: Action (1-click=Commit, 2-click=Space, 3-click=Send)
            actionPressStartTime = now;
            actionClickCount++;
            actionLastClickTime = now;

            if (actionClickCount >= 3) {
              // 3 Clicks reached immediately → Send full data!
              actionClickCount = 0;
              sendFullBufferToServos();
            }
          }
        }
        else {
          // Button Released (Rising edge)
          if (i == 6) {
            actionPressStartTime = 0;
          }
        }
      }
    }

    btnLastState[i] = reading;
  }

  // 2. Long Press on Button 7 to Clear Buffer (>1.2s)
  if (btnStableState[6] == LOW && actionPressStartTime > 0) {
    if (now - actionPressStartTime > 1200) {
      actionPressStartTime = 0; // Prevent repeated triggers while holding
      actionClickCount = 0;
      pendingPattern = 0;
      kbInputBuffer = "";
      Serial.println("[KB] Button 7 (Long Press >1.2s) → Buffer and pending dots CLEARED!");
    }
  }

  // 3. Multi-Click Window Timeout on Button 7 (1 Click = Commit, 2 Clicks = Space)
  if (actionClickCount > 0 && (now - actionLastClickTime >= MULTI_CLICK_WINDOW_MS)) {
    if (actionClickCount == 1) {
      commitPendingPattern();
    } else if (actionClickCount == 2) {
      insertSpace();
    }
    actionClickCount = 0;
  }
}

// ══════════════════════════════════════════════════════════════
// BRAILLE INFO — Pretty-print a character's Braille pattern
// ══════════════════════════════════════════════════════════════

void printBrailleInfo(char c, uint8_t pattern) {
  // Binary pattern
  Serial.print("0b");
  for (int b = 5; b >= 0; b--) {
    Serial.print((pattern >> b) & 1);
  }

  // Active dots
  Serial.print("  Dots: ");
  bool first = true;
  for (int d = 0; d < 6; d++) {
    if (pattern & (1 << d)) {
      if (!first)
        Serial.print(",");
      Serial.print(d + 1);
      first = false;
    }
  }
  if (first)
    Serial.print("(none)");

  // Visual Braille cell
  Serial.print("  Cell: ");
  Serial.print((pattern & 0x01) ? "O" : ".");
  Serial.print((pattern & 0x08) ? "O" : ".");
  Serial.print(" ");
  Serial.print((pattern & 0x02) ? "O" : ".");
  Serial.print((pattern & 0x10) ? "O" : ".");
  Serial.print(" ");
  Serial.print((pattern & 0x04) ? "O" : ".");
  Serial.println((pattern & 0x20) ? "O" : ".");
}

// ══════════════════════════════════════════════════════════════
// DISPLAY WORD — drive servos in batches of 4
// ══════════════════════════════════════════════════════════════

void displayWord(String word) {
  int totalChars = word.length();
  int totalBatches = (totalChars + NUM_MODULES - 1) / NUM_MODULES;

  Serial.print("Word: ");
  Serial.print(word);
  Serial.print(" → ");
  Serial.print(totalChars);
  Serial.print(" chars, ");
  Serial.print(totalBatches);
  Serial.println(" batch(es)");

  for (int batch = 0; batch < totalBatches; batch++) {
    int startIdx = batch * NUM_MODULES;
    int charsInBatch = min(NUM_MODULES, totalChars - startIdx);

    currentStatus = "Batch " + String(batch + 1) + "/" + String(totalBatches);

    Serial.print("  Batch ");
    Serial.print(batch + 1);
    Serial.print("/");
    Serial.println(totalBatches);

    uint8_t patterns[NUM_MODULES];
    bool active[NUM_MODULES];

    for (int m = 0; m < NUM_MODULES; m++) {
      if (m < charsInBatch) {
        char c = word.charAt(startIdx + m);
        patterns[m] = charToBraille(c);
        active[m] = (patterns[m] != 0xFF) && moduleOK[m];

        Serial.print("    M");
        Serial.print(m + 1);
        Serial.print(": '");
        Serial.print(c);
        Serial.print("' → ");

        if (!moduleOK[m]) {
          Serial.println("(not attached)");
        } else if (patterns[m] == 0xFF) {
          Serial.println("(unsupported char)");
        } else {
          printBrailleInfo(c, patterns[m]);
        }
      } else {
        patterns[m] = 0b000000;
        active[m] = moduleOK[m];
      }
    }

    Serial.println("  Driving servos...");

    for (int m = 0; m < NUM_MODULES; m++) {
      if (active[m]) {
        driveModule(m, patterns[m]);
        delay(200);
      }
    }

    delay(SERVO_MOVE_TIME);

    Serial.print("  Holding ");
    Serial.print(CHAR_HOLD_TIME / 1000);
    Serial.println("s...");
    delay(CHAR_HOLD_TIME);

    Serial.println("  Returning to home...");
    homeAllServos();

    if (batch < totalBatches - 1) {
      Serial.print("  Pause ");
      Serial.print(BATCH_GAP_TIME / 1000);
      Serial.println("s...");
      delay(BATCH_GAP_TIME);
    }

    Serial.println();
  }

  Serial.println("Word complete!");
}

// ══════════════════════════════════════════════════════════════
// TEST FUNCTIONS
// ══════════════════════════════════════════════════════════════

void runTestAllLetters() {
  Serial.println("Running A-Z in groups of 4...");

  for (int i = 0; i < 26; i += NUM_MODULES) {
    int charsInBatch = min(NUM_MODULES, 26 - i);

    for (int m = 0; m < charsInBatch; m++) {
      char c = 'a' + i + m;
      uint8_t pattern = charToBraille(c);

      Serial.print("  M");
      Serial.print(m + 1);
      Serial.print(": ");
      Serial.print(c);
      Serial.print(" → ");
      printBrailleInfo(c, pattern);

      if (moduleOK[m]) {
        driveModule(m, pattern);
      }
    }

    delay(SERVO_MOVE_TIME);
    delay(CHAR_HOLD_TIME);
    homeAllServos();
    delay(BATCH_GAP_TIME);
  }

  Serial.println("All 26 letters complete!");
}

void testSingleModule(int m) {
  if (m < 0 || m >= NUM_MODULES)
    return;
  Serial.print("--- Testing Module ");
  Serial.print(m + 1);
  Serial.print(" (Left GPIO ");
  Serial.print(SERVO_PINS[m * 2]);
  Serial.print(", Right GPIO ");
  Serial.print(SERVO_PINS[m * 2 + 1]);
  Serial.println(") ---");

  if (!moduleOK[m]) {
    Serial.println(
        "  WARNING: Module not marked OK! Attempting drive anyway...");
  }

  Serial.println("  Left servo -> 90°...");
  writeLeftServo(m, 90);
  delay(1000);
  Serial.println("  Left servo -> 22° (home)...");
  writeLeftServo(m, 22);
  delay(1000);

  Serial.println("  Right servo -> 90°...");
  writeRightServo(m, 90);
  delay(1000);
  Serial.println("  Right servo -> 22° (home)...");
  writeRightServo(m, 22);
  delay(1000);

  Serial.println("  Testing Braille 'a'...");
  driveModule(m, charToBraille('a'));
  delay(2000);
  homeModule(m);
  Serial.println("  Done module test.");
}

void runDiagnostics() {
  Serial.println("Testing each module individually...");

  for (int m = 0; m < NUM_MODULES; m++) {
    Serial.print("Module ");
    Serial.print(m + 1);

    if (!moduleOK[m]) {
      Serial.println(" SKIPPED (not attached)");
      continue;
    }

    Serial.println(":");

    // Test left servo
    writeLeftServo(m, 90);
    delay(500);
    writeLeftServo(m, 22);
    delay(500);

    // Test right servo
    writeRightServo(m, 90);
    delay(500);
    writeRightServo(m, 22);
    delay(500);

    // Test with letter 'a'
    driveModule(m, charToBraille('a'));
    delay(2000);
    homeModule(m);
    delay(300);

    Serial.println("  done.");
  }

  Serial.println("Diagnostics complete.");
}

void runSweepTest() {
  Serial.println("Sweeping all servos...");

  for (int m = 0; m < NUM_MODULES; m++) {
    if (!moduleOK[m]) {
      Serial.print("Module ");
      Serial.print(m + 1);
      Serial.println(" SKIPPED");
      continue;
    }

    Serial.print("Module ");
    Serial.print(m + 1);
    Serial.println(":");

    // Sweep left
    for (int angle = 0; angle <= 180; angle += 10) {
      writeLeftServo(m, angle);
      delay(150);
    }
    for (int angle = 180; angle >= 0; angle -= 10) {
      writeLeftServo(m, angle);
      delay(150);
    }
    Serial.println("  Left done.");

    // Sweep right
    for (int angle = 0; angle <= 180; angle += 10) {
      writeRightServo(m, angle);
      delay(150);
    }
    for (int angle = 180; angle >= 0; angle -= 10) {
      writeRightServo(m, angle);
      delay(150);
    }
    Serial.println("  Right done.");
  }

  homeAllServos();
  Serial.println("Sweep complete.");
}

// ══════════════════════════════════════════════════════════════
// PROCESS INPUT — from Web UI, Serial, or Keyboard
// ══════════════════════════════════════════════════════════════

void processInput(String text) {
  if (isBusy)
    return;
  isBusy = true;

  if (text.equalsIgnoreCase("test")) {
    currentStatus = "Running test...";
    runTestAllLetters();
  } else if (text.equalsIgnoreCase("home")) {
    currentStatus = "Homing...";
    homeAllServos();
    Serial.println("All servos at home (22°).");
  } else if (text.equalsIgnoreCase("sweep")) {
    currentStatus = "Sweeping...";
    runSweepTest();
  } else if (text.equalsIgnoreCase("diag")) {
    currentStatus = "Diagnostics...";
    runDiagnostics();
  } else if (text.equalsIgnoreCase("m1")) {
    testSingleModule(0);
  } else if (text.equalsIgnoreCase("m2")) {
    testSingleModule(1);
  } else if (text.equalsIgnoreCase("m3")) {
    testSingleModule(2);
  } else if (text.equalsIgnoreCase("m4")) {
    testSingleModule(3);
  } else if (text.equalsIgnoreCase("clear")) {
    kbInputBuffer = "";
    Serial.println("[KB] Buffer cleared.");
  } else if (text.equalsIgnoreCase("send_kb")) {
    // Display whatever was typed via keyboard buttons
    if (kbInputBuffer.length() > 0) {
      currentStatus = "KB → Display: " + kbInputBuffer;
      Serial.print("[KB → Display] ");
      Serial.println(kbInputBuffer);
      displayWord(kbInputBuffer);
      kbInputBuffer = "";
    } else {
      Serial.println("[KB] Buffer is empty — nothing to display.");
    }
  } else {
    // Normal text — display on servos
    lastWord = text;
    currentStatus = "Displaying: " + text;
    displayWord(text);
  }

  currentStatus = "Ready";
  isBusy = false;
  Serial.println("Ready for next input...");
}

// ══════════════════════════════════════════════════════════════
// SETUP — runs once on power-up
// ══════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("════════════════════════════════════════");
  Serial.println("  BRAILLE MODULE v3 — ESP32-S3");
  Serial.println("  4-Cell Display + 7-Button Keyboard (Header J3)");
  Serial.println("════════════════════════════════════════");
  Serial.println();

  // ── Build Braille decode table (keyboard → letter) ─────
  buildBrailleTable();

  // ── Setup keyboard button pins (All 7 on Header J3) ───
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(BTN_PINS[i], INPUT_PULLUP);
    btnLastState[i] = digitalRead(BTN_PINS[i]);
    btnStableState[i] = btnLastState[i];
  }
  Serial.println("[KB] 7 button pins configured on Right Header J3 (INPUT_PULLUP):");
  Serial.print("     Dots 1-6 (Btns 1-6): GPIOs ");
  for (int i = 0; i < 6; i++) {
    if (i > 0)
      Serial.print(", ");
    Serial.print(BTN_PINS[i]);
  }
  Serial.println();
  Serial.print("     ACTION   (Button 7): GPIO ");
  Serial.println(BTN_ACTION);
  Serial.println("     [Action Logic: 1-Click=Commit, 2-Clicks=Space, 3-Clicks=Send, Hold=Clear]");

  // ── Setup WiFi AP ──────────────────────────────────────
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("[WiFi] AP: ");
  Serial.print(AP_SSID);
  Serial.print(" / ");
  Serial.println(AP_PASS);
  Serial.print("[WiFi] IP: ");
  Serial.println(WiFi.softAPIP());

  // ── Setup Web Server ───────────────────────────────────
  server.on("/", handleRoot);
  server.on("/send", handleSend);
  server.on("/status", handleStatus);
  server.begin();
  Serial.println("[Web] Server started on port 80");

  // ── Setup Servos ────────────────────────────────────────
  // ESP32-S3: Allocate all 4 timers (0, 1, 2, 3) to enable all 8 channels for
  // ESP32Servo.
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

#if defined(CONFIG_IDF_TARGET_ESP32S3)
  // Force ESP32Servo to use LEDC channels instead of buggy MCPWM on ESP32-S3:
  // On ESP32-S3, ESP32Servo's internal MCPWM unit 1 misconfigures output pins,
  // preventing M4 (channels 6 & 7) from receiving pulses.
  // Filling MCPWM operator slots forces fallback to clean LEDC hardware
  // channels (0-7).
  for (int u = 0; u < MCPWM_NUM_UNITS; u++) {
    for (int t = 0; t < MCPWM_NUM_TIMERS_PER_UNIT; t++) {
      ESP32PWM::mcpwmTimers[u][t].operatorCount = MCPWM_NUM_OPERATORS_PER_TIMER;
    }
  }
#endif

  Serial.println("[Servos] Attaching all 8 servos (M1-M4) via ESP32Servo (LEDC "
                 "forced)...");

  const char *servoNames[8] = {"M1-Left", "M1-Right", "M2-Left", "M2-Right",
                               "M3-Left", "M3-Right", "M4-Left", "M4-Right"};

  for (int i = 0; i < 8; i++) {
    servos[i].setPeriodHertz(50);
    servos[i].attach(SERVO_PINS[i], SERVO_MIN_US, SERVO_MAX_US);
    delay(100);

    Serial.print("  [");
    Serial.print(servoNames[i]);
    Serial.print("] GPIO ");
    Serial.print(SERVO_PINS[i]);
    Serial.print("... ");
    Serial.println(servos[i].attached() ? "OK" : "FAILED");
  }

  for (int m = 0; m < NUM_MODULES; m++) {
    moduleOK[m] = servos[m * 2].attached() && servos[m * 2 + 1].attached();
  }

  // Count OK modules
  int okCount = 0;
  for (int m = 0; m < NUM_MODULES; m++) {
    if (moduleOK[m])
      okCount++;
  }
  Serial.print(okCount);
  Serial.println("/4 modules attached.");

  // Home all servos
  Serial.println("Moving all servos to home (22°)...");
  homeAllServos();
  delay(1000);
  Serial.println("All servos at home.");

  // Ready message
  Serial.println();
  Serial.println("════════════════════════════════════════");
  Serial.println("  READY!");
  Serial.println("════════════════════════════════════════");
  Serial.println("WiFi: " + String(AP_SSID) + " → http://192.168.4.1");
  Serial.println();
  Serial.println("Serial commands:");
  Serial.println("  test    → display A-Z on servos");
  Serial.println("  home    → reset servos to 22°");
  Serial.println("  sweep   → sweep all servos");
  Serial.println("  diag    → test each module");
  Serial.println("  m1..m4  → test single module (e.g. 'm4')");
  Serial.println("  clear   → clear keyboard buffer");
  Serial.println("  send_kb → display keyboard input on servos");
  Serial.println("  <text>  → display text on servos");
  Serial.println();
  Serial.println("Keyboard: press Braille buttons to type");
  Serial.println();
}

// ══════════════════════════════════════════════════════════════
// MAIN LOOP
// ══════════════════════════════════════════════════════════════

void loop() {
  // 1. Handle web requests
  server.handleClient();

  // 2. Handle pending commands from web UI
  if (hasPending) {
    hasPending = false;
    processInput(pendingWord);
  }

  // 3. Handle serial input
  if (Serial.available() > 0) {
    String serialInput = Serial.readStringUntil('\n');
    serialInput.trim();
    if (serialInput.length() > 0) {
      Serial.print("[Serial] ");
      Serial.println(serialInput);
      processInput(serialInput);
    }
  }

  // 4. Read keyboard buttons (every loop iteration)
  readKeyboard();

  // 5. Short yield (5ms) for WiFi and background tasks
  delay(5);
}
