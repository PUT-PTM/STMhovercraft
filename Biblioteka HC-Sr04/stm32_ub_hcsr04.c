//--------------------------------------------------------------
// File     : stm32_ub_hcsr04.c
// Datum    : 22.12.2013
// Version  : 1.0
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : GPIO, TIM, MISC
// Funktion : Ultraschall Näherungs-Sensor (HC-SR04)
//            Messbereich (laut Datenblatt) 2cm bis 400cm
//
// Pinout   : Trigger = PD3
//            Echo    = PA0
//
// Hinweis  : der Echo-Pin muss ein Input-Capture-Pin
//            vom benutzten Timer (hier Timer-2) sein
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_hcsr04.h"



//--------------------------------------------------------------
// interne Funktionen
//--------------------------------------------------------------
void P_HCSR04_InitIO(void);
void P_HCSR04_InitTIM(void);
void P_HCSR04_InitNVIC(void);
void P_HCSR04_Trigger(void);



//--------------------------------------------------------------
// init vom HC-SR04 Sensor
//--------------------------------------------------------------
void UB_HCSR04_Init(void)
{
  // init der Variabeln
  HCSR04.t2_akt_time=0;
  HCSR04.t7_akt_time=0;
  HCSR04.delay_us=0;
  // init der IO-Pins
  P_HCSR04_InitIO();
  // init der Timer
  P_HCSR04_InitTIM();
  // init vom NVIC
  P_HCSR04_InitNVIC();
}


//--------------------------------------------------------------
// Abstandsmessung per HC-SR04 Sensor
// Hinweis : nach der Messung wird ein Delay von 60ms eingefügt
//
// return_wert
//   >0  =  gemessener Abstand in cm (z.B. 23.4 = 23,4 cm)
//   -1  =  fehler bei der Messung
//--------------------------------------------------------------
float UB_HCSR04_Distance_cm(void)
{
  float ret_wert=0.0;
  uint32_t ok=0;

  //-----------------------------------
  // Timer-2 reset
  //-----------------------------------
  HCSR04.t2_akt_time=0;
  // Timer reset
  TIM_SetCounter(TIM2, 0);
  // Timer enable
  TIM_Cmd(TIM2, ENABLE);
  //-----------------------------------
  // Sensor Trigger Signal ausgeben
  //-----------------------------------
  P_HCSR04_Trigger();

  //-----------------------------------
  // warten bis Messung fertig
  // oder Timeout per Timer-7
  //-----------------------------------
  ok=0;
  HCSR04.t7_akt_time=0;
  // Timer enable
  TIM_Cmd(TIM7, ENABLE);
  do {
    if(HCSR04.t2_akt_time!=0) ok=1;               // wenn Messung fertig
    if(HCSR04.t7_akt_time>=HCSR04_TIMEOUT) ok=2;  // bei Timeout
  }while(ok==0);

  //-----------------------------------
  // Timer disable
  //-----------------------------------
  TIM_Cmd(TIM7, DISABLE);
  TIM_Cmd(TIM2, DISABLE);

  //-----------------------------------
  // Delay (notwendig laut Datenblatt)
  //-----------------------------------
  HCSR04.t7_akt_time=0;
  // Timer enable
  TIM_Cmd(TIM7, ENABLE);
  //-----------------------------------
  // warten bis Delay vorbei
  //-----------------------------------
  while(HCSR04.t7_akt_time<HCSR04_DELAY);
  // Timer disable
  TIM_Cmd(TIM7, DISABLE);

  //-----------------------------------
  // Auswertung
  //-----------------------------------
  if(ok==1) {
    // umwandeln der gemessenen Delay Zeit (in us)
    // in einen Abstandswert (in cm)
    ret_wert=(float)(HCSR04.delay_us)*(float)(HCSR04_FAKTOR_US_2_CM);
  }
  else {
    // Fehlmessung (Timeout)
    ret_wert=0.0;
  }

  return(ret_wert);
}


//--------------------------------------------------------------
// interne Funktion
// Trigger-Signal (10us) ausgeben
// (per Timer-7)
//--------------------------------------------------------------
void P_HCSR04_Trigger(void)
{
  HCSR04.t7_akt_time=0;
  //-----------------------------------
  // Trigger-Pin Hi-Pegel ausgeben
  //-----------------------------------
  HCSR04_TRIGGER_PORT->BSRRL = HCSR04_TRIGGER_PIN;
  // Timer enable
  TIM_Cmd(TIM7, ENABLE);
  //-----------------------------------
  // warten bis 10us vorbei
  //-----------------------------------
  while(HCSR04.t7_akt_time<10);
  // Timer disable
  TIM_Cmd(TIM7, DISABLE);
  //-----------------------------------
  // Trigger-Pin Lo-Pegel ausgeben
  //-----------------------------------
  HCSR04_TRIGGER_PORT->BSRRH = HCSR04_TRIGGER_PIN;
}


