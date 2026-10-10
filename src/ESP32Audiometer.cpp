#include "ESP32Audiometer.h"
#include <math.h>

namespace {
constexpr float kTwoPi = 6.2831853071795864769f;
constexpr float kMaxAmplitude = 12000.0f;
constexpr float kRampSeconds = 0.020f;
}

ESP32Audiometer::ESP32Audiometer(uint32_t sampleRate)
  : sampleRate_(sampleRate) {}

bool ESP32Audiometer::begin(int bclkPin, int wsPin, int doutPin) {
  if (sampleRate_ == 0) return false;
  i2s_.setPins(bclkPin, wsPin, doutPin);
  begun_ = i2s_.begin(I2S_MODE_STD, sampleRate_,
                      I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
  return begun_;
}

void ESP32Audiometer::setFrequency(float hz) {
  // Keep below Nyquist; high frequencies still require acoustic verification.
  if (isfinite(hz) && hz > 0.0f && hz < sampleRate_ * 0.5f)
    frequencyHz_ = hz;
}
void ESP32Audiometer::setLevelDb(float relativeDb) {
  if (isfinite(relativeDb)) levelDb_ = constrain(relativeDb, -90.0f, 0.0f);
}
void ESP32Audiometer::setWarble(bool enabled) { warbleEnabled_ = enabled; }
void ESP32Audiometer::setWarbleParameters(float rateHz, float depthFraction) {
  if (isfinite(rateHz) && rateHz >= 0.0f && rateHz <= 100.0f)
    warbleRateHz_ = rateHz;
  if (isfinite(depthFraction) && depthFraction >= 0.0f && depthFraction <= 0.20f)
    warbleDepth_ = depthFraction;
}
void ESP32Audiometer::setPresent(bool enabled) { present_ = enabled; }

void ESP32Audiometer::process() {
  if (!begun_) return;
  const float targetAmplitude = kMaxAmplitude * powf(10.0f, levelDb_ / 20.0f);
  const float rampStep = 1.0f / (sampleRate_ * kRampSeconds);
  const float modStep = kTwoPi * warbleRateHz_ / sampleRate_;
  for (size_t i = 0; i < kBufferFrames; ++i) {
    envelope_ = constrain(envelope_ + (present_ ? rampStep : -rampStep), 0.0f, 1.0f);
    const float currentFreq = warbleEnabled_
      ? frequencyHz_ * (1.0f + warbleDepth_ * sinf(warblePhase_))
      : frequencyHz_;
    const int16_t sample = static_cast<int16_t>(sinf(phase_) * targetAmplitude * envelope_);
    buffer_[2 * i] = sample;
    buffer_[2 * i + 1] = sample; // Both channels deliberately identical.
    phase_ += kTwoPi * currentFreq / sampleRate_;
    if (phase_ >= kTwoPi) phase_ -= kTwoPi;
    warblePhase_ += modStep;
    if (warblePhase_ >= kTwoPi) warblePhase_ -= kTwoPi;
  }
  i2s_.write(reinterpret_cast<uint8_t *>(buffer_), sizeof(buffer_));
}
