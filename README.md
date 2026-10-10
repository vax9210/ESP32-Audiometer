# ESP32Audiometer (experimental library)

A reusable Arduino-ESP32 (3.x) I2S **pure-tone and warble-tone generator** extracted from the original ESP32 + SSD1306 demonstration sketch.

> **Educational and experimental use only.** This is not an audiometer certified or calibrated for clinical hearing evaluation. Levels are **relative digital dB**, not dB HL or dB SPL. Audio output must be validated with appropriate transducers and measurement equipment. Output is currently the **same mono signal in both stereo slots**.

## Requirements

- ESP32 board with Arduino-ESP32 **3.x** (uses `ESP_I2S.h`)
- External I2S-compatible audio DAC/amplifier; the original pins are **BCLK 26, WS/LRCLK 25, DOUT 27**
- An appropriate speaker/transducer, at safe levels
- For `OLED_Audiometer` example: `Adafruit GFX Library`, `Adafruit SSD1306`, 128x64 SSD1306 I2C OLED at `0x3C` using SDA 21, SCL 22

## Arduino examples

Open **File → Examples → ESP32Audiometer → BasicTone** (or `OLED_Audiometer`).

```cpp
#include <ESP32Audiometer.h>
ESP32Audiometer audio;
void setup() {
  if (!audio.begin(26, 25, 27)) while (true) delay(1000);
  audio.setFrequency(1000);     // Hz
  audio.setLevelDb(-24);        // relative digital dB, NOT dB HL
  audio.setWarble(true);        // 5 Hz FM, ±5% by default
}
void loop() {
  audio.setPresent((millis() % 1000) < 500);
  audio.process();             // must be called continuously
}
```

## API

- `begin(bclkPin, wsPin, doutPin)` — initialize 16-bit, 44.1 kHz stereo I2S (default)
- `setFrequency(hz)` — adjust frequency below Nyquist
- `setLevelDb(relativeDb)` — digital gain in the range -90 to 0 dB
- `setWarble(enabled)` — enable frequency modulation
- `setWarbleParameters(rateHz, depthFraction)` — customize rate/depth
- `setPresent(enabled)` — enable the amplitude envelope (20 ms ramp)
- `process()` — fill and write the next 256 frames; call repeatedly

## Notes and limitations

- This library does not implement ear-specific channels, audiometric threshold algorithms, ANSI/IEC measurement compliance, or transducer-specific calibration.
- Current 16 kHz output at 44.1 kHz sampling can have significant system-dependent artifacts; verify your complete audio chain and use appropriate antialias filtering.
- Default output uses identical L/R samples, so it is **not independently addressable**.
- The OLED example closely preserves the original sketch's UI pins and behavior.
- Distributed under the MIT License; see [LICENSE](LICENSE).
- Arduino IDE compilation and an initial ESP32 hardware demonstration test were reported successful on 2026-10-10. This is not independent verification of performance or calibration.

## License

MIT License. Copyright (c) 2026 vax9210. See [LICENSE](LICENSE).
