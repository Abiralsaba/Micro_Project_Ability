#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* AP_SSID = "BrailleModule";
const char* AP_PASS = "braille123";

#define M1_LEFT_PIN   13
#define M1_RIGHT_PIN  12
#define M2_LEFT_PIN   14
#define M2_RIGHT_PIN  27
#define M3_LEFT_PIN   26
#define M3_RIGHT_PIN  25
#define M4_LEFT_PIN   33
#define M4_RIGHT_PIN  32

#define NUM_MODULES 4

#define SERVO_MIN_US 500
#define SERVO_MAX_US 2400

#define CHAR_HOLD_TIME  5000
#define BATCH_GAP_TIME  3000
#define SERVO_MOVE_TIME 300
#define SERVO_SETTLE_MS 30

const int CAM_ANGLES[8] = {
    32, 66, 180, 88, 154, 152, 132, 105
};

const int RIGHT_CAM_ANGLES[8] = {
    22, 44, 170, 66, 148, 152, 132, 105
};

uint8_t charToBraille(char c) {
  c = tolower(c);
  switch (c) {
  case 'a': return 0b000001;
  case 'b': return 0b000011;
  case 'c': return 0b001001;
  case 'd': return 0b011001;
  case 'e': return 0b010001;
  case 'f': return 0b001011;
  case 'g': return 0b011011;
  case 'h': return 0b010011;
  case 'i': return 0b001010;
  case 'j': return 0b011010;
  case 'k': return 0b000101;
  case 'l': return 0b000111;
  case 'm': return 0b001101;
  case 'n': return 0b011101;
  case 'o': return 0b010101;
  case 'p': return 0b001111;
  case 'q': return 0b011111;
  case 'r': return 0b010111;
  case 's': return 0b001110;
  case 't': return 0b011110;
  case 'u': return 0b100101;
  case 'v': return 0b100111;
  case 'w': return 0b111010;
  case 'x': return 0b101101;
  case 'y': return 0b111101;
  case 'z': return 0b110101;
  case ' ': return 0b000000;
  default:  return 0xFF;
  }
}

Servo m1Left;
Servo m1Right;
Servo m2Left;
Servo m2Right;
Servo m3Left;
Servo m3Right;
Servo m4Left;
Servo m4Right;

bool moduleOK[NUM_MODULES] = { false, false, false, false };

WebServer server(80);

