#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define I2C_ADDRESS 0x3C

//16x15 size
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
int signal;
SoftwareSerial mySerial(3, 2);  //SIM800L Tx & Rx is connected to Arduino #3 & #2

int hangup_pin = 4;
bool lastHangupState = LOW; // este LOW cand se ridica receptorul

int enable_dial_pin = 5;
bool lastEnableDialState = HIGH; //cand nu se misca rotia este pe HIGH

int dial_pin = 6;
bool lastDialState = LOW; //cand se formeaza numarul este pe LOW la inceput

int pin_changes = 0;
int numbers_typed = 0;
char numbers[11] = {};
String simDate = "";
String simTime = "";

bool inCall = false;

<<<<<<< Updated upstream
unsigned int lastClockUpdate = 0;
const unsigned int clockInterval = 1800000UL;
// unsigned long lastBlinkTime = 0;
// bool showIcon = true;

void setup() {

  Serial.begin(115200);
  delay(500);
=======
// --- CLOCK variables ---
String simDate = "";
String simTime = "";
unsigned long lastClockUpdate = 0;
const unsigned long clockInterval = 1800000UL; // 30 minutes

void setup() {
  Serial.begin(9600);
>>>>>>> Stashed changes
  mySerial.begin(9600);
  Wire.begin();
  delay(500);
  pinMode(hangup_pin, INPUT_PULLUP);
  pinMode(enable_dial_pin, INPUT_PULLUP);
  pinMode(dial_pin, INPUT_PULLUP);

<<<<<<< Updated upstream
  display.begin(I2C_ADDRESS, true);

  delay(500);
  // Serial.println("sadsa");
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  drawDialingInterface();
=======
  delay(1000);
>>>>>>> Stashed changes

  // Serial.println("adasdad");
  mySerial.println("AT");
<<<<<<< Updated upstream
=======
  updateSerial();

  getClock();  // Initial clock fetch at startup
  drawClock(); // Show initial time
}

void loop() {
  updateSerial();

  // Update clock every 30 minutes
  if (millis() - lastClockUpdate >= clockInterval || lastClockUpdate == 0) {
    getClock();
    drawClock();
    lastClockUpdate = millis();
  }

  int hangupState = digitalRead(hangup_pin);
  if (hangupState == LOW) {
    analyze_input();
    if (numbers_typed == 10) {
      call_number();
    }
  } else {
    if (inCall) {
      mySerial.println("ATH");
      inCall = false;
    }
  }

  lastHangupState = hangupState;
>>>>>>> Stashed changes
}

void count_pin_changes() {
  int state = digitalRead(dial_pin);
  if (state == HIGH && lastDialState == LOW) {
    pin_changes++;
  }
  lastDialState = state;
}


void analyze_input() {
  int enableDial = digitalRead(enable_dial_pin);
  if (enableDial == LOW) {
    count_pin_changes();
    lastEnableDialState = LOW;
    delay(1);
<<<<<<< Updated upstream
  } else if (lastEnableDialState == LOW  && enableDial == HIGH) {
    //rotita a ajuns la final
    // Serial.print("Rotita a ajuns la final ");
    // Serial.println(pin_changes);
    lastEnableDialState = HIGH;
    // daca am avut 0 pin_changes, cifra invalida
    if (numbers_typed < 11) {
      if (pin_changes != 0) {
        if (pin_changes == 10) { // 10 semnale = cifra 0
          pin_changes = 0;
        }
        // Serial.println("Adasda");
        numbers[numbers_typed] = pin_changes + '0';
        numbers_typed++;
        drawDialingInterface();
      }
      pin_changes = 0;
=======
  } else if (lastEnableDialState == LOW && enableDial == HIGH) {
    Serial.print("Rotita a ajuns la final ");
    Serial.println(pin_changes);
    lastEnableDialState = HIGH;
    if (pin_changes != 0) {
      if (pin_changes == 10) {
        pin_changes = 0;
>>>>>>> Stashed changes
      }
  }
}

void call_number() {
  if (numbers[0] == '0' && numbers[1] == '7') {
    // Serial.println("Calling phone number: ");
    String phoneNumber = "ATD+4";
    for (int i = 0; i < numbers_typed; i++) {
<<<<<<< Updated upstream
      phoneNumber += numbers[i]; 
      // Serial.print(numbers[i]);
=======
      phoneNumber += String(numbers[i]);
      Serial.print(numbers[i]);
>>>>>>> Stashed changes
    }
    phoneNumber += ";";
    // Serial.println(phoneNumber);
    mySerial.println(phoneNumber);
    inCall = true;
    delay(1000);
  } else {
    // Serial.println("Invalid NUMBER!");
  }
  numbers_typed = 0;
}

<<<<<<< Updated upstream
void loop() {
  int hangupState = digitalRead(hangup_pin);
  if (hangupState == LOW) {
    // daca receptorul nu a fost pus jos
      analyze_input();
      if (numbers_typed == 10) {
        call_number();
      }
  } else {
    //se inchide telefonul cand receptorul e jos
    if (inCall) {
      mySerial.println("ATH");
    }
  }
  lastHangupState = hangupState;
  if (millis() - lastClockUpdate >= clockInterval || lastClockUpdate == 0) {
    getClock();  // Update time from SIM800L
    lastClockUpdate = millis();
  }
  // display.clearDisplay();
  drawClock(simTime);
}
void getClock() {
  mySerial.println("AT+CLK?");
  delay(100);
  String response = "";
  unsigned long timeout = millis() + 2000;

  while (millis() < timeout) {
    while (mySerial.available()) {
      char c = mySerial.read();
      response += c;
    }
  }
  display.clearDisplay();
  display.println(response);
  int indexStart = response.indexOf("\"");
  int indexEnd = response.indexOf("\"", indexStart + 1);

  if (indexStart != -1 && indexEnd != -1) {
    String dateTime = response.substring(indexStart + 1, indexEnd);
    int commaIndex = dateTime.indexOf(",");

    if (commaIndex != -1) {
      simDate = dateTime.substring(0, commaIndex);    // "24/05/22"
      simTime = dateTime.substring(commaIndex + 1);   // "13:10:45+08"
    }
  }
}
void drawClock(const String& timeStr) {
  // Extract time only (remove time zone if present)
  int plusIndex = timeStr.indexOf('+');
  String timeOnly = (plusIndex != -1) ? timeStr.substring(0, plusIndex) : timeStr;

  display.setTextSize(1);
  display.setCursor(SCREEN_WIDTH - 40, 0); // Top-right corner
  display.print(timeOnly);
  display.display();
}

void drawDialingInterface() {
  // display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 15);
  display.println("Dialing:");

  // Display the digits
  display.setTextSize(2);
  int spacing = 11;
  int totalWidth = spacing * numbers_typed;
  int xStart = (SCREEN_WIDTH - totalWidth) / 2;

  for (int i = 0; i < numbers_typed; i++) {
    display.setCursor(xStart + i * spacing, 30);
    display.print(numbers[i]);
  }

  // If all digits entered, show "Calling..." and icon
  if (numbers_typed == 10) {
    display.setTextSize(1);
    display.setCursor(20, 50);
    display.print("Calling...");

  }
  display.drawBitmap(90, 48, calling_symbol, 16, 15, SH110X_WHITE);
  display.display();
}

void mainInterface() {

}
=======
void updateSerial() {
  while (Serial.available()) {
    mySerial.write(Serial.read());
  }
  while (mySerial.available()) {
    Serial.write(mySerial.read());
  }
}

// --- CLOCK Functions ---

void getClock() {
  mySerial.println("AT+CCLK?");
  delay(200);
  String response = "";
  unsigned long timeout = millis() + 2000;

  while (millis() < timeout) {
    while (mySerial.available()) {
      char c = mySerial.read();
      response += c;
    }
  }

  int indexStart = response.indexOf("\"");
  int indexEnd = response.indexOf("\"", indexStart + 1);

  if (indexStart != -1 && indexEnd != -1) {
    String dateTime = response.substring(indexStart + 1, indexEnd);
    int commaIndex = dateTime.indexOf(",");
    if (commaIndex != -1) {
      simDate = dateTime.substring(0, commaIndex);
      simTime = dateTime.substring(commaIndex + 1);
    }
  }
}

void drawClock() {
  Serial.print("SIM Clock: ");
  Serial.print(simDate);
  Serial.print(" ");
  Serial.println(simTime);
}
>>>>>>> Stashed changes
