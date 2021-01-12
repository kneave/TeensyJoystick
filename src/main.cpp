#include <Arduino.h>
#include <Encoder.h>

/* 
  Based on the Complete USB Joystick Example
  Teensy becomes a USB joystick with 16 or 32 buttons and 6 axis input, this one is in "extreme mode".

  BIG NOTE!
  This wont work unless you change JOYSTICK_SIZE to 64 in ".platformio\packages\framework_arduinoteensy\cores\teensy4\usb_desc.h"

  The USB mode of the Teensy is set using a build flag in the platform.ini file.
*/

// Configure the number of buttons.  Be careful not
// to use a pin for both a digital button and analog
// axis.  The pullup resistor will interfere with
// the analog voltage.

const int numButtons = 6;  // 16 for Teensy, 32 for Teensy++
const int buttonStart = 2; // 16 for Teensy, 32 for Teensy++
const int joy1button = 23;
const int joy2button = 16;
const int encButton = 10;

byte allButtons[numButtons];
byte prevButtons[numButtons];
byte encoderPrev;

volatile long angle = 0;

const int deadzone = 3000;
const int midPoint = 32767;
const int lowPoint =  midPoint - deadzone;
const int highPoint = midPoint + deadzone;

Encoder enc(12, 11);

//  LED pins
enum colour
{
  RED = 13,
  GREEN = 15,
  BLUE = 14
};

//  RGB State
//  states [red, green, blue] map to [0, 1, 2]
//  The timeout and lastChange are used to debounce between loops
int state = 0;
unsigned long lastChange = 0;
unsigned long timeout = 250;

//  Sets the brightness of an LED on a given pin
void SetLED(colour led, float value)
{
  //  I've implemented this method as the LEDs are common anode, this means that rather than providing
  //  the voltage for the LED we're adjusting the value of the drain. This varies the potential between
  //  the the anode and cathode in the same way but is "backwards" from what most of us are used to.

  int newValue = map(value, 0, 1, 255, 0);
  analogWrite(led, newValue);
}

void setup()
{
  // you can print to the serial monitor while the joystick is active!
  Serial.begin(9600);
  // configure the joystick to manual send mode.  This gives precise
  // control over when the computer receives updates, but it does
  // require you to manually call Joystick.send_now().
  Joystick.useManualSend(true);
  for (int i = buttonStart; i < buttonStart + numButtons; i++)
  {
    pinMode(i, INPUT_PULLUP);
  }
  pinMode(joy1button, INPUT_PULLUP);
  pinMode(joy2button, INPUT_PULLUP);
  pinMode(encButton, INPUT);

  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  enc.write(512);

  Serial.println("Joystick Setup Complete.");
}

int CheckAxisValue(int value, bool invert)
{
  if (invert)
  {
    value = map(value, 0, 1023, 65535, 0);
  }
  else
  {
    value = map(value, 0, 1023, 0, 65535);
  }
  

  if ((value > lowPoint) & (value < highPoint))
  {
    value = midPoint;
  }

  return value;
}

//  This currently just sets between R, G, and B colours
//  Brighness is taken from the hat angle
void UpdateLED(bool change)
{
  //  "/ 100" creates a float
  float brightness = map(angle, 0, 1024, 0, 100) / 100.0;
  switch (state)
  {
  case 0:
    SetLED(RED, brightness);
    SetLED(GREEN, 0);
    SetLED(BLUE, 0);
    break;
  case 1:
    SetLED(RED, 0);
    SetLED(GREEN, brightness);
    SetLED(BLUE, 0);
    break;
  case 2:
    SetLED(RED, 0);
    SetLED(GREEN, 0);
    SetLED(BLUE, brightness);
    break;
  }

  //  Update only if button pressedn and not in timeout
  if (change && (millis() - lastChange > timeout))
  {
    state++;
    if (state > 2)
    {
      state = 0;
    }
    lastChange = millis();
    Serial.println("Brightness: " + String(brightness));
  }
}

void loop()
{
  // read 6 analog inputs and use them for the joystick axis
  Joystick.X(CheckAxisValue(analogRead(20), true));
  Joystick.Y(CheckAxisValue(analogRead(21), true));
  Joystick.Z(CheckAxisValue(analogRead(22), false));
  Joystick.Xrotate(CheckAxisValue(analogRead(19), true));
  Joystick.Yrotate(CheckAxisValue(analogRead(18), true));
  Joystick.Zrotate(CheckAxisValue(analogRead(17), false));
  
  // read digital pins and use them for the buttons
  for (int i = 0; i < numButtons; i++)
  {
    if (digitalRead(i + buttonStart))
    {
      // when a pin reads high, the button is not pressed
      // the pullup resistor creates the "on" signal
      allButtons[i] = 0;
    }
    else
    {
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
  
  //  Common anode on this so inverted to the "norm"
  bool encPressed = encButtonState == HIGH;
  Joystick.button(9, encPressed);

  // make the hat switch automatically move in a circle
  long newAngle = enc.read();
  
  if (newAngle > 1024)
  {
    enc.write(1024);
    angle = 1024;
  }
  else if (newAngle < 0)
  {
    enc.write(0);
    angle = 0;
  }
  else
  {
    angle = newAngle;
  }

  Joystick.slider(1, angle * 64);

  // Because setup configured the Joystick manual send,
  // the computer does not see any of the changes yet.
  // This send_now() transmits everything all at once.
  Joystick.send_now();

  // check to see if any button changed since last time
  boolean anyChange = false;
  for (int i = 0; i < numButtons; i++)
  {
    if (allButtons[i] != prevButtons[i])
      anyChange = true;
    prevButtons[i] = allButtons[i];
  }

  // if any button changed, print them to the serial monitor
  if (anyChange)
  {
    Serial.print("Buttons: ");
    for (int i = 0; i < numButtons; i++)
    {
      Serial.print(allButtons[i], DEC);
    }
    Serial.println();
  }
  
  bool encoderChanged = false;
  if (encPressed != encoderPrev)
  {
    encoderChanged = true;
  }

  UpdateLED(encoderChanged);

  // if any button changed, print them to the serial monitor
  if (anyChange | encoderChanged)
  {
    Serial.print("Buttons: ");
    for (int i = 0; i < numButtons; i++)
    {
      Serial.print(allButtons[i], DEC);
    }
    Serial.println();
  }

  // a brief delay, so this runs "only" 200 times per second
  delay(5);
}