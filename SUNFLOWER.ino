#include <ESP32Servo.h>

// ===================== LDR PINS =====================
const int LDR1 = 34;
const int LDR2 = 35;
const int LDR3 = 32;
const int LDR4 = 33;

// ===================== SERVOS =====================
Servo servoH;
Servo servoV;

const int servoHPin = 23;
const int servoVPin = 18;

// ===================== TIME =====================
#define uS_TO_S_FACTOR 1000000ULL
#define SLEEP_TIME 720  // 12 minutes = 720 seconds

// ===================== YOUR FUNCTIONS =====================
void ldrsetup() {
  // your setup code
}

void sunsetup() {
  // your setup code
}

void ldrloop() {
  // your LDR logic
  Serial.println("LDR running...");
}

void sunloop() {
  // your sun tracking logic
  Serial.println("SUN running...");
}

// ===================== SETUP =====================
void setup() {

  Serial.begin(115200);
  delay(1000);

  servoH.attach(servoHPin);
  servoV.attach(servoVPin);

  ldrsetup();
  sunsetup();

  Serial.println("Running sensors...");

  ldrloop();
  sunloop();

  Serial.println("Going to sleep for 12 minutes");

  esp_sleep_enable_timer_wakeup(SLEEP_TIME * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

// ===================== LOOP (NOT USED) =====================
void loop() {
  // never used because ESP sleeps
}