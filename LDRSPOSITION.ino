#include <ESP32Servo.h>


// ===================== SERVO =====================
extern Servo servoH;
extern const int LDR1 ;
extern const int LDR2 ;
extern const int LDR3;
extern const int LDR4 ;


// ===================== PARAMETERS =====================
int deadZone = 30;
const int LIGHT_THRESHOLD = 400;

// Number of readings for averaging
const int samples = 6;

// Buffer smoothing
const int SMOOTH_WINDOW = 5;

int topBuffer[SMOOTH_WINDOW];
int bottomBuffer[SMOOTH_WINDOW];

int bufferIndex = 0;

// ===================== CONTINUOUS SERVO VALUES =====================
int STOP = 90;

int UP = 33;
int DOWN = 160;

// Movement duration
int moveTime = 160;

// ===================== SMOOTH FUNCTION =====================
int smooth(int *buffer, int newValue)
{
  buffer[bufferIndex] = newValue;

  long sum = 0;

  for (int i = 0; i < SMOOTH_WINDOW; i++)
  {
    sum += buffer[i];
  }

  return sum / SMOOTH_WINDOW;
}

// ===================== SETUP =====================
void ldrsetup()
{
  Serial.begin(115200);

  servoH.attach(servoHPin, 1000, 2000);

  servoH.write(STOP);

  // Initialize buffers
  for (int i = 0; i < SMOOTH_WINDOW; i++)
  {
    topBuffer[i] = 0;
    bottomBuffer[i] = 0;
  }

  delay(1000);
}

// ===================== LOOP =====================
void ldrloop()
{
  long sumErrorH = 0;
  long sumLight = 0;

  // ===================== MULTIPLE READINGS =====================
  for (int i = 0; i < samples; i++)
  {
    // ---------- READ SENSORS ----------
    int TopR = analogRead(LDR1);
    int TopL = analogRead(LDR2);
    int BotR = analogRead(LDR3);
    int BotL = analogRead(LDR4);

    // ---------- RAW AVERAGES ----------
    int topRaw = (TopR + TopL) / 2;
    int bottomRaw = (BotR + BotL) / 2;

    // ---------- BUFFER SMOOTHING ----------
    int top = smooth(topBuffer, topRaw);
    int bottom = smooth(bottomBuffer, bottomRaw);

    bufferIndex = (bufferIndex + 1) % SMOOTH_WINDOW;

    // ---------- TOTAL LIGHT ----------
    int avgAll = (TopR + TopL + BotR + BotL) / 4;

    // ---------- ERROR ----------
    int errorH = top - bottom;

    // ---------- SUM ----------
    sumErrorH += errorH;
    sumLight += avgAll;

    delay(10);
  }

  // ===================== FINAL AVERAGES =====================
  int avgErrorH = sumErrorH / samples;
  int avgLight = sumLight / samples;

  // ===================== DEBUG =====================
  Serial.println("------ DEBUG ------");

  Serial.print("Avg Error H: ");
  Serial.println(avgErrorH);

  Serial.print("Avg Light: ");
  Serial.println(avgLight);

  // ===================== LIGHT CHECK =====================
  if (avgLight > LIGHT_THRESHOLD)
  {
    // ===================== MOTOR CONTROL =====================
    if (avgErrorH > deadZone)
    {
      Serial.println("Moving DOWN");

      servoH.write(DOWN);
      delay(moveTime);

      servoH.write(STOP);
    }

    else if (avgErrorH < -deadZone)
    {
      Serial.println("Moving UP");

      servoH.write(UP);
      delay(moveTime);

      servoH.write(STOP);
    }

    else
    {
      servoH.write(STOP);

      Serial.println("STOP");
    }
  }

  else
  {
    servoH.write(STOP);

    Serial.println("LOW LIGHT - STOP");
  }

  delay(50);
}