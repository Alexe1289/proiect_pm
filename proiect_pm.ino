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

int ring_pin = 7;
bool lastRingState = HIGH;

int pin_changes = 0;
int numbers_typed = 0;
char numbers[11] = {};

bool inCall = false;
bool noNetwork = true;
bool incCall = false;

// unsigned long lastBlinkTime = 0;
// bool showIcon = true;

void setup() {

  // Serial.begin(115200);
  // delay(500);
  mySerial.begin(9600);
  Wire.begin();
  delay(500);
  pinMode(hangup_pin, INPUT_PULLUP);
  pinMode(enable_dial_pin, INPUT_PULLUP);
  pinMode(dial_pin, INPUT_PULLUP);
  pinMode(ring_pin, INPUT_PULLUP);

  display.begin(I2C_ADDRESS, true);

  delay(500);
  // Serial.println("sadsa");
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  // drawDialingInterface();

  // Serial.println("adasdad");
  // mySerial.println("AT");
}

void count_pin_changes() {
    int state = digitalRead(dial_pin);
    if (state == HIGH && lastDialState == LOW) {
        pin_changes++;
    }
    lastDialState = state;
}


void analyze_input() {
  //ENABLE DIAL = HIGH DACA ROTITA A AJUNS LA FINAL / NU SE MISCA
  // LOW DACA ROTITA E IN MISCARE
  int enableDial = digitalRead(enable_dial_pin);
  // Serial.println(enableDial);
  if (enableDial == LOW) {
    count_pin_changes();
    lastEnableDialState = LOW;
    delay(1);
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
      }
  }
}

void call_number() {
  if (numbers[0] == '0' && numbers[1] == '7') {
    // Serial.println("Calling phone number: ");
    String phoneNumber = "ATD+4";
    for (int i = 0; i < numbers_typed; i++) {
      phoneNumber += numbers[i]; 
      // Serial.print(numbers[i]);
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

void loop() {
  int hangupState = digitalRead(hangup_pin);
  int ring_val = digitalRead(ring_pin);
  if (ring_val == LOW && lastRingState != ring_pin) {
    incomingCall();
    lastRingState = ring_val;
    incCall = true;
  } else if (ring_val == HIGH && lastRingState != ring_pin) {
    incCall = false;
  }
  if (hangupState == LOW && !noNetwork) {
    // daca receptorul nu a fost pus jos
      if (!incCall) {
        analyze_input();
        if (numbers_typed == 10) {
          call_number();
        }
      } else {
        mySerial.println("ATA");
        inCall = true;
      }
  } else {
    //se inchide telefonul cand receptorul e jos
    if (inCall) {
      mySerial.println("ATH");
      inCall = false;
    }
    numbers_typed = 0;
    if (!incCall)
      mainInterface();
  }
  lastHangupState = hangupState;
  // display.clearDisplay();
}

void drawDialingInterface() {
  display.clearDisplay();
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

void incomingCall() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.println("Call");
  display.setCursor(10, 45);
  display.println("Incoming!");
  display.display();
}
void mainInterface() {
  // Request signal quality
  mySerial.println("AT+CSQ");
  delay(300);
  String csqResponse = "";
  while (mySerial.available()) {
    char c = mySerial.read();
    csqResponse += c;
  }

  int signalStrength = -1;
  int csqIndex = csqResponse.indexOf("+CSQ: ");
  if (csqIndex != -1) {
    // Extract the number after "+CSQ: "
    int commaIndex = csqResponse.indexOf(',', csqIndex);
    if (commaIndex != -1) {
      String rssiStr = csqResponse.substring(csqIndex + 6, commaIndex);
      signalStrength = rssiStr.toInt();
    }
  }

  // Request network registration status
  mySerial.println("AT+CREG?");
  delay(300);
  String cregResponse = "";
  while (mySerial.available()) {
    char c = mySerial.read();
    cregResponse += c;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Phone Status:");

  // Select signal bitmap based on signalStrength value
  const unsigned char* signalBitmap = no_signal;

  if (signalStrength >= 15) {
    signalBitmap = strong;
  } else if (signalStrength >= 10) {
    signalBitmap = medium;
  } else if (signalStrength >= 5) {
    signalBitmap = low;
  } else {
    signalBitmap = no_signal;
  }

  // Draw signal icon top right (x=SCREEN_WIDTH-12, y=0)
  display.drawBitmap(SCREEN_WIDTH - 12, 0, signalBitmap, 12, 12, SH110X_WHITE);

  if (cregResponse.indexOf("+CREG: 0,1") != -1 || cregResponse.indexOf("+CREG: 0,5") != -1) {
    display.setTextSize(2);
    display.setCursor(30, 20);
    display.println("Ready");
    noNetwork = false;
  } else {
    display.setTextSize(2);
    display.setCursor(5, 30);
    display.println("No Network");
    noNetwork = true;
  }

  display.display();
}