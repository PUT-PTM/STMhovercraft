# Hovercraft
Remote-controlled hovercraft with two engines. Powered by battery.<br />
[Hovercaft remote control project] (https://github.com/PUT-PTM/STMhovercraftRemote)

## Demo
https://drive.google.com/folderview?id=0B-LZLyx-e6vsSm80X2F5YXJQZ0U&usp=sharing

## Description
- microcontroller STM32f4
- Bluetooth module HC-05 for communication with remote controller (or PC)
- distance sensor HC-SR04
- hovercraft body
- servomechanism TowerPro MG-91
- two engines with fans
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

#### Control instructions:
- xyzab~
- x - servo turn in the values between 40 and 80
- y - engine 1 speed in the values between 0 and 120
- z - engine 2 speed in the values between 0 and 120
- a - rotate direction of engine 1 in the values of 33 or others
- b - rotate direction of engine 2 in the values of 33 or others
- sign "~" means the end of the command

## Tools:

CoCOX CoIDE

## How to run:

Charge battery, plug the engines, make sure that all connections are right and stable.

## How to compile:

Download the project and compile it with CooCox CoIDE.

## Future improvements:

- Bottom engine is a bit weak, so hovercraft floats quite low.
- While running voltage could drop to low level.
- Not tested on water.

## License:

MIT

## Credits:

Maciej Marciniak<br />
Krzysztof Łuczak

The project was conducted during the Microprocessor Lab course held by the Institute of Control and Information Engineering, Poznan University of Technology.<br />
Supervisor: Tomasz Mańkowski
