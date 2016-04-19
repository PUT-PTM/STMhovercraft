/*************************************************************************************************************
 *														 PINY:
 * B8 - serwo
 * B7 - silnik 2 (unosz�cy)
 * B6 - silnik 1 (nap�d)
 *
 * E7 i E9 - piny seruj�ce silnikiem 1
 * E10 i E12 - piny steruj�ce silnikiem 2
 *
 * C10 - linia TX
 * C11 - linia RX
 *
 * PD3 - wyzwalacz czujnika HC04
 * PA0 - echo czujnika HC04
 *
 * 													Dane steruj�ce:
 * xxxxyyyyyyzzzzzab
 * x - skr�t serwa
 * y - obroty silnika 1
 * z - obroty silnika 2
 * a - kierunek obrotu silnika 1
 * b - kierunek obrot�w silnika 2
 * np. 1200655006550011 - jazda prosto z maksymaln� pr�dkosci�
 *************************************************************************************************************/

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_usart.h"
#include "misc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"
#include "stm32_ub_hcsr04.h"

static __IO uint32_t TimingDelay; // zmienna pomocna do zegara SysTick
char odebrane[16]; // �a�cuch znak�w przeznaczony do zbierania odbieranych danych
/* Inicjalizacja zmiennych steruj�cych */
volatile uint16_t dane_serwo = 1200; // wartosc srodkowa serwa
volatile uint16_t dane_silnik1 = 0; // silnik napedzajacy, wy�aczony, maks 65500, min 0
volatile uint16_t dane_silnik2 = 0; // silnik unosz�cy, wy�aczony, w�aczony 65500
volatile uint8_t kierunek_silnik1 = 1; // silnik napedzajacy, kierunek w prz�d
volatile uint8_t kierunek_silnik2 = 1; // silnik unoszacy, kierunek unoszacy
volatile float odl = 0; // zmienna przechowuj�ca odleg�osc w cm, od przeszkody z czujnika HC04
char* do_wyslania = "000"; // �a�cuch znak�w przeznaczony do wys�ania do kontrolera

/*******************************************************************************************************
 Funkcje inicjalizujace zegar SysTtick
 ********************************************************************************************************/
void Delay(__IO uint32_t nTime) {
	TimingDelay = nTime;
	while (TimingDelay != 0)
		;
}
void TimingDelay_Decrement(void) {
	if (TimingDelay != 0x00) {
		TimingDelay--;
	}
}
void SysTick_Handler(void) {
	TimingDelay_Decrement();
}

/*******************************************************************************************************
 Funckja inicjalizuj�ca linie steruj�ce
 *******************************************************************************************************/
void Zegar() {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
}

/*******************************************************************************************************
 Funkcja inicjalizuj�ca port�w GPIO
 ********************************************************************************************************/
