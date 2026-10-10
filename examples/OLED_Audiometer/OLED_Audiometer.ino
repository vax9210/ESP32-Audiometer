#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Audiometer.h>

// Existing OLED + button interface using the reusable sound library.
// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// ---------- Controls ----------
#define BTN_FREQ_DOWN   18
#define BTN_FREQ_UP     19
#define BTN_LEVEL_DOWN  32
#define BTN_LEVEL_UP    33
#define BTN_PRESENT     23
#define BTN_WARBLE      13

// ---------- I2S ----------
#define I2S_BCLK 26
#define I2S_LRC  25
#define I2S_DOUT 27

ESP32Audiometer audiometer;

// ---------- Frequencies ----------
int frequencies[] = {
  125, 250, 500, 1000,
  2000, 4000, 8000,
  12000, 16000
};

const int freqCount =
  sizeof(frequencies) / sizeof(frequencies[0]);

int freqIndex = 3;  // 1000 Hz

// Relative digital amplitude (not calibrated dB HL)
int dbLevels[] = {
  -24,
  -18,
  -12,
  -6,
  0
};

const int levelCount =
  sizeof(dbLevels) / sizeof(dbLevels[0]);

int levelIndex = 2;

const float maxAmplitude = 12000.0;

bool warbleMode = false;

// ---------- Debounce ----------
bool lastFreqDown = HIGH;
bool lastFreqUp = HIGH;
bool lastLevelDown = HIGH;
bool lastLevelUp = HIGH;
bool lastWarble = HIGH;

unsigned long lastFreqDownTime = 0;
unsigned long lastFreqUpTime = 0;
unsigned long lastLevelDownTime = 0;
unsigned long lastLevelUpTime = 0;
unsigned long lastWarbleTime = 0;

const unsigned long debounceTime = 150;


// ---------- OLED ----------
void showStatus(bool toneOn) {

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Frequency
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(frequencies[freqIndex]);
  display.println(" Hz");

  // Level
  display.setTextSize(1);
  display.setCursor(0, 27);
  display.print(dbLevels[levelIndex]);
  display.print(" dB");

  // Pure / Warble
  display.setCursor(65, 27);

  if (warbleMode) {
    display.print("WARBLE");
  } else {
    display.print("PURE");
  }

  // Present
  display.setCursor(0, 50);

  if (toneOn) {
    display.print("TONE ON");
  } else {
    display.print("TONE OFF");
  }

  display.display();
}


// ---------- Controls ----------
void checkControls() {

  unsigned long now = millis();

  bool freqDownState =
    digitalRead(BTN_FREQ_DOWN);

  bool freqUpState =
    digitalRead(BTN_FREQ_UP);

  bool levelDownState =
    digitalRead(BTN_LEVEL_DOWN);

  bool levelUpState =
    digitalRead(BTN_LEVEL_UP);

  bool warbleState =
    digitalRead(BTN_WARBLE);

  bool toneOn =
    digitalRead(BTN_PRESENT) == LOW;


  // Frequency DOWN
  if (
    freqDownState == LOW &&
    lastFreqDown == HIGH &&
    now - lastFreqDownTime > debounceTime
  ) {

    if (freqIndex > 0) {
      freqIndex--;
      showStatus(toneOn);
    }

    lastFreqDownTime = now;
  }


  // Frequency UP
  if (
    freqUpState == LOW &&
    lastFreqUp == HIGH &&
    now - lastFreqUpTime > debounceTime
  ) {

    if (freqIndex < freqCount - 1) {
      freqIndex++;
      showStatus(toneOn);
    }

    lastFreqUpTime = now;
  }


  // Level DOWN
  if (
    levelDownState == LOW &&
    lastLevelDown == HIGH &&
    now - lastLevelDownTime > debounceTime
  ) {

    if (levelIndex > 0) {
      levelIndex--;
      showStatus(toneOn);
    }

    lastLevelDownTime = now;
  }


  // Level UP
  if (
    levelUpState == LOW &&
    lastLevelUp == HIGH &&
    now - lastLevelUpTime > debounceTime
  ) {

    if (levelIndex < levelCount - 1) {
      levelIndex++;
      showStatus(toneOn);
    }

    lastLevelUpTime = now;
  }


  // PURE / WARBLE
  if (
    warbleState == LOW &&
    lastWarble == HIGH &&
    now - lastWarbleTime > debounceTime
  ) {

    warbleMode = !warbleMode;

    showStatus(toneOn);

    lastWarbleTime = now;
  }


  lastFreqDown = freqDownState;
  lastFreqUp = freqUpState;
  lastLevelDown = levelDownState;
  lastLevelUp = levelUpState;
  lastWarble = warbleState;
}


void setup() {
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true) delay(1000);
  }
  pinMode(BTN_FREQ_DOWN, INPUT_PULLUP);
  pinMode(BTN_FREQ_UP, INPUT_PULLUP);
  pinMode(BTN_LEVEL_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEVEL_UP, INPUT_PULLUP);
  pinMode(BTN_PRESENT, INPUT_PULLUP);
  pinMode(BTN_WARBLE, INPUT_PULLUP);
  if (!audiometer.begin(I2S_BCLK, I2S_LRC, I2S_DOUT)) {
    while (true) delay(1000);
  }
  showStatus(false);
}

void loop() {
  checkControls();
  const bool toneOn = digitalRead(BTN_PRESENT) == LOW;
  audiometer.setFrequency(frequencies[freqIndex]);
  audiometer.setLevelDb(dbLevels[levelIndex]);
  audiometer.setWarble(warbleMode);
  audiometer.setPresent(toneOn);
  audiometer.process();
  static bool lastToneOn = false;
  if (toneOn != lastToneOn) {
    showStatus(toneOn);
    lastToneOn = toneOn;
  }
}
