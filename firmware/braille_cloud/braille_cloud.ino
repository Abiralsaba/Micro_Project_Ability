#define BUTTON_PIN 14

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.println("Button Ready");
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Button Pressed");

    while (digitalRead(BUTTON_PIN) == LOW) {
      delay(10);
    }

    delay(30);   // debounce
  }
}