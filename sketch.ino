
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

// LED PINS
#define GREEN_LED 27
#define YELLOW_LED 26
#define RED_LED 25

// BUZZER
#define BUZZER 23

// I2C PINS
#define SDA_PIN 21
#define SCL_PIN 22

// OLED CONFIGURATION
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_MPU6050 mpu;

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// VIBRATION THRESHOLDS
#define MODERATE_LIMIT 1.5
#define STRONG_LIMIT 3.5

float vibration = 0;
float totalAcceleration = 0;

int alertLevel = 0;

unsigned long lastBlink = 0;
unsigned long lastDisplay = 0;
unsigned long lastBuzzer = 0;

bool blinkState = false;

void setup() {

  Serial.begin(115200);

  // LED SETUP
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);

  // I2C SETUP
  Wire.begin(SDA_PIN, SCL_PIN);

  // OLED INITIALIZATION
  if (!display.begin(
        SSD1306_SWITCHCAPVCC, 0x3C)) {

    Serial.println("OLED NOT FOUND!");
    while (true) {
      delay(100);
    }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 20);
  display.println("EARTHQUAKE");
  display.setCursor(25, 35);
  display.println("DETECTOR");
  display.display();

  delay(1500);

  // MPU6050 INITIALIZATION
  if (!mpu.begin()) {

    Serial.println("MPU6050 NOT FOUND!");

    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("MPU6050 ERROR");
    display.display();

    while (true) {
      delay(100);
    }
  }

  mpu.setAccelerometerRange(
    MPU6050_RANGE_8_G
  );

  mpu.setGyroRange(
    MPU6050_RANGE_500_DEG
  );

  mpu.setFilterBandwidth(
    MPU6050_BAND_21_HZ
  );

  Serial.println("==========================");
  Serial.println("EARTHQUAKE DETECTOR READY");
  Serial.println("==========================");
}

void loop() {

  // READ SENSOR DATA
  sensors_event_t a, g, temp;

  mpu.getEvent(&a, &g, &temp);

  // CALCULATE TOTAL ACCELERATION
  totalAcceleration = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z
  );

  // REMOVE APPROXIMATE GRAVITY COMPONENT
  vibration = fabs(totalAcceleration - 9.81);

  // DETECT VIBRATION LEVEL
  if (vibration >= STRONG_LIMIT) {
    alertLevel = 2;
  }
  else if (vibration >= MODERATE_LIMIT) {
    alertLevel = 1;
  }
  else {
    alertLevel = 0;
  }

  unsigned long now = millis();

  // NORMAL CONDITION
  if (alertLevel == 0) {

    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);

    blinkState = false;
  }

  // MODERATE VIBRATION
  else if (alertLevel == 1) {

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);

    if (now - lastBlink >= 500) {

      lastBlink = now;
      blinkState = !blinkState;

      digitalWrite(YELLOW_LED, blinkState);
    }
  }

  // STRONG VIBRATION
  else {

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);

    if (now - lastBlink >= 150) {

      lastBlink = now;
      blinkState = !blinkState;

      digitalWrite(RED_LED, blinkState);
    }

    if (now - lastBuzzer >= 500) {

      lastBuzzer = now;
      tone(BUZZER, 2000, 200);
    }
  }

  // SERIAL MONITOR
  Serial.print("Acceleration: ");
  Serial.print(totalAcceleration, 2);

  Serial.print(" | Vibration: ");
  Serial.print(vibration, 2);

  Serial.print(" | Status: ");

  if (alertLevel == 0) {
    Serial.println("NORMAL");
  }
  else if (alertLevel == 1) {
    Serial.println("MODERATE VIBRATION");
  }
  else {
    Serial.println("STRONG ALERT");
  }

  // OLED UPDATE
  if (now - lastDisplay >= 250) {

    lastDisplay = now;

    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("EARTHQUAKE MONITOR");

    display.drawLine(0, 12, 127, 12,
                     SSD1306_WHITE);

    display.setCursor(0, 19);
    display.print("Accel: ");
    display.print(totalAcceleration, 2);
    display.println(" m/s2");

    display.setCursor(0, 32);
    display.print("Vibration: ");
    display.print(vibration, 2);

    display.setCursor(0, 47);
    display.print("STATUS: ");

    if (alertLevel == 0) {
      display.println("NORMAL");
    }
    else if (alertLevel == 1) {
      display.println("MODERATE");
    }
    else {
      display.println("STRONG ALERT");
    }

    display.display();
  }

  delay(50);
}


