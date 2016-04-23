//--------------------------------------------------------------
// File     : stm32_ub_hcsr04.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_HCSR04_H
#define __STM32F4_UB_HCSR04_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"


//--------------------------------------------------------------
// Umrechnungsfaktor
//--------------------------------------------------------------
#define  HCSR04_FAKTOR_US_2_CM    0.017  // delay us -> abstand cm


//--------------------------------------------------------------
// Timeout der Messung (in us)
//--------------------------------------------------------------
#define  HCSR04_TIMEOUT           30000  // 30000us = 30ms => (Abstand ca 5m)


//--------------------------------------------------------------
// Delay nach einer Messung (in us)
// (laut Datenblatt notwendig)
//--------------------------------------------------------------
#define  HCSR04_DELAY             60000  // 60ms Delay nach der Messung



//--------------------------------------------------------------
// Echo Pin von HC-SR04 (PA0)
// Hiweis : der PIN muss ein Capture-Pin vom benutzten Timer sein !!
//--------------------------------------------------------------
#define  HCSR04_ECHO_PORT     GPIOA
#define  HCSR04_ECHO_CLK      RCC_AHB1Periph_GPIOA
#define  HCSR04_ECHO_PIN      GPIO_Pin_0
#define  HCSR04_ECHO_SOURCE   GPIO_PinSource0


//--------------------------------------------------------------
// Trigger Pin von HC-SR04 (PD3)
//--------------------------------------------------------------
#define  HCSR04_TRIGGER_PORT  GPIOD
#define  HCSR04_TRIGGER_CLK   RCC_AHB1Periph_GPIOD
#define  HCSR04_TRIGGER_PIN   GPIO_Pin_3


//--------------------------------------------------------------
// Input-Capture Einstellungen (Frequenz vom Timer2)
//
// Grundfrequenz = 2*APB1 (APB1=42MHz) => TIM_CLK=84MHz
// prescale  : 0 bis 0xFFFF
//
// T2_Frq = TIM_CLK/(vorteiler+1)
//--------------------------------------------------------------
#define  HCSR04_TIM2_PRESCALE  83     // prescaler => (83 => T2_Frq = 1 MHz)


//--------------------------------------------------------------
// Delay Einstellungen (von Timer7)
//
// Grundfrequenz = 2*APB1 (APB1=42MHz) => TIM_CLK=84MHz
// prescale  : 0 bis 0xFFFF
//
// T7_Delay = TIM_CLK/(vorteiler+1)/(periode+1)
//--------------------------------------------------------------
#define  HCSR04_TIM7_PRESCALE  83     // prescaler => (83 => T2_Frq = 1 MHz)
#define  HCSR04_TIM7_PERIOD    9      // 10us


//--------------------------------------------------------------
typedef struct {
  uint32_t t2_akt_time;
  uint32_t t7_akt_time;
  uint32_t delay_us;
}HCSR04_t;
HCSR04_t HCSR04;



//--------------------------------------------------------------
// Globale Funktionen
//--------------------------------------------------------------
void UB_HCSR04_Init(void);
float UB_HCSR04_Distance_cm(void);



//--------------------------------------------------------------
#endif // __STM32F4_UB_HCSR04_H
