# Hovercraft
Remote-controlled hovercraft with two engines. Powered by a large battery.


## Description
- microcontroller STM32f4
- Bluetooth module HC-05 for communication with remote controller (or PC)
- distance sensor HC-SR04
- body hovercraft
- servomechanism S90
- two engines with propellers
- battery 9.6V
- controller L298 with radiat

#### PINs scheme
- B8 - serwo PWM 
- B7 - engine 2 PWM (top-down)
- B6 - engine 1 PWM (forward)
- E7 i E9 - engine 1 control pins
- E10 i E12 - engine 2 control pins

#### Bluetooth HC=05:
- C10 - line TX
- C11 - line RX

#### Sensor HC-Sr04:
- PD3 - trigger sensor HC-Sr04
- PA0 - echo sensor HC-Sr04

#### Control data:
- xyzab
- x - servos turn in the values between 0 and 85
- y - engine 1 speed in the values between 0 and 120
- z - engine 1 speed in the values between 0 and 120
- a - rotate direction of engine 1 in the values of 0 or 1
- b - rotate direction of engine 2 in the values of 0 or 1

## Tools:



## How to run:



## How to compile:



## Future improvements:



## License:

MIT

## Credits:

Maciej Marciniak<br />
Krzysztof Łuczak

The project was conducted during the Microprocessor Lab course held by the Institute of Control and Information Engineering, Poznan University of Technology.<br />
Supervisor: Michał Fularz