String currentStatus = "Ready";
String lastWord = "";
bool isBusy = false;
String pendingWord = "";
bool hasPending = false;

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>Braille Module</title>
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
.container{
  width:100%;
  max-width:420px;
}
.header{
  text-align:center;
  padding:30px 0 20px;
}
.header h1{
  font-size:22px;
  font-weight:700;
  background:linear-gradient(135deg,#60a5fa,#a78bfa);
  -webkit-background-clip:text;
  -webkit-text-fill-color:transparent;
  letter-spacing:1px;
}
.header p{
  font-size:13px;
  color:#666;
  margin-top:6px;
}
.card{
  background:rgba(255,255,255,0.04);
  border:1px solid rgba(255,255,255,0.08);
  border-radius:16px;
  padding:24px;
  margin-bottom:16px;
  backdrop-filter:blur(10px);
}
.status-bar{
  display:flex;
  align-items:center;
  gap:10px;
  padding:12px 16px;
  border-radius:12px;
  background:rgba(255,255,255,0.03);
  border:1px solid rgba(255,255,255,0.06);
  margin-bottom:16px;
}
.status-dot{
  width:10px;
  height:10px;
  border-radius:50%;
  background:#22c55e;
  box-shadow:0 0 8px #22c55e80;
  flex-shrink:0;
}
.status-dot.busy{
  background:#f59e0b;
  box-shadow:0 0 8px #f59e0b80;
  animation:pulse 1s infinite;
}
@keyframes pulse{
  0%,100%{opacity:1}
  50%{opacity:0.4}
}
.status-text{
  font-size:14px;
  color:#aaa;
}
.status-text span{
  color:#e0e0e0;
  font-weight:500;
}
.input-group{
  display:flex;
  gap:10px;
  margin-bottom:16px;
}
input[type="text"]{
  flex:1;
  padding:14px 18px;
  border-radius:12px;
  border:1px solid rgba(255,255,255,0.1);
  background:rgba(255,255,255,0.06);
  color:#fff;
  font-size:16px;
  outline:none;
  transition:border-color 0.2s;
}
input[type="text"]:focus{
  border-color:#60a5fa;
}
input[type="text"]::placeholder{
  color:#555;
}
.btn{
  padding:14px 24px;
  border-radius:12px;
  border:none;
  font-size:15px;
  font-weight:600;
  cursor:pointer;
  transition:all 0.2s;
  letter-spacing:0.5px;
}
.btn-send{
  background:linear-gradient(135deg,#3b82f6,#8b5cf6);
  color:#fff;
}
.btn-send:hover{
  transform:translateY(-1px);
  box-shadow:0 4px 15px rgba(59,130,246,0.4);
}
.btn-send:active{
  transform:translateY(0);
}
.btn-send:disabled{
  opacity:0.5;
  cursor:not-allowed;
  transform:none;
  box-shadow:none;
}
.quick-btns{
  display:grid;
  grid-template-columns:1fr 1fr;
  gap:8px;
}
.btn-quick{
  padding:12px;
  border-radius:10px;
  border:1px solid rgba(255,255,255,0.08);
  background:rgba(255,255,255,0.04);
  color:#aaa;
  font-size:13px;
  font-weight:500;
  cursor:pointer;
  transition:all 0.2s;
}
.btn-quick:hover{
  background:rgba(255,255,255,0.08);
  color:#fff;
  border-color:rgba(255,255,255,0.15);
}
.btn-quick:active{
  transform:scale(0.97);
}
.modules{
  display:grid;
  grid-template-columns:repeat(4,1fr);
  gap:8px;
  margin-top:16px;
}
.module-box{
  text-align:center;
  padding:12px 4px;
  border-radius:10px;
  background:rgba(255,255,255,0.03);
  border:1px solid rgba(255,255,255,0.06);
}
.module-box .num{
  font-size:11px;
  color:#666;
  margin-bottom:4px;
}
.module-box .letter{
  font-size:28px;
  font-weight:700;
  color:#60a5fa;
  min-height:36px;
  line-height:36px;
}
.module-box .letter.empty{
  color:#333;
}
.braille-cell{
  display:inline-grid;
  grid-template-columns:1fr 1fr;
  gap:3px;
  margin-top:6px;
}
.braille-cell .dot{
  width:8px;
  height:8px;
  border-radius:50%;
  background:#222;
  border:1px solid #333;
}
.braille-cell .dot.active{
  background:#60a5fa;
  border-color:#60a5fa;
  box-shadow:0 0 4px #60a5fa80;
}
.log{
  margin-top:16px;
  padding:12px;
  border-radius:10px;
  background:rgba(0,0,0,0.3);
  font-family:'Courier New',monospace;
  font-size:12px;
  color:#666;
  max-height:120px;
  overflow-y:auto;
  line-height:1.6;
}
.footer{
  text-align:center;
  padding:20px;
  font-size:11px;
  color:#333;
}
</style>
</head>
<body>
<div class="container">
  <div class="header">
    <h1>BRAILLE MODULE</h1>
    <p>4-Module Display Controller</p>
  </div>

  <div class="status-bar">
    <div class="status-dot" id="statusDot"></div>
    <div class="status-text">Status: <span id="statusText">Ready</span></div>
  </div>

  <div class="card">
    <div class="input-group">
      <input type="text" id="textInput" placeholder="Type text here..." maxlength="50" autocomplete="off">
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
    <div class="modules" id="modules">
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
  </div>

  <div class="log" id="log">Waiting for input...</div>

  <div class="footer">
    Project Ability - Braille Module v2
  </div>
</div>

<script>
const brailleMap = {
  'a':0b000001,'b':0b000011,'c':0b001001,'d':0b011001,'e':0b010001,
  'f':0b001011,'g':0b011011,'h':0b010011,'i':0b001010,'j':0b011010,
  'k':0b000101,'l':0b000111,'m':0b001101,'n':0b011101,'o':0b010101,
  'p':0b001111,'q':0b011111,'r':0b010111,'s':0b001110,'t':0b011110,
  'u':0b100101,'v':0b100111,'w':0b111010,'x':0b101101,'y':0b111101,
  'z':0b110101,' ':0b000000
};

function sendText() {
  const input = document.getElementById('textInput');
  const text = input.value.trim();
  if (!text) return;

  fetch('/send?text=' + encodeURIComponent(text))
    .then(r => r.json())
    .then(d => {
      if (d.ok) {
        addLog('Sent: "' + text + '"');
        input.value = '';
        updateModuleDisplay(text);
      } else {
        addLog('Busy! Try again.');
      }
      pollStatus();
    })
    .catch(() => addLog('Error sending'));
}

function sendQuick(cmd) {
  fetch('/send?text=' + cmd)
    .then(r => r.json())
    .then(d => {
      addLog('Command: ' + cmd);
      pollStatus();
    })
    .catch(() => addLog('Error'));
}

function updateModuleDisplay(text) {
  text = text.toLowerCase();
  for (let i = 0; i < 4; i++) {
    const letter = document.getElementById('m' + (i+1) + 'letter');
    const dotsEl = document.getElementById('m' + (i+1) + 'dots');
    const dots = dotsEl.querySelectorAll('.dot');

    if (i < text.length) {
      const ch = text[i];
      letter.textContent = ch.toUpperCase();
      letter.classList.remove('empty');
      const pattern = brailleMap[ch] || 0;
      const dotOrder = [0, 3, 1, 4, 2, 5];
      for (let d = 0; d < 6; d++) {
        dots[d].classList.toggle('active', (pattern >> dotOrder[d]) & 1);
      }
    } else {
      letter.textContent = '-';
      letter.classList.add('empty');
      for (let d = 0; d < 6; d++) {
        dots[d].classList.remove('active');
      }
    }
  }
}

function pollStatus() {
  fetch('/status')
    .then(r => r.json())
    .then(d => {
      document.getElementById('statusText').textContent = d.status;
      const dot = document.getElementById('statusDot');
      const btn = document.getElementById('sendBtn');
      if (d.busy) {
        dot.classList.add('busy');
        btn.disabled = true;
        setTimeout(pollStatus, 1000);
      } else {
        dot.classList.remove('busy');
        btn.disabled = false;
      }
    })
    .catch(() => {});
}

function addLog(msg) {
  const log = document.getElementById('log');
  const time = new Date().toLocaleTimeString();
  log.innerHTML += '<br>[' + time + '] ' + msg;
  log.scrollTop = log.scrollHeight;
}

document.getElementById('textInput').addEventListener('keydown', function(e) {
  if (e.key === 'Enter') sendText();
});

setInterval(pollStatus, 3000);
</script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", HTML_PAGE);
}

void handleSend() {
  if (!server.hasArg("text")) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"no text\"}");
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

  Serial.print("Web input: ");
  Serial.println(text);
}

