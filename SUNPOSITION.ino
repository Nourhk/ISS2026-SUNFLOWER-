#include <WiFi.h>
#include <ESP32Servo.h>
#include <time.h>

// ====== SERVO ======
extern Servo servoH;
extern Servo servoV;

// ====== LOCATION (Tunisia) ======
#define DEG_TO_RAD 0.01745329251

float Lon = 10.2 * DEG_TO_RAD;
float Lat = 36.8 * DEG_TO_RAD;

// ====== TIME SETTINGS ======
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 0;

// ====== SUN VARIABLES ======
int sun_azimuth, sun_elevation;

// ====== FUNCTION DECLARATIONS ======
void Calculate_Sun_Position(int hour, int minute, int second,
                            int day, int month, int year);

long JulianDate(int year, int month, int day);

// =========================

void sunsetup() {

  // Attach servo
  servoV.attach(servoVPin);

  // Connect WiFi
  WiFi.begin("Ooredoo-ALHN-022C", "qCKMZW7eKx");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");

  // Internet time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

// =========================

void sunloop() {

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to get time");
    return;
  }

  // ===== CALCULATE SUN POSITION =====
  Calculate_Sun_Position(
    timeinfo.tm_hour,
    timeinfo.tm_min,
    timeinfo.tm_sec,
    timeinfo.tm_mday,
    timeinfo.tm_mon + 1,
    timeinfo.tm_year + 1900
  );

  // ===== MAP AZIMUTH TO SERVO =====
  int panAngle = map(sun_azimuth, 0, 360, 0, 180);

  // Safety limits
  panAngle = constrain(panAngle, 10, 170);

  // ===== NIGHT RESET =====
  if (sun_elevation <= 0) {
    panAngle = 10; // East/start position at night
  }

  // ===== MOVE SERVO =====
  servoV.write(180-(panAngle-90));

  // ===== DEBUG =====
  Serial.println("-----");
  Serial.print("Azimuth: ");
  Serial.println(sun_azimuth);

  Serial.print("Elevation: ");
  Serial.println(sun_elevation);

  Serial.print("Servo Angle: ");
  Serial.println(panAngle);

  delay(5000);
}

// =========================
// SUN POSITION CALCULATION
// =========================

void Calculate_Sun_Position(int hour, int minute, int second,
                            int day, int month, int year) {

  float T, JD_frac, L0, M, e, C, L_true;
  float GrHrAngle, Obl, RA, Decl, HrAngle;

  long JDate, JDx;

  JDate = JulianDate(year, month, day);

  JD_frac =
    (hour + minute / 60.0 + second / 3600.0) / 24.0 - 0.5;

  T = (JDate - 2451545 + JD_frac) / 36525.0;

  L0 =
    DEG_TO_RAD *
    fmod(280.46645 + 36000.76983 * T, 360);

  M =
    DEG_TO_RAD *
    fmod(357.5291 + 35999.0503 * T, 360);

  e = 0.016708617 - 0.000042037 * T;

  C =
    DEG_TO_RAD *
    ((1.9146 - 0.004847 * T) * sin(M)
    + (0.019993 - 0.000101 * T) * sin(2 * M)
    + 0.00029 * sin(3 * M));

  Obl = DEG_TO_RAD * 23.44;

  JDx = JDate - 2451545;

  GrHrAngle =
    280.46061837 +
    360.98564736629 * (JDx + JD_frac);

  GrHrAngle = fmod(GrHrAngle, 360.0);

  L_true = fmod(C + L0, 2 * PI);

  RA =
    atan2(
      sin(L_true) * cos(Obl),
      cos(L_true)
    );

  Decl =
    asin(sin(Obl) * sin(L_true));

  HrAngle =
    DEG_TO_RAD * GrHrAngle + Lon - RA;

  float elevation =
    asin(
      sin(Lat) * sin(Decl)
      + cos(Lat) * cos(Decl) * cos(HrAngle)
    );

  float azimuth =
    PI + atan2(
      sin(HrAngle),
      cos(HrAngle) * sin(Lat)
      - tan(Decl) * cos(Lat)
    );

  sun_azimuth = azimuth / DEG_TO_RAD;
  sun_elevation = elevation / DEG_TO_RAD;
}

// =========================

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