#include <ESP32Audiometer.h>

ESP32Audiometer audiometer;

void setup() {
  Serial.begin(115200);
  if (!audiometer.begin(26, 25, 27)) {
    Serial.println("I2S initialization failed");
    while (true) delay(1000);
  }
  audiometer.setFrequency(1000);
  audiometer.setLevelDb(-24); // Relative digital dB, NOT dB HL.
  audiometer.setWarble(false);
}

void loop() {
  // 500 ms on, 500 ms off. Keep calling process() to maintain I2S output.
  audiometer.setPresent((millis() % 1000) < 500);
  audiometer.process();
}
