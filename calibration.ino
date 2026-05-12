#include <math.h>

const int x_out = 34;
const int y_out = 35;
const int z_out = 32;

long x_sum = 0;
long y_sum = 0;
long z_sum = 0;

const int samples = 200;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  Serial.println("Keep sensor FLAT and do NOT move it...");
  delay(3000);

  for (int i = 0; i < samples; i++) {
    x_sum += analogRead(x_out);
    y_sum += analogRead(y_out);
    z_sum += analogRead(z_out);
    delay(10);
  }

  int x_offset = x_sum / samples;
  int y_offset = y_sum / samples;
  int z_offset = z_sum / samples;

  Serial.println("=== CALIBRATION RESULTS ===");
  Serial.print("X_offset = "); Serial.println(x_offset);
  Serial.print("Y_offset = "); Serial.println(y_offset);
  Serial.print("Z_offset = "); Serial.println(z_offset);

  Serial.println("Copy these values into main code!");
}

void loop() {

}