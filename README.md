# NE-Joystick
A Teensy based joystick to use for robotics control, using two three axis joysticks with buttons, a bunch of switches, and an encoder.

## Hardware Details
### Microcontroller
The core of this board is a Teensy 4.0 in USB serial/HID mode. This is defined in the plaformio.ini file. This code will also only work if you put the Teensy in to "Extreme joystick mode", this gives access to the full six axes and up to 32 buttons and is enabled as follows.

Open the following file:
`"%HOME%\.platformio\packages\framework_arduinoteensy\cores\teensy4\usb_desc.h"`

Find the definition for `JOYSTICK_SIZE` and change it from 12 to 64. If you see freezing issues with the joystick, you may want to change `JOYSTICK_INTERVAL` to 2 as per [this thread.](https://forum.pjrc.com/threads/70501-Teensy-Joystick-Stops-Responding) 

### Joysticks
I'm using two [ServoCity 3-axis joysticks with buttons](https://www.robotshop.com/uk/3-axis-joystick-w-button.htm) for the actual joysticks, you need to wire each of the three potentiometers to pins on the Teensy. 
These are defined in `LEFT_` and `RIGHT_` values at the start of main.cpp and you can swap them out for your values. 

The buttons on these joysticks are attached to pins defined as `joy1button` and `joy2button`.

### Switches/Buttons
Any toggle switches or buttons will work here depending on your use case but for me I used a selection of chunky switches from an old analogue RC transmitter. The use contiguous pins on the Teensy starting at `firstSwitch` and continuing for `numSwitches`. 

In my case I have a toggle switch wired up as if it's two switches, I essentially get an up, down, and off values from this. The Teensy simply sends on or off for either switch and the client side can figure out what before that implements.

In `loop`, the pins are simply parsed and thier values used for the joystick values.

### Encoder
The [rotary encoder](https://coolcomponents.co.uk/products/rotary-encoder-illuminated-rgb?_pos=1&_sid=2837bdace&_ss=r) has two cool features, a push button and RGB LED. The encoder button is attached to the pin definied by `encButton`, the encoder is attached to pins 12 and 11 in the constructer to `enc`.

For the LEDs there is an enum called `colour` which defines red, green, and blue and specifies the pins they are attached to.

## Support
Massive thanks as ever to my [Patrons](www.patreon.com/neaveeng)!

Patrons!
- Angie and John Neave
- Bodger

Supporters!
- Mark Long
- Adam Gilmore
- David Shrive

Followers!
- Phoenix Labs
- Andy Batey
- Dave Booth
- Cara S.
- Shelagh Lewins
- Mandy Berry
- Alister Perrott
