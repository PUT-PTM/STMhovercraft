# Hovercraft
Projekt budowy i zaprogramowania poduszkowca zdalnie sterowanego.

## Potrzebne materiały
- mikrokontroler STM32f4
- moduł Bluetooth HC-05 do komunikacji z pilotem (lub aplikacją komputerową)
- czujnik odległości HC-SR04
- korpus poduszkowca
- serwomachanizm S90
- dwa silniki ze śmigłami
- bateria 9.6V
- sterownik L298 z radiatorem

### Opis PIN'ów
- B8 - serwo PWM 
- B7 - silnik 2 PWM (unoszący)
- B6 - silnik 1 PWM (napęd)
- E7 i E9 - piny sterujące silnikiem 1
- E10 i E12 - piny sterujące silnikiem 2

### Bluetooth HC=05:
- C10 - linia TX
- C11 - linia RX

### Czujnik HC-Sr04:
- PD3 - wyzwalacz czujnika HC-Sr04
- PA0 - echo czujnika HC-Sr04

### Dane sterujące:
- xyzab
- x - skręt serwa w wartościach między 0, a 85
- y - obroty silnika 1 w wartościach między 0, a 120
- z - obroty silnika 2 w wartościach między 0, a 120
- a - kierunek obrotów silnika 1 w wartościach 0 lub 1
- b - kierunek obrotów silnika 2 w wartościach 0 lub 1
