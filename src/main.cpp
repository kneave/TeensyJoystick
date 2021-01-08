#include <Arduino.h>
#include <Encoder.h>

/* Complete USB Joystick Example
   Teensy becomes a USB joystick with 16 or 32 buttons and 6 axis input

   You must select Joystick from the "Tools > USB Type" menu

   Pushbuttons should be connected between the digital pins and ground.
   Potentiometers should be connected to analog inputs 0 to 5.

   This example code is in the public domain.
*/

// Configure the number of buttons.  Be careful not
// to use a pin for both a digital button and analog
// axis.  The pullup resistor will interfere with
// the analog voltage.

const int numButtons = 6;  // 16 for Teensy, 32 for Teensy++
const int buttonStart = 2;  // 16 for Teensy, 32 for Teensy++
const int joy1button = 23;
const int joy2button = 16;
const int encButton = 10;

Encoder enc(12, 11);

const int redLed = 13;
const int greenLed = 15;
const int blueLed = 14;

int state = -1;
unsigned long lastChange = 0;
unsigned long changeInterval = 1000;

void setup() {
  // you can print to the serial monitor while the joystick is active!
  Serial.begin(9600);
  // configure the joystick to manual send mode.  This gives precise
  // control over when the computer receives updates, but it does
  // require you to manually call Joystick.send_now().
  Joystick.useManualSend(true);
  for (int i=buttonStart; i< buttonStart + numButtons; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  pinMode(joy1button, INPUT_PULLUP);
  pinMode(joy2button, INPUT_PULLUP);
  pinMode(encButton, INPUT);

  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);

  analogWrite(redLed, 255);
  analogWrite(blueLed, 255);
  analogWrite(greenLed, 255);

  Serial.println("Begin Complete Joystick Test");
}

byte allButtons[numButtons];
byte prevButtons[numButtons];

volatile long angle = 0;

void loop() {
  // read 6 analog inputs and use them for the joystick axis
  Joystick.X(map(analogRead(20), 0, 1023, 1023, 0));
  Joystick.Y(analogRead(21));
  Joystick.Z(analogRead(22));
  Joystick.Zrotate(map(analogRead(19), 0, 1023, 1023, 0));
  Joystick.sliderLeft(analogRead(17));
  Joystick.sliderRight(map(analogRead(18), 0, 1023, 1023, 0));
  
  // read digital pins and use them for the buttons
  for (int i=0; i < numButtons; i++) {
    if (digitalRead(i+buttonStart)) {
      // when a pin reads high, the button is not pressed
      // the pullup resistor creates the "on" signal
      allButtons[i] = 0;
    } else {
      // when a pin reads low, the button is connecting to ground.
      allButtons[i] = 1;
    }
  }

  int joy1State = digitalRead(joy1button);
  int joy2State = digitalRead(joy2button);
  int encButtonState = digitalRead(encButton);

  //  Set the joystick buttons
  Joystick.button(1, !allButtons[0]);
  Joystick.button(2, !allButtons[1]);
  Joystick.button(3, allButtons[3]);
  Joystick.button(4, allButtons[2]);
  Joystick.button(5, !allButtons[4]);
  Joystick.button(6, !allButtons[5]);

  Joystick.button(7, joy1State == LOW);
  Joystick.button(8, joy2State == LOW);
  Joystick.button(9, encButtonState == HIGH);

  // make the hat switch automatically move in a circle
  long newAngle = enc.read();
  if(newAngle > 315) 
  { 
    enc.write(315);
    angle = 315;
  }
  else if(newAngle < 0)
  {
    enc.write(0);
    angle = 0;
  }
  else
  {
    angle = newAngle;
  }  
  
  Joystick.hat(angle);
  
  // Because setup configured the Joystick manual send,
  // the computer does not see any of the changes yet.
  // This send_now() transmits everything all at once.
  Joystick.send_now();
  
  // check to see if any button changed since last time
  boolean anyChange = false;
  for (int i=0; i<numButtons; i++) {
    if (allButtons[i] != prevButtons[i]) anyChange = true;
    prevButtons[i] = allButtons[i];
  }
  
  // if any button changed, print them to the serial monitor
  if (anyChange) {
    Serial.print("Buttons: ");
    for (int i=0; i<numButtons; i++) {
      Serial.print(allButtons[i], DEC);
    }
    Serial.println();
  }



  // a brief delay, so this runs "only" 200 times per second
  delay(5);
}

void UpdateLED()
{
    switch(state)
  {
    case 0:
      analogWrite(redLed, 127);
      analogWrite(blueLed, 255);
      analogWrite(greenLed, 255);
      break;
    case 1:
      analogWrite(redLed, 255);
      analogWrite(blueLed, 127);
      analogWrite(greenLed, 255);
      break;
    case 2:
      analogWrite(redLed, 255);
      analogWrite(blueLed, 255);
      analogWrite(greenLed, 127);
      break;
  }
  
  unsigned long timeDiff = millis() - lastChange;
  if(timeDiff >= changeInterval)
  {
    state++;
    if(state > 2)
    {
      state = 0;
    }
    lastChange = millis();
  }
}