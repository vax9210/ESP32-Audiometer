#pragma once
#include <Arduino.h>
#include <ESP_I2S.h>

// Experimental 16-bit stereo I2S tone generator for Arduino-ESP32 3.x.
// Levels are relative digital amplitude, NOT calibrated dB HL or dB SPL.
class ESP32Audiometer {
public:
  explicit ESP32Audiometer(uint32_t sampleRate = 44100);
  bool begin(int bclkPin, int wsPin, int doutPin);
  void setFrequency(float hz);
  void setLevelDb(float relativeDb);
  void setWarble(bool enabled);
  void setWarbleParameters(float rateHz, float depthFraction);
  void setPresent(bool enabled);
  void process(); // Continuously call from loop(); writes one block of audio.
  float frequency() const { return frequencyHz_; }
  float levelDb() const { return levelDb_; }
  bool warble() const { return warbleEnabled_; }
  bool presenting() const { return present_; }
private:
  static const size_t kBufferFrames = 256;
  I2SClass i2s_;
  uint32_t sampleRate_;
  bool begun_ = false;
  float frequencyHz_ = 1000.0f;
  float levelDb_ = -12.0f;
  float phase_ = 0.0f;
  float warblePhase_ = 0.0f;
  float envelope_ = 0.0f;
  bool present_ = false;
  bool warbleEnabled_ = false;
  float warbleRateHz_ = 5.0f;
  float warbleDepth_ = 0.05f;
  int16_t buffer_[kBufferFrames * 2];
};