//--------------------------------------------------------------
// interne Funktion
// Init aller IO-Pins
//--------------------------------------------------------------
void P_HCSR04_InitIO(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  //-----------------------------------
  // Trigger-Pin
  //-----------------------------------

  // Clock Enable
  RCC_AHB1PeriphClockCmd(HCSR04_TRIGGER_CLK, ENABLE);

  // Config als Digital-Ausgang
  GPIO_InitStructure.GPIO_Pin = HCSR04_TRIGGER_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(HCSR04_TRIGGER_PORT, &GPIO_InitStructure);

  // Lo-Pegel ausgeben
  HCSR04_TRIGGER_PORT->BSRRH = HCSR04_TRIGGER_PIN;

  //-----------------------------------
  // Echo-Pin
  //-----------------------------------

  // Clock Enable
  RCC_AHB1PeriphClockCmd(HCSR04_ECHO_CLK, ENABLE);

  // Config des Pins als AF-Input
  GPIO_InitStructure.GPIO_Pin = HCSR04_ECHO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(HCSR04_ECHO_PORT, &GPIO_InitStructure);

  // Alternative-Funktion mit dem IO-Pin verbinden
  GPIO_PinAFConfig(HCSR04_ECHO_PORT, HCSR04_ECHO_SOURCE, GPIO_AF_TIM2);
}


//--------------------------------------------------------------
// interne Funktion
// init vom Timer
//--------------------------------------------------------------
void P_HCSR04_InitTIM(void)
{
  TIM_ICInitTypeDef  TIM_ICInitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  //-----------------------------------
  // Timer-7 fuer Delay
  //-----------------------------------

  // Clock enable
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

  // Timer disable
  TIM_Cmd(TIM7, DISABLE);

  // Timer auf 10us einstellen
  TIM_TimeBaseStructure.TIM_Period=HCSR04_TIM7_PERIOD;
  TIM_TimeBaseStructure.TIM_Prescaler=HCSR04_TIM7_PRESCALE;
  TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);

  TIM_ARRPreloadConfig(TIM7, ENABLE);


  //-----------------------------------
  // Timer-2 im Input-Capture Mode
  //-----------------------------------

  // Clock enable
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  // Vorteiler einstellen
  TIM_PrescalerConfig(TIM2, HCSR04_TIM2_PRESCALE, TIM_PSCReloadMode_Immediate);

  TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
  TIM_ICInitStructure.TIM_ICFilter = 0x0;
  TIM_PWMIConfig(TIM2, &TIM_ICInitStructure);

  // input Trigger
  TIM_SelectInputTrigger(TIM2, TIM_TS_TI1FP1);

  // Slave-Mode (Reset)
  TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_Reset);
  TIM_SelectMasterSlaveMode(TIM2,TIM_MasterSlaveMode_Enable);
}


//--------------------------------------------------------------
// interne Funktion
// init vom NVIC
//--------------------------------------------------------------
void P_HCSR04_InitNVIC(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  //-----------------------------------
  // NVIC init für Timer-2
  //-----------------------------------
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);

  //-----------------------------------
  // NVIC init für Timer-7
  //-----------------------------------
  NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  // Update Interrupt enable
  TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE);
}


//--------------------------------------------------------------
// ISR von Timer2
// wird bei Lo-Flanke vom Echo-Signal aufgerufen
//--------------------------------------------------------------
void TIM2_IRQHandler(void)
{
  uint32_t start,stop;

  if(TIM_GetITStatus(TIM2, TIM_IT_CC1) == SET) {
    // Interrupt Flags loeschen
    TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);

    //-----------------------------------
    // Messwerte auslesen
    //-----------------------------------
    start=TIM_GetCapture1(TIM2);
    stop=TIM_GetCapture2(TIM2);
    HCSR04.delay_us=start-stop;
    HCSR04.t2_akt_time++;
  }
}


//------------------------------------------------------------
// Timer-7 ISR
// wird alle 10us aufgerufen
//------------------------------------------------------------
void TIM7_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET) {
    // wenn Interrupt aufgetreten
    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);

    HCSR04.t7_akt_time+=10;
  }
}

