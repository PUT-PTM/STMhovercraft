# Hovercraft
Projekt budowy i zaprogramowania poduszkowca zdalnie sterowanego.

## Potrzebne materiały
- mikrokontroler STM32f4
- moduł Bluetooth do komunikacji z pilotem (lub aplikacją komputerową)
- czujnik odległości HC04
- korpus poduszkowca
- serwomachanizm
- dwa silniki ze śmigłami
- bateria

### Opis PIN'ów
- B8 - serwo PWM 
- B7 - silnik 2 PWM (unoszący)
- B6 - silnik 1 PWM (napęd)
- E7 i E9 - piny sterujące silnikiem 1
- E10 i E12 - piny sterujące silnikiem 2

### Bluetooth:
- C10 - linia TX
- C11 - linia RX

### Czujnik HC04:
- PD3 - wyzwalacz czujnika HC04
- PA0 - echo czujnika HC04

### Dane sterujace:
- schemat: xxxxyyyyyyzzzzzab
- x - skręt serwa
- y - obroty silnika 1
- z - obroty silnika 2
- a - kierunek obrotóww silnika 1
- b - kierunek obrotóww silnika 2
- np. 1200655006550011 - jazda prosto z maksymalną prędkością