void handleStatus() {
  String json = "{\"status\":\"" + currentStatus + "\",\"busy\":" + (isBusy ? "true" : "false") + ",\"last\":\"" + lastWord + "\"}";
  server.send(200, "application/json", json);
}

void writeLeftServo(int module, int angle) {
  switch (module) {
    case 0: m1Left.write(angle); break;
    case 1: m2Left.write(angle); break;
    case 2: m3Left.write(angle); break;
    case 3: m4Left.write(angle); break;
  }
}

void writeRightServo(int module, int angle) {
  switch (module) {
    case 0: m1Right.write(angle); break;
    case 1: m2Right.write(angle); break;
    case 2: m3Right.write(angle); break;
    case 3: m4Right.write(angle); break;
  }
}

void driveModule(int module, uint8_t pattern) {
  uint8_t leftPattern = pattern & 0x07;
  uint8_t rightPattern = (pattern >> 3) & 0x07;

  rightPattern = ((rightPattern & 0x01) << 2)
                 | (rightPattern & 0x02)
                 | ((rightPattern & 0x04) >> 2);

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
  Serial.print(" R=");
  Serial.print(rightAngle);
  Serial.println("");
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

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("BRAILLE 4-MODULE DISPLAY (WiFi)");
  Serial.println();

  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("WiFi AP started: ");
  Serial.println(AP_SSID);
  Serial.print("Password: ");
  Serial.println(AP_PASS);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/send", handleSend);
  server.on("/status", handleStatus);
  server.begin();
  Serial.println("Web server started on port 80");

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  Serial.print("[Module 1] GPIO 13 + GPIO 12... ");
  m1Left.setPeriodHertz(50);
  m1Left.attach(M1_LEFT_PIN, SERVO_MIN_US, SERVO_MAX_US);
  delay(100);
  m1Right.setPeriodHertz(50);
  m1Right.attach(M1_RIGHT_PIN, SERVO_MIN_US, SERVO_MAX_US);
  delay(100);
  moduleOK[0] = m1Left.attached() && m1Right.attached();
  Serial.println(moduleOK[0] ? "OK" : "FAILED");

  Serial.print("[Module 2] GPIO 14 + GPIO 27... ");
  m2Left.setPeriodHertz(50);
  m2Left.attach(M2_LEFT_PIN, SERVO_MIN_US, SERVO_MAX_US);
  delay(100);
  m2Right.setPeriodHertz(50);
  m2Right.attach(M2_RIGHT_PIN, SERVO_MIN_US, SERVO_MAX_US);
  delay(100);
  moduleOK[1] = m2Left.attached() && m2Right.attached();
  Serial.println(moduleOK[1] ? "OK" : "FAILED");

  Serial.print("[Module 3] GPIO 26 + GPIO 25... ");
  m3Left.setPeriodHertz(50);
  m3Left.attach(M3_LEFT_PIN, SERVO_MIN_US, SERVO_MAX_US);
  delay(100);
  m3Right.setPeriodHertz(50);
  m3Right.attach(M3_RIGHT_PIN, SERVO_MIN_US, SERVO_MAX_US);
  delay(100);
  moduleOK[2] = m3Left.attached() && m3Right.attached();
  Serial.println(moduleOK[2] ? "OK" : "FAILED");

  Serial.print("[Module 4] GPIO 33 + GPIO 32... ");
  m4Left.setPeriodHertz(50);
  m4Left.attach(M4_LEFT_PIN, SERVO_MIN_US, SERVO_MAX_US);
  delay(100);
  m4Right.setPeriodHertz(50);
  m4Right.attach(M4_RIGHT_PIN, SERVO_MIN_US, SERVO_MAX_US);
  delay(100);
  moduleOK[3] = m4Left.attached() && m4Right.attached();
  Serial.println(moduleOK[3] ? "OK" : "FAILED");

  int okCount = 0;
  for (int m = 0; m < NUM_MODULES; m++) {
    if (moduleOK[m]) okCount++;
  }

  Serial.print(okCount);
  Serial.println("/4 modules attached.");

  Serial.println("Moving all servos to home (22)...");
  homeAllServos();
  delay(1000);
  Serial.println("All servos at home.");

  Serial.println();
  Serial.println("READY!");
  Serial.println("Connect to WiFi: " + String(AP_SSID));
  Serial.println("Open browser: http://192.168.4.1");
  Serial.println("Serial commands also work: test, home, sweep, diag");
  Serial.println();
}

