#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ESP32Servo.h>

extern Servo servoH;
extern Servo servoV;

// WiFi
#define WIFI_SSID "Ooredoo-ALHN-022C"
#define WIFI_PASSWORD "qCKMZW7eKx"

// Firebase
#define API_KEY "AIzaSyBY5dSsjRSH6i-8rvkXooKM3kjNGzL-i1k"
#define DATABASE_URL "https://esp32-3740e-default-rtdb.europe-west1.firebasedatabase.app"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;


void firebasesetup() {

  // Connect WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected!");

  // Firebase config
  config.api_key ="AIzaSyBY5dSsjRSH6i-8rvkXooKM3kjNGzL-i1k";
  config.database_url = "https://esp32-3740e-default-rtdb.europe-west1.firebasedatabase.app";
  // 🔥 MUST BE INSIDE setup()
  auth.user.email = "nourhajkacem2006@gmail.com";
  auth.user.password = "ISS1234";

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  servoH.attach(servoHPin);
}



void firebaseloop() {
  int ldrValue = analogRead(LDR1);

  Serial.print("LDR: ");
  Serial.println(ldrValue);

  // Send to Firebase
  if (Firebase.RTDB.setInt(&fbdo, "/sensors/ldr", ldrValue)) {
    Serial.println("Sent OK");
  } else {
    Serial.println("FAILED");
    Serial.println(fbdo.errorReason());
  }




  int lastAngle = -1;
  // Read servo angle from Firebase
if (Firebase.RTDB.getInt(&fbdo, "/servo/orientation")) {

  int angle = fbdo.intData();

  Serial.print("Servo angle: ");
  Serial.println(angle);

  servoH.write(angle);  // move motor

} else {
  Serial.println("Failed to read servo");
  Serial.println(fbdo.errorReason());
}

  delay(2000);


}