# Hovercraft
Remote-controlled hovercraft with two engines. Powered by a large battery.


## Description
- microcontroller STM32f4
- Bluetooth module HC-05 for communication with remote controller (or PC)
- distance sensor HC-SR04
- body hovercraft
- servomechanism TowerPro MG-91
- two engines with propellers
- battery 9.6V
- controller L298 with radiator

Additionally: PC application written in Python 2.7.

#### PINs scheme
- PB8 - servo PWM 
- PB7 - engine 2 PWM (top-down)
- PB6 - engine 1 PWM (forward)
- PE7 i PE9 - engine 1 control pins
- PE10 i PE12 - engine 2 control pins

#### Bluetooth HC-05:
- PC10 - line TX
- PC11 - line RX

#### Sensor HC-Sr04:
- PD3 - trigger sensor HC-Sr04
- PA0 - echo sensor HC-Sr04

#### ADC:
- PA1 - read from ADC<br />
For the safety of the battery, the voltage is measured and power is cut off when battery is low.

#### Control data:
- xyzab~
- x - servos turn in the values between 0 and 85
- y - engine 1 speed in the values between 0 and 120
- z - engine 1 speed in the values between 0 and 120
- a - rotate direction of engine 1 in the values of 0 or 1
- b - rotate direction of engine 2 in the values of 0 or 1
- sign "~" means that this is the end of data

## Tools:

CoCOX CoIDE

## How to run:

Assemble all parts of the description. Build the body of hovercraft and go to next point.

## How to compile:

Download the project and compile it with CooCox CoIDE.

## Future improvements:

- Bottom engine is a bit weak, so hovercraft floats quite low.
- Sometimes there are voltage drops.
- Not tested on water.

## License:

MIT

## Credits:

Maciej Marciniak<br />
Krzysztof Łuczak

The project was conducted during the Microprocessor Lab course held by the Institute of Control and Information Engineering, Poznan University of Technology.<br />
Supervisor: Tomasz Mańkowski
