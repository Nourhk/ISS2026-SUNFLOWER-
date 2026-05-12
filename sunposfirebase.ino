#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ESP32Servo.h>
#include <time.h>

// ================= SERVO =================
Servo myServo;
int servoPin = 18;

// ================= WIFI =================
#define WIFI_SSID "Ooredoo-ALHN-022C"
#define WIFI_PASSWORD "qCKMZW7eKx"

// ================= FIREBASE =================
#define API_KEY "AIzaSyBY5dSsjRSH6i-8rvkXooKM3kjNGzL-i1k"
#define DATABASE_URL "https://esp32-3740e-default-rtdb.europe-west1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ================= LDR =================
int ldrPin = 34;

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
void Calculate_Sun_Position(int hour, int minute, int second,
                            int day, int month, int year);

long JulianDate(int year, int month, int day);

// ================= SETUP =================
void setup() {

  Serial.begin(115200);

  myServo.attach(servoPin);

  // ===== WIFI =====
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi Connected");
  Serial.println(WiFi.localIP());

  // ===== TIME =====
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // ===== FIREBASE =====
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = "nourhajkacem2006@gmail.com";
  auth.user.password = "ISS1234";

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase Started");
}


// ================= LOOP =================
void loop() {

  // ===== LDR READ =====
  int ldrValue = analogRead(ldrPin);

  Serial.print("LDR: ");
  Serial.println(ldrValue);

  Firebase.RTDB.setInt(&fbdo, "/sensors/ldr", ldrValue);

  // ===== READ SERVO FROM FIREBASE =====
  if (Firebase.RTDB.getInt(&fbdo, "/servo/orientation")) {

    int angle = fbdo.intData();
    myServo.write(angle);

    Serial.print("Servo Angle: ");
    Serial.println(angle);
  }

  // ===== TIME =====
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

    Serial.print("Azimuth: ");
    Serial.println(sun_azimuth);

    Serial.print("Elevation: ");
    Serial.println(sun_elevation);

    Firebase.RTDB.setInt(&fbdo, "/solar/azimuth", sun_azimuth);
    Firebase.RTDB.setInt(&fbdo, "/solar/elevation", sun_elevation);
  }

  delay(2000);
}

// ================= SUN CALC =================
void Calculate_Sun_Position(int hour, int minute, int second,
                            int day, int month, int year) {

  float T, JD_frac, L0, M, e, C, L_true;
  float GrHrAngle, Obl, RA, Decl, HrAngle;

  long JDate = JulianDate(year, month, day);

  JD_frac = (hour + minute / 60.0 + second / 3600.0) / 24.0 - 0.5;

  T = (JDate - 2451545 + JD_frac) / 36525.0;

  L0 = DEG_TO_RAD * fmod(280.46645 + 36000.76983 * T, 360);
  M  = DEG_TO_RAD * fmod(357.5291 + 35999.0503 * T, 360);

  C = DEG_TO_RAD * ((1.9146 - 0.004847 * T) * sin(M)
      + (0.019993 - 0.000101 * T) * sin(2 * M)
      + 0.00029 * sin(3 * M));

  Obl = DEG_TO_RAD * 23.44;

  GrHrAngle = 280.46061837 + 360.98564736629 * (JDate - 2451545 + JD_frac);
  GrHrAngle = fmod(GrHrAngle, 360.0);

  L_true = fmod(C + L0, 2 * PI);

  RA = atan2(sin(L_true) * cos(Obl), cos(L_true));
  Decl = asin(sin(Obl) * sin(L_true));

  HrAngle = DEG_TO_RAD * GrHrAngle + Lon - RA;

  float elevation = asin(
    sin(Lat) * sin(Decl) +
    cos(Lat) * cos(Decl) * cos(HrAngle)
  );

  float azimuth = PI + atan2(
    sin(HrAngle),
    cos(HrAngle) * sin(Lat) - tan(Decl) * cos(Lat)
  );

  sun_azimuth = azimuth / DEG_TO_RAD;
  sun_elevation = elevation / DEG_TO_RAD;
}

// ================= JULIAN DATE =================
long JulianDate(int year, int month, int day) {

  if (month <= 2) {
    year--;
    month += 12;
  }

  int A = year / 100;
  int B = 2 - A + A / 4;

  long JD =
    (long)(365.25 * (year + 4716))
    + (int)(30.6001 * (month + 1))
    + day + B - 1524;

  return JD;
}