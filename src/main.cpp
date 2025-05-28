#include <util/delay.h>
#include <usart.c>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define I2C_ADDRESS 0x3C

#define HANGUP_PIN PD4
#define ENABLE_DIAL_PIN PD5
#define DIAL_PIN PD6
#define RING_PIN PD7
#define BUZZER_PIN PB0

#define RX_PIN PD2
#define TX_PIN PD3

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

SoftwareSerial mySerial =  SoftwareSerial(TX_PIN, RX_PIN);
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int signal;
bool lastHangupState = false; // este LOW cand se ridica receptorul
bool lastEnableDialState = true; //cand nu se misca rotia este pe HIGH
bool lastDialState = false; //cand se formeaza numarul este pe LOW la inceput
bool lastRingState = true;

int pin_changes = 0;
int numbers_typed = 0;
char numbers[11] = {};

bool inCall = false;
bool noNetwork = true;
bool incCall = false;

void drawDialingInterface() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 15);
  display.println("Dialing:");

  display.setTextSize(2);
  int spacing = 11;
  int totalWidth = spacing * numbers_typed;
  int xStart = (SCREEN_WIDTH - totalWidth) / 2;

  for (int i = 0; i < numbers_typed; i++) {
    display.setCursor(xStart + i * spacing, 30);
    display.print(numbers[i]);
  }

  if (numbers_typed == 10) {
    display.setTextSize(1);
    display.setCursor(20, 50);
    display.print("Calling...");

  }
  display.drawBitmap(90, 48, calling_symbol, 16, 15, SH110X_WHITE);
  display.display();
}

void count_pin_changes() {
  int state = (PIND & (1 << DIAL_PIN)) ? true : false;
  if (state == true && lastDialState == false) {
    pin_changes++;
  }
  lastDialState = state;
}

void analyze_input() {
  int enableDial = (PIND & (1 << ENABLE_DIAL_PIN)) ? true : false;
  if (enableDial == false) {
    count_pin_changes();
    lastEnableDialState = false;
    _delay_ms(1);
  } else if (lastEnableDialState == false && enableDial == true) {
    //rotita a ajuns la final
    lastEnableDialState = true;
    // daca am avut 0 pin_changes, cifra invalida
    if (numbers_typed < 11) {
      if (pin_changes != 0) {
        if (pin_changes == 10) { // 10 semnale = cifra 0
          pin_changes = 0;
        }
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
    String phoneNumber = "ATD+4";
    for (int i = 0; i < numbers_typed; i++) {
      phoneNumber += numbers[i];
    }
    phoneNumber += ";";
    mySerial.println(phoneNumber);
    inCall = true;
    _delay_ms(1000);
  }
  numbers_typed = 0;
}



void incomingCall() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.println("Call");
  display.setCursor(10, 45);
  display.println("Incoming!");
  display.display();
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 1000);
    _delay_ms(100);
    noTone(BUZZER_PIN);
    _delay_ms(100);
  }
}

void mainInterface() {
  // Request signal quality
  mySerial.println("AT+CSQ");
  _delay_ms(300);
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
  _delay_ms(300);
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

int main()
{
    mySerial.begin(9600);
    Wire.begin();
    _delay_ms(500);
    DDRD &= ~((1 << HANGUP_PIN) | (1 << ENABLE_DIAL_PIN) | (1 << DIAL_PIN) | (1 << RING_PIN));
    PORTD |= (1 << HANGUP_PIN) | (1 << ENABLE_DIAL_PIN) | (1 << DIAL_PIN) | (1 << RING_PIN);

    DDRB |= (1 << BUZZER_PIN);
    PORTB &= ~(1 << BUZZER_PIN);
    display.begin(I2C_ADDRESS, true);
    delay(500);
    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    mySerial.println("AT");

    while(true) {
      int hangupState = (PIND & (1 << HANGUP_PIN)) ? true : false;
      int ring_val = (PIND & (1 << RING_PIN)) ? true : false;

      if (ring_val == false && lastRingState != ring_val) {
        incomingCall();
        lastRingState = ring_val;
        incCall = true;
      } else if (ring_val == HIGH && lastRingState != ring_val) {
        incCall = false;
      }
      if (hangupState == false && !noNetwork) {
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
    }
    return 0;
}