void loop() {
  server.handleClient();

  if (hasPending) {
    hasPending = false;
    processInput(pendingWord);
  }

  if (Serial.available() > 0) {
    String serialInput = Serial.readStringUntil('\n');
    serialInput.trim();
    if (serialInput.length() > 0) {
      Serial.print("Serial input: ");
      Serial.println(serialInput);
      processInput(serialInput);
    }
  }
}

void processInput(String text) {
  if (isBusy) return;
  isBusy = true;

  if (text.equalsIgnoreCase("test")) {
    currentStatus = "Running test...";
    runTestAllLetters();
  } else if (text.equalsIgnoreCase("home")) {
    currentStatus = "Homing...";
    homeAllServos();
    Serial.println("All servos at 22.");
  } else if (text.equalsIgnoreCase("sweep")) {
    currentStatus = "Sweeping...";
    runSweepTest();
  } else if (text.equalsIgnoreCase("diag")) {
    currentStatus = "Diagnostics...";
    runDiagnostics();
  } else {
    lastWord = text;
    currentStatus = "Displaying: " + text;
    displayWord(text);
  }

  currentStatus = "Ready";
  isBusy = false;
  Serial.println("Ready for next input...");
}

void displayWord(String word) {
  int totalChars = word.length();
  int totalBatches = (totalChars + NUM_MODULES - 1) / NUM_MODULES;

  Serial.print("Word: ");
  Serial.print(word);
  Serial.print(" -> ");
  Serial.print(totalChars);
  Serial.print(" chars, ");
  Serial.print(totalBatches);
  Serial.println(" batch(es)");

  for (int batch = 0; batch < totalBatches; batch++) {
    int startIdx = batch * NUM_MODULES;
    int charsInBatch = min(NUM_MODULES, totalChars - startIdx);

    currentStatus = "Batch " + String(batch + 1) + "/" + String(totalBatches);

    Serial.print("Batch ");
    Serial.print(batch + 1);
    Serial.print("/");
    Serial.println(totalBatches);

    uint8_t patterns[NUM_MODULES];
    bool    active[NUM_MODULES];

    for (int m = 0; m < NUM_MODULES; m++) {
      if (m < charsInBatch) {
        char c = word.charAt(startIdx + m);
        patterns[m] = charToBraille(c);
        active[m] = (patterns[m] != 0xFF) && moduleOK[m];

        Serial.print("  Module ");
        Serial.print(m + 1);
        Serial.print(": '");
        Serial.print(c);
        Serial.print("' -> ");

        if (!moduleOK[m]) {
          Serial.println("(not attached)");
        } else if (patterns[m] == 0xFF) {
          Serial.println("(unsupported)");
        } else {
          printBrailleInfo(c, patterns[m]);
        }
      } else {
        patterns[m] = 0b000000;
        active[m] = moduleOK[m];
      }
    }

    Serial.println("  Driving modules...");

    for (int m = 0; m < NUM_MODULES; m++) {
      if (active[m]) {
        driveModule(m, patterns[m]);
      }
    }

    delay(SERVO_MOVE_TIME);

    Serial.print("  Holding ");
    Serial.print(CHAR_HOLD_TIME / 1000);
    Serial.println(" seconds...");
    delay(CHAR_HOLD_TIME);

    Serial.println("  Returning to home...");
    homeAllServos();

    if (batch < totalBatches - 1) {
      Serial.print("  Pause ");
      Serial.print(BATCH_GAP_TIME / 1000);
      Serial.println(" seconds...");
      delay(BATCH_GAP_TIME);
    }

    Serial.println();
  }

  Serial.println("Word complete!");
}

void printBrailleInfo(char c, uint8_t pattern) {
  Serial.print("0b");
  for (int b = 5; b >= 0; b--) {
    Serial.print((pattern >> b) & 1);
  }

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

void runTestAllLetters() {
  Serial.println("Running a-z in groups of 4...");

  for (int i = 0; i < 26; i += NUM_MODULES) {
    int charsInBatch = min(NUM_MODULES, 26 - i);

    for (int m = 0; m < charsInBatch; m++) {
      char c = 'a' + i + m;
      uint8_t pattern = charToBraille(c);

      Serial.print("  M");
      Serial.print(m + 1);
      Serial.print(": ");
      Serial.print(c);
      Serial.print(" -> ");
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

    writeLeftServo(m, 90);
    delay(500);
    writeLeftServo(m, 22);
    delay(500);

    writeRightServo(m, 90);
    delay(500);
    writeRightServo(m, 22);
    delay(500);

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

    for (int angle = 0; angle <= 180; angle += 10) {
      writeLeftServo(m, angle);
      delay(150);
    }
    for (int angle = 180; angle >= 0; angle -= 10) {
      writeLeftServo(m, angle);
      delay(150);
    }
    Serial.println("  Left done.");

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
