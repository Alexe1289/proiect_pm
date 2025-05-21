#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define I2C_ADDRESS 0x3C

// 'Calling', 16x15px
const unsigned char calling_symbol [] PROGMEM = {
	0x01, 0x80, 0x38, 0x30, 0x7c, 0x08, 0x7c, 0x64, 0xfc, 0x12, 0xf8, 0x0a, 0x78, 0x08, 0x7c, 0x01, 
	0x3e, 0x01, 0x1f, 0x00, 0x0f, 0x9c, 0x07, 0xfe, 0x03, 0xfe, 0x01, 0xfe, 0x00, 0xfc, 0x00, 0x30
};

const unsigned char strong [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x20, 0x40, 0x80, 0x10, 0x9f, 0x90, 0x7f, 0xe0, 0x3f, 0xc0, 0x1f, 0x80, 
	0x0f, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char medium [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x20, 0x40, 0x80, 0x10, 0x80, 0x10, 0x40, 0x20, 0x3f, 0xc0, 0x1f, 0x80, 
	0x0f, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00
};


const unsigned char low [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x20, 0x40, 0x80, 0x10, 0x80, 0x10, 0x40, 0x20, 0x20, 0x40, 0x10, 0x80, 
	0x0f, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char no_signal [] PROGMEM = {
	0x00, 0x00, 0x40, 0x00, 0x27, 0x00, 0x33, 0xc0, 0x68, 0x60, 0x0c, 0x00, 0x1e, 0x00, 0x01, 0x00, 
	0x06, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00
};



Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int dialedNumbers[10] = {};
int numCount = 0;
int signal = 0;

// Blinking control
unsigned long lastBlinkTime = 0;
bool showIcon = true;

void setup() {
  Wire.begin();
  display.begin(I2C_ADDRESS, true);
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  drawDialingInterface();
}

void loop() {
  // Simulate number input: add one digit every 1.5s

  drawClock(15, 32);
  if (numCount < 10) {
    delay(300);
    dialedNumbers[numCount] = random(0, 10);
    numCount++;
    drawDialingInterface();
  } else {
    // Blink symbol every 1s
    if (millis() - lastBlinkTime >= 1000) {
      showIcon = !showIcon;
      lastBlinkTime = millis();
      drawDialingInterface();
    }
  }
}

void drawClock(int hours, int minutes) {
  display.clearDisplay();
  int sigX = SCREEN_WIDTH - 52;
  int sigY = 0;
  const unsigned char* icon;

  switch (signal) {
    case 3: icon = strong; break;
    case 2: icon = medium; break;
    case 1: icon = low; break;
    default: icon = no_signal; break;
  }
  if (icon == no_signal) {
    if (millis() - lastBlinkTime >= 1000) {
      showIcon = !showIcon;
      lastBlinkTime = millis();
    }
    if (showIcon) {
      display.drawBitmap(sigX, sigY, icon, 12, 12, SH110X_WHITE);
    }
  } else {
    display.drawBitmap(sigX, sigY, icon, 12, 12, SH110X_WHITE);
  }

  // Draw clock (top-right)
  display.setTextSize(1);
  display.setCursor(SCREEN_WIDTH - 35, 2);
  if (hours < 10) display.print("0");
    display.print(hours);
  display.print(":");
  if (minutes < 10) display.print("0");
    display.print(minutes);
}

void drawDialingInterface() {

  // Title
  display.setTextSize(1);
  display.setCursor(0, 15);
  display.println("Dialing:");

  // Display the digits
  display.setTextSize(2);
  int spacing = 11;
  int totalWidth = spacing * numCount;
  int xStart = (SCREEN_WIDTH - totalWidth) / 2;

  for (int i = 0; i < numCount; i++) {
    display.setCursor(xStart + i * spacing, 30);
    display.print(dialedNumbers[i]);
  }

  // If all digits entered, show "Calling..." and icon
  if (numCount == 10) {
    display.setTextSize(1);
    display.setCursor(20, 50);
    display.print("Calling...");

    if (showIcon) {
      // Draw bitmap to the right of "Calling..." (adjust as needed)
      display.drawBitmap(90, 48, calling_symbol, 16, 15, SH110X_WHITE);
    }
  }

  display.display();
}
