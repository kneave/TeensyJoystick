#include <Arduino.h>
#include <Encoder.h>

/* 
  Based on the Complete USB Joystick Example
  Teensy becomes a USB joystick with 16 or 32 buttons and 6 axis input

  The USB mode of the Teensy is set using a build flag in the platform.ini file.

*/

// Configure the number of buttons.  Be careful not
// to use a pin for both a digital button and analog
// axis.  The pullup resistor will interfere with
// the analog voltage.

//  Buttons
const int numButtons = 6;
const int buttonStart = 2;
const int joy1button = 23;
const int joy2button = 16;

//  Button states
byte allButtons[numButtons];
byte prevButtons[numButtons];
byte encoderPrev;

//  Encoder pins
Encoder enc(12, 11);
const int encButton = 10;

//  This is the hat angle, it currently maps to the encoder
//  Joystick hats have limited values as they're based off old dpads I believe:
//  0,45,90,135,180,225,270,315,-1
//  -1 is centre, I'm using it as a dial for now so starting at 0
volatile long angle = 0;

//  LED pins
const int redLed = 13;
const int greenLed = 15;
const int blueLed = 14;

//  RGB State
//  states [red, green, blue] map to [0, 1, 2]
//  The timeout and lastChange are used to debounce between loops
int state = 0;
unsigned long lastChange = 0;
unsigned long timeout = 250;

enum colour
{
  RED = 13,
  GREEN = 15,
  BLUE = 14
};

//  Sets the brightness of an LED on a given pin
void SetLED(colour led, float value)
{
  //  I've implemented this method as the LEDs are common anode, this means that rather than providing
  //  the voltage for the LED we're adjusting the value of the drain. This varies the potential between
  //  the the anode and cathode in the same way but is "backwards" from what most of us are used to.

  int newValue = map(value, 0, 1, 255, 0);
  analogWrite(led, newValue);
}

//  This currently just sets between R, G, and B colours
//  Brighness is taken from the hat angle
void UpdateLED(bool change)
{
  //  "/ 100" creates a float
  float brightness = map(angle, 0, 315, 0, 100) / 100.0;
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

void setup()
{
  // you can print to the serial monitor while the joystick is active!
  Serial.begin(9600);

  // configure the joystick to manual send mode.  This gives precise
  // control over when the computer receives updates, but it does
  // require you to manually call Joystick.send_now().
  Joystick.useManualSend(true);

  //  Set each of the buttons to INPUT_PULLUP
  for (int i = buttonStart; i < buttonStart + numButtons; i++)
  {
    pinMode(i, INPUT_PULLUP);
  }

  pinMode(joy1button, INPUT_PULLUP);
  pinMode(joy2button, INPUT_PULLUP);

  //  The RGB encoder is common anode so needs to be INPUT
  pinMode(encButton, INPUT);

  //  Set the LED pins to output
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);

  lastChange = millis();

  Serial.println("Joystick initialisation complete.");
}

void loop()
{
  // read 6 analog inputs and use them for the joystick axis
  Joystick.X(map(analogRead(20), 0, 1023, 1023, 0));
  Joystick.Y(analogRead(21));
  Joystick.Z(analogRead(22));
  Joystick.Zrotate(map(analogRead(19), 0, 1023, 1023, 0));
  Joystick.sliderLeft(analogRead(17));
  Joystick.sliderRight(map(analogRead(18), 0, 1023, 1023, 0));

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

  //  Set the joystick buttons, some are inverted to make up for being installed upside down.
  //  You can remove or add the ! as needed for your setup
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

  // Read the value from the rotary encoder
  long newAngle = enc.read();

  //  Lock it to a value between 0..315
  if (newAngle > 315)
  {
    enc.write(315);
    angle = 315;
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

  Joystick.hat(angle);

  // Because setup configured the Joystick manual send,
  // the computer does not see any of the changes yet.
  // This send_now() transmits everything all at once.
  Joystick.send_now();

  // check to see if any button changed since last time
  boolean anyChange = false;
  for (int i = 0; i < numButtons; i++)
  {
    if (allButtons[i] != prevButtons[i])
    {
      anyChange = true;
    }
    prevButtons[i] = allButtons[i];
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