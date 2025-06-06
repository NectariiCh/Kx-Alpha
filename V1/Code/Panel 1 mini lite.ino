#include "KerbalSimpit.h"

bool CAG_MODE_TOGGLE = false;
bool TOGGLED1 = false;
bool TOGGLED2 = false;
bool TOGGLED3 = false;

#define SWT_EN_PIN 12

#define TRANSLATION_X A0
#define TRANSLATION_Y A1
#define TRANSLATION_Z A2

#define SASAP 2
#define RCSAP 3
#define GERAP 4
#define BRKAP 5
#define LITAP 6
#define AG1AP 7
#define AG2AP 8
#define STGAP 9
#define ABRAP 10

#define TRGAP 11

#define SASMG SAS_ACTION
#define RCSMG RCS_ACTION
#define GERMG GEAR_ACTION
#define BRKMG BRAKES_ACTION
#define LITMG LIGHT_ACTION
#define AG1MG 1
#define AG2MG 2
#define STGMG STAGE_ACTION
#define ABRMG ABORT_ACTION

#define TRGMG 7

#define THC_X_DEADZONE_MIN (511 - 20)
#define THC_X_DEADZONE_MAX (511 + 20)
#define THC_Y_DEADZONE_MIN (511 - 20)
#define THC_Y_DEADZONE_MAX (511 + 20)
#define THC_Z_DEADZONE_MIN (511 - 20)
#define THC_Z_DEADZONE_MAX (511 + 20)

KerbalSimpit mySimpit(Serial);

bool checkBtnWDebounce(int pin, int time) {
  if (!digitalRead(pin)) {
    delay(time);
    if (!digitalRead(pin)) return true; else return false;
  } else return false;
}

void checkAGs() {
  // TRG
  if (!digitalRead(TRGAP)) {
    mySimpit.activateCAG(7);
  } else {
    mySimpit.deactivateCAG(7);
  }

  // SAS
  if (!digitalRead(SASAP)) {
    mySimpit.activateAction(SASMG);
  } else {
    mySimpit.deactivateAction(SASMG);
  }

  // RCS
  if (!digitalRead(RCSAP)) {
    mySimpit.activateAction(RCSMG);
  } else {
    mySimpit.deactivateAction(RCSMG);
  }

  // STG
  if (checkBtnWDebounce(STGAP, 10)) {
     if (!TOGGLED3) {
      mySimpit.activateAction(STGMG);
     }
    } else {
      TOGGLED3 = false;
      mySimpit.deactivateAction(STGMG);
    }

  // AG1
  if (CAG_MODE_TOGGLE) {
    if (checkBtnWDebounce(AG1AP, 10)) {
     if (!TOGGLED1) {
      mySimpit.toggleCAG(AG1MG);
      TOGGLED1 = true;
     }
    } else {
      TOGGLED1 = false;
    }
  } else {
    if (!digitalRead(AG1AP)) {
      mySimpit.activateCAG(AG1MG);
    } else {
     mySimpit.deactivateCAG(AG1MG);
    }
  }

  // AG2
  if (CAG_MODE_TOGGLE) {
    if (checkBtnWDebounce(AG2AP, 10)) {
     if (!TOGGLED2) {
      mySimpit.toggleCAG(AG2MG);
      TOGGLED2 = true;
     }
    } else {
      TOGGLED2 = false;
    }
  } else {
    if (!digitalRead(AG2AP)) {
      mySimpit.activateCAG(AG2MG);
    } else {
     mySimpit.deactivateCAG(AG2MG);
    }
  }

  // GER
  if (!digitalRead(GERAP)) {
    mySimpit.activateAction(GERMG);
  } else {
    mySimpit.deactivateAction(GERMG);
  }

  // BRK
  if (!digitalRead(BRKAP)) {
    mySimpit.activateAction(BRKMG);
  } else {
    mySimpit.deactivateAction(BRKMG);
  }

  // LIT
  if (!digitalRead(LITAP)) {
    mySimpit.activateAction(LITMG);
  } else {
    mySimpit.deactivateAction(LITMG);
  }

  // ABR
  if (!digitalRead(ABRAP)) {
    mySimpit.activateAction(ABRMG);
  } else {
    mySimpit.deactivateAction(ABRMG);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(SWT_EN_PIN, INPUT_PULLUP);
  pinMode(SASAP, INPUT_PULLUP);
  pinMode(RCSAP, INPUT_PULLUP);
  pinMode(STGAP, INPUT_PULLUP);
  pinMode(AG1AP, INPUT_PULLUP);
  pinMode(AG2AP, INPUT_PULLUP);
  pinMode(GERAP, INPUT_PULLUP);
  pinMode(BRKAP, INPUT_PULLUP);
  pinMode(LITAP, INPUT_PULLUP);
  pinMode(ABRAP, INPUT_PULLUP);
  pinMode(TRGAP, INPUT_PULLUP);
  pinMode(TRANSLATION_X, INPUT);
  pinMode(TRANSLATION_Y, INPUT);
  pinMode(TRANSLATION_Z, INPUT);

  if (!digitalRead(SWT_EN_PIN)) {
    CAG_MODE_TOGGLE = true;
  }

  while (!mySimpit.init()) {
    delay(40);
  }

  mySimpit.printToKSP("Connected asdf", PRINT_TO_SCREEN);
  mySimpit.inboundHandler(messageHandler);
  mySimpit.registerChannel(FLIGHT_STATUS_MESSAGE);
  mySimpit.registerChannel(THROTTLE_CMD_MESSAGE);
}
void loop() {
  checkAGs();

  int analogInput = analogRead(TRANSLATION_X);
  int translationX = 0;
  if (analogInput < THC_X_DEADZONE_MIN) translationX = map(analogInput, 0, THC_X_DEADZONE_MIN, INT16_MIN, 0);
  else if (analogInput > THC_X_DEADZONE_MAX) translationX = map(analogInput, THC_X_DEADZONE_MAX, 1023, 0, INT16_MAX);

  analogInput = -analogRead(TRANSLATION_Y) + 1023;
  int translationY = 0;
  if (analogInput < THC_Y_DEADZONE_MIN) translationY = map(analogInput, 0, THC_Y_DEADZONE_MIN, INT16_MIN, 0);
  else if (analogInput > THC_Y_DEADZONE_MAX) translationY = map(analogInput, THC_Y_DEADZONE_MAX, 1023, 0, INT16_MAX);

  analogInput = analogRead(TRANSLATION_Z);
  int translationZ = 0;
  translationZ = map(analogInput, 1023, 710, 0, INT16_MAX);
  int reading = analogRead(A2);

  rotationMessage translation_msg;
  translation_msg.setYaw(-translationX);
  translation_msg.setPitch(-translationY);
  mySimpit.send(ROTATION_MESSAGE, translation_msg);
  throttleMessage throttle_msg;
  throttle_msg.throttle = map(reading, 1023, 630, 0, INT16_MAX); 
  mySimpit.send(19, throttle_msg);

  mySimpit.update();
}

void messageHandler(byte messageType, byte msg[], byte msgSize) {
  switch (messageType) {
    case FLIGHT_STATUS_MESSAGE:
      {
        flightStatusMessage fsm = parseMessage<flightStatusMessage>(msg);
      }
      break;
  }
}
