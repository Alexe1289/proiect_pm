#include <SoftwareSerial.h>

//Create software serial object to communicate with SIM800L
SoftwareSerial mySerial(3, 2);  //SIM800L Tx & Rx is connected to Arduino #3 & #2
int hangup_pin = 4;
bool lastHangupState = LOW; // este LOW cand se ridica receptorul

int enable_dial_pin = 5;
bool lastEnableDialState = HIGH; //cand nu se misca rotia este pe HIGH

int dial_pin = 6;
bool lastDialState = LOW; //cand se formeaza numarul este pe LOW la inceput

int pin_changes = 0;
int numbers_typed = 0;
int numbers[100];

bool inCall = false;


void setup() {

  Serial.begin(9600);
  mySerial.begin(9600);

  pinMode(hangup_pin, INPUT_PULLUP);
  pinMode(enable_dial_pin, INPUT_PULLUP);
  pinMode(dial_pin, INPUT_PULLUP);


  delay(1000);

  mySerial.println("AT");
  updateSerial();
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
    Serial.print("Rotita a ajuns la final ");
    Serial.println(pin_changes);
    lastEnableDialState = HIGH;
    // daca am avut 0 pin_changes, cifra invalida
    if (pin_changes != 0) {
      if (pin_changes == 10) { // 10 semnale = cifra 0
        pin_changes = 0;
      }
      numbers[numbers_typed++] = pin_changes;
    }
    pin_changes = 0;
  }
}

void call_number() {
  if (numbers[0] == 0 && numbers[1] == 7) {
    Serial.println("Calling phone number: ");
    String phoneNumber = "ATD+4";
    for (int i = 0; i < numbers_typed; i++) {
      phoneNumber += String(numbers[i]); 
      Serial.print(numbers[i]);
    }
    phoneNumber += ";";
    mySerial.println(phoneNumber);
    inCall = true;
    delay(1000);
  } else {
    Serial.println("Invalid NUMBER!");
  }
  numbers_typed = 0;
}

void loop() {
  updateSerial();
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
}

void updateSerial() {
  while (Serial.available()) {
    mySerial.write(Serial.read());  //Forward what Serial received to Software Serial Port
  }
  while (mySerial.available()) {
    Serial.write(mySerial.read());  //Forward what Software Serial received to Serial Port
  }
}