void GPIO() {
	/* piny steru�ce kierunkami silnik�w */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_10 | GPIO_Pin_9
			| GPIO_Pin_7;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* piny PWM dla serwa i obu silnik�w */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/****************************************************************************************************
 Funkcja configuruj�ca USART
 ****************************************************************************************************/
void Config_USART() {
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
}

/******************************************************************************************************
 Funkcje konfiguruj�ce piny TX i RX wykorzystywane przy transmisji danych
 *******************************************************************************************************/

void Config_Tx() {
	// konfiguracja linii Tx
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void Config_Rx() {
	// konfiguracja linii Rx
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/*********************************************************************************************************
 Funkcja konfiguruj�ca przerwania dla USART
 **********************************************************************************************************/
void Config_NVIC() {
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/**********************************************************************************************************
 Funkcja pomocnicza wykorzystywana do konwertowania �a�cuch�w znakowych na integer
 **********************************************************************************************************/
int CharToInt(const char c) {
	switch (c) {
	case '0':
		return 0;
	case '1':
		return 1;
	case '2':
		return 2;
	case '3':
		return 3;
	case '4':
		return 4;
	case '5':
		return 5;
	case '6':
		return 6;
	case '7':
		return 7;
	case '8':
		return 8;
	case '9':
		return 9;
	default:
		return 0;
	}
}

/*****************************************************************************************************
 Funkcja pomocnicza wykorzystywana do konwertowania liczb na �a�cuchy znakowe
 ******************************************************************************************************/
char IntToChar(const int c) {
	switch (c) {
	case 0:
		return '0';
	case 1:
		return '1';
	case 2:
		return '2';
	case 3:
		return '3';
	case 4:
		return '4';
	case 5:
		return '5';
	case 6:
		return '6';
	case 7:
		return '7';
	case 8:
		return '8';
	case 9:
		return '9';
	default:
		return '0';
	}
}

/************************************************************************************************
 Funkcja czytaj�ca 16-znakowe �a�cuchy steruj�ce
 *************************************************************************************************/
void read_string() {
	char d[16];
	uint8_t i = 0;
	for (i = 0; i < 16;) {
		while (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET) {
			d[i] = USART_ReceiveData(USART3);

			/* funkcja filtruj�ca czy odebrane dane s� prawid�owe (s� liczb�) */
			if (d[i] == '1' || d[i] == '2' || d[i] == '3' || d[i] == '4'
					|| d[i] == '5' || d[i] == '6' || d[i] == '7' || d[i] == '8'
					|| d[i] == '9' || d[i] == '0')
				i++;
		}
	}

	/* konwertowanie �a�cuch�w znakowych do poszczeg�lnych zmiennych sterujacy */
	dane_serwo = CharToInt(d[0]) * 1000 + CharToInt(d[1]) * 100
			+ CharToInt(d[2]) * 10 + CharToInt(d[3]);
	dane_silnik1 = CharToInt(d[4]) * 10000 + CharToInt(d[5]) * 1000
			+ CharToInt(d[6]) * 100 + CharToInt(d[7]) * 10 + CharToInt(d[8]);
	dane_silnik2 = CharToInt(d[9]) * 10000 + CharToInt(d[10]) * 1000
			+ CharToInt(d[11]) * 100 + CharToInt(d[12]) * 10 + CharToInt(d[13]);
	kierunek_silnik1 = CharToInt(d[14]);
	kierunek_silnik2 = CharToInt(d[15]);
}

/************************************************************************************************
 Funkcje umo�liwiaj�ce wysy�anie �a�cuch�w znakowych
 *************************************************************************************************/
void send_char(char c) {
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
		;
	USART_SendData(USART3, c);
}

void send_string(const char* s) {
	while (*s) {
		send_char(*s++);
	}
}

/************************************************************************************************
 Funckja wykonujaca si� podczas wywo�ania przerwania dla USARTa
 ************************************************************************************************/
void USART3_IRQHandler(void) {
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
		read_string(); // odczytanie �a�cuchu sterujacego
		odl = UB_HCSR04_Distance_cm(); //odczytywanie odleg�osci z czujnika odleglosci HC04

		/* konwertowanie odleglosci do lancuhu znak�w */
		int8_t s = (int) (odl / 100);
		do_wyslania[0] = IntToChar(s);
		int8_t d = (int) ((odl - 100 * s) / 10);
		do_wyslania[1] = IntToChar(d);
		int8_t j = odl - 100 * s - d * 10;
		do_wyslania[2] = IntToChar(j);
		send_string(do_wyslania); // wys�anie odleglosci odczyntanej z HC04 do kontolera
		while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET) {
		}
	}
}

/***************************************************************************************************
 Funckja inicjalizuj�ca zegar TIM4 dla PWM
 ***************************************************************************************************/
void Timer4() {
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Period = 65500;
	TIM_TimeBaseStructure.TIM_Prescaler = 83;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM4, ENABLE);
}

/***************************************************************************************************
 Funkcja inicjalizuj�ca PWM
 ***************************************************************************************************/
void PWM() {
	TIM_OCInitTypeDef TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	/* KONFIGURACJA 1 KANA�U */
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* KONFIGURACJA 2 KANA�U */
	TIM_OC2Init(TIM4, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* KONFIGURACJA 3 KANA�U */
	TIM_OC3Init(TIM4, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_TIM4);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB, GPIO_Pin_6);
	GPIO_SetBits(GPIOB, GPIO_Pin_7);
	GPIO_SetBits(GPIOB, GPIO_Pin_8);
}

/*******************************************************************************************
 MAIN
 *******************************************************************************************/

int main(void) {
	SystemInit();
	Zegar(); // inicjalizacja zegar�w
	Config_Tx(); // konfiguracja linii TX
	Config_Rx(); // konfiguracja linii RX
	Config_USART(); //konfiguracja USART
	Config_NVIC(); // konfiguracja przerwa� USART
	USART_Cmd(USART3, ENABLE); // uruchomienie USART
	NVIC_EnableIRQ(USART3_IRQn); // w��czenie przerwa� dla USART
	GPIO(); // inicjalizacja port�w GPIO
	Timer4(); // konfikuracja zegara TIM4
	PWM(); // inicjalizacja i konfiguracja PWM

	/* ustawienie zegara SySTick tak by odmierza� 1ms */
	if (SysTick_Config(SystemCoreClock / 1000)) {
		while (1)
			;
	}

	/* g��wna p�tla programu */
	while (1) {
		/* umozliwia zmian� kierunku obor�t�w silnika napedz�j�cego */
		if (kierunek_silnik1 != 0) { // jesli nie zero to kierunek w prz�d
			GPIO_ResetBits(GPIOE, GPIO_Pin_7);
			GPIO_ResetBits(GPIOE, GPIO_Pin_9);

			GPIO_SetBits(GPIOE, GPIO_Pin_7);
			GPIO_ResetBits(GPIOE, GPIO_Pin_9);
		}
		if (kierunek_silnik1 == 0) { // jesli zero to kierunek w ty�
			GPIO_ResetBits(GPIOE, GPIO_Pin_7);
			GPIO_ResetBits(GPIOE, GPIO_Pin_9);

			GPIO_ResetBits(GPIOE, GPIO_Pin_7);
			GPIO_SetBits(GPIOE, GPIO_Pin_9);
		}
		/* umozliwia zmian� kierunku ob�r�t�w silika unosz�cego */
		if (kierunek_silnik2 != 0) { // jesli nie zero to kierunek unosz�cy
			GPIO_ResetBits(GPIOE, GPIO_Pin_12);
			GPIO_ResetBits(GPIOE, GPIO_Pin_10);

			GPIO_SetBits(GPIOE, GPIO_Pin_12);
			GPIO_ResetBits(GPIOE, GPIO_Pin_10);
		}
		if (kierunek_silnik2 == 0) { // jesli zero to kierunek zasycaj�cy
			GPIO_ResetBits(GPIOE, GPIO_Pin_12);
			GPIO_ResetBits(GPIOE, GPIO_Pin_10);

			GPIO_ResetBits(GPIOE, GPIO_Pin_12);
			GPIO_SetBits(GPIOE, GPIO_Pin_10);
		}
		TIM4->CCR1 = dane_silnik1; // przypisanie wartosci PWM do silnika napedzaj�cego
		TIM4->CCR2 = dane_silnik2; // przypisanie wartosci PWM do silnika unoszacego
		TIM4->CCR3 = dane_serwo; // przypisanie wartosci PWM do serwa
		Delay(100); // op�nienie 0.1s
	}
}
