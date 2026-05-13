#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ESP32Servo.h>
#include <time.h>
extern Servo servoH;
extern Servo servoV;

// WiFi
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// Firebase
#define API_KEY ""
#define DATABASE_URL ""

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ================= SUN VARIABLES =================
#define DEG_TO_RAD 0.01745329251

float Lon = 10.2 * DEG_TO_RAD;
float Lat = 36.8 * DEG_TO_RAD;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 0;

int sun_azimuth;
int sun_elevation;

// ================= FUNCTIONS =================
void Calculate_Sun_Position(
  int hour,
  int minute,
  int second,
  int day,
  int month,
  int year
);

long JulianDate(int year, int month, int day);

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
  config.api_key ="";
  config.database_url = "";
  // 🔥 MUST BE INSIDE setup()
  auth.user.email = "";
  auth.user.password = "";

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  servoH.attach(servoHPin);
  servoV.attach(servoVPin, 500, 2400);
  // ===== TIME =====
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

}



void firebaseloop() {
  
  // ==================================================
  // ================== LDR READING ===================
  // ==================================================
  int ldrValue = analogRead(LDR1);

  Serial.print("LDR Value: ");
  Serial.println(ldrValue);

  // Send LDR to Firebase
  if (Firebase.RTDB.setInt(&fbdo, "/sensors/ldr", ldrValue)) {
    Serial.println("LDR Sent OK");
  } else {
    Serial.println("Failed to send LDR");
    Serial.println(fbdo.errorReason());
  }

  // ==================================================
  // ============ READ SERVO FROM FIREBASE ============
  // ==================================================
  if (Firebase.RTDB.getInt(&fbdo, "/servo/orientation")) {

    int angle = fbdo.intData();

    // Safety limits
    angle = constrain(angle, 0, 180);

    Serial.print("Servo Angle: ");
    Serial.println(angle);

    servoH.write(angle);

  } else {

    Serial.println("Failed to read servo angle");
    Serial.println(fbdo.errorReason());
  }

  // ==================================================
  // ================= SUN POSITION ===================
  // ==================================================
  struct tm timeinfo;

  if (getLocalTime(&timeinfo)) {

    Calculate_Sun_Position(
      timeinfo.tm_hour,
      timeinfo.tm_min,
      timeinfo.tm_sec,
      timeinfo.tm_mday,
      timeinfo.tm_mon + 1,
      timeinfo.tm_year + 1900
    );

    Serial.print("Sun Azimuth: ");
    Serial.println(sun_azimuth);

    Serial.print("Sun Elevation: ");
    Serial.println(sun_elevation);

    // Send solar data to Firebase
    Firebase.RTDB.setInt(&fbdo, "/solar/azimuth", sun_azimuth);
    Firebase.RTDB.setInt(&fbdo, "/solar/elevation", sun_elevation);

  } else {

    Serial.println("Failed to obtain time");
  }

  delay(2000);


}
// ======================================================
// ================= SUN CALCULATION ====================
// ======================================================
void Calculate_Sun_Position(
  int hour,
  int minute,
  int second,
  int day,
  int month,
  int year
) {

  float T, JD_frac, L0, M, C, L_true;
  float GrHrAngle, Obl, RA, Decl, HrAngle;

  long JDate = JulianDate(year, month, day);

  JD_frac =
    (hour + minute / 60.0 + second / 3600.0) / 24.0 - 0.5;

  T = (JDate - 2451545 + JD_frac) / 36525.0;

  L0 = DEG_TO_RAD *
       fmod(280.46645 + 36000.76983 * T, 360);

  M = DEG_TO_RAD *
      fmod(357.5291 + 35999.0503 * T, 360);

  C = DEG_TO_RAD *
      (
        (1.9146 - 0.004847 * T) * sin(M) +
        (0.019993 - 0.000101 * T) * sin(2 * M) +
        0.00029 * sin(3 * M)
      );

  Obl = DEG_TO_RAD * 23.44;

  GrHrAngle =
    280.46061837 +
    360.98564736629 *
    (JDate - 2451545 + JD_frac);

  GrHrAngle = fmod(GrHrAngle, 360.0);

  L_true = fmod(C + L0, 2 * PI);

  RA = atan2(
    sin(L_true) * cos(Obl),
    cos(L_true)
  );

  Decl = asin(
    sin(Obl) * sin(L_true)
  );

  HrAngle =
    DEG_TO_RAD * GrHrAngle + Lon - RA;

  float elevation = asin(
    sin(Lat) * sin(Decl) +
    cos(Lat) * cos(Decl) * cos(HrAngle)
  );

  float azimuth = PI + atan2(
    sin(HrAngle),
    cos(HrAngle) * sin(Lat) -
    tan(Decl) * cos(Lat)
  );

  sun_azimuth = azimuth / DEG_TO_RAD;
  sun_elevation = elevation / DEG_TO_RAD;
}

// ======================================================
// =================== JULIAN DATE ======================
// ======================================================
long JulianDate(int year, int month, int day) {

  if (month <= 2) {
    year--;
    month += 12;
  }

  int A = year / 100;
  int B = 2 - A + A / 4;

  long JD =
    (long)(365.25 * (year + 4716)) +
    (int)(30.6001 * (month + 1)) +
    day + B - 1524;

  return JD;
}
