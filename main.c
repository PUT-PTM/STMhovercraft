/******************************************************************************************************
 *														 PINY:
 * B8 - serwo PWM
 * B7 - silnik 2 PWM (unoszacy)
 * B6 - silnik 1 PWM (naped)
 *
 * E7 i E9 - piny sterujace silnikiem 1
 * E10 i E12 - piny sterujace silnikiem 2
 *
 *Bluetooth HC-05:
 * C10 - linia TX
 * C11 - linia RX
 *
 *Czujnik HC-Sr04:
 * PD3 - wyzwalacz czujnika HC-Sr04
 * PA0 - echo czujnika HC-Sr04
 *
 * 													Dane sterujace:
 * xyzab
 * x - skret serwa w wartosciach miedzy 45, a 95
 * y - obroty silnika 1 w wartosciach miedzy 0, a 120
 * z - obroty silnika 2 w wartosciach miedzy 0, a 120
 * a - kierunek obrotow silnika 1 w wartosciach 0 lub 1
 * b - kierunek obrotow silnika 2 w wartosciach 0 lub 1
 ******************************************************************************************************/

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_usart.h"
#include "misc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"
#include "stm32_ub_hcsr04.h"
#include "konfiguracje.h"

static __IO uint32_t TimingDelay; // zmienna pomocna do zegara SysTick
volatile uint16_t dane_serwo = 1200; // wartosc srodkowa serwa
volatile uint16_t dane_silnik1 = 0; // silnik napedzajacy, wylaczony, maks 60000, min 0
volatile uint16_t dane_silnik2 = 0; // silnik unoszacy, wylaczony, wlaczony 60000
volatile uint8_t kierunek_silnik1 = 1; // silnik napedzajacy, kierunek w przod
volatile uint8_t kierunek_silnik2 = 1; // silnik unoszacy, kierunek unoszacy
volatile uint8_t odl = 0; // zmienna przechowujaca odleglosc w cm, od przeszkody z czujnika HC-Sr04
volatile uint16_t sprawdz = 0; // licznik zapobiegajacy utknieciu w petli w przypadku zlego odebrania danych
volatile uint16_t licznik_danych = 0; // licznik odebranych wlasciwie danych
volatile uint16_t licznik = 0; // sluzy do sprawdzania jak dlugo nie odebrano nowych danych

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

/************************************************************************************************
 Funkcja czytajaca 5-znakowe lancuchy sterujace
 *************************************************************************************************/
void read_string() {
	const int16_t dlugosc = 6;
	uint8_t d[dlugosc]; // roboczy lancuch znakow
	licznik_danych = 0; // licznik odebranych wlasciwie danych
	sprawdz = 0; // licznik zapobiegajacy utknieciu w petli w przypadku zlego odebrania danych
	for (licznik_danych = 0; licznik_danych < dlugosc;) {
		sprawdz++;
		while (USART_GetFlagStatus(USART3, USART_FLAG_RXNE) == SET) {
			d[licznik_danych] = USART_ReceiveData(USART3);
			licznik_danych++;
			sprawdz = 0;
		}
		// jesli przekroczono dopuszczalny prog bledu to wyjdz z petli i anuluj dane
		if (sprawdz > 30000) {
			licznik_danych = 0;
			break;
		}
	}
	// przypisuje tylko dane odebrane we wlasciwy sposob
	if (licznik_danych == dlugosc && d[5]=='~') {
		/* konwertowanie lancuchow znakowych do poszczegolnych zmiennych sterujacy */
		dane_serwo = (uint16_t)(d[0] * 20);
		dane_silnik1 = (uint16_t)(d[1] * 500);
		dane_silnik2 = (uint16_t)(d[2] * 500);
		kierunek_silnik1 = d[3];
		kierunek_silnik2 = d[4];
	}
}

/************************************************************************************************
 Funkcja wysylajaca char przez USART
 *************************************************************************************************/
void send_char(uint16_t c) {
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
		;
	USART_SendData(USART3, c);
}

/************************************************************************************************
 Funckja wykonujaca sie podczas wywolania przerwania dla USARTa
 ************************************************************************************************/
void USART3_IRQHandler(void) {
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
		read_string(); // odczytanie lancucha sterujacego
		licznik = 0;
		if (licznik_danych == 5)
			send_char(odl); // wyslanie odleglosci odczytanej z HC-Sr04 do kontrolera
		while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET) {
		}
	}
}

/*******************************************************************************************
 MAIN
 *******************************************************************************************/
int main(void) {
	SystemInit();
	Zegar(); // inicjalizacja zegarow
	Config_Tx(); // konfiguracja linii TX
	Config_Rx(); // konfiguracja linii RX
	Config_USART(); //konfiguracja USART
	Config_NVIC(); // konfiguracja przerwan USART
	USART_Cmd(USART3, ENABLE); // uruchomienie USART
	NVIC_EnableIRQ(USART3_IRQn); // wlaczenie przerwan dla USART
	GPIO(); // inicjalizacja portow GPIO
	Timer4(); // konfiguracja zegara TIM4
	PWM(); // inicjalizacja i konfiguracja PWM
	UB_HCSR04_Init(); // inicjalizacja czujnika odlegosci HC-Sr04

	/* ustawienie zegara SySTick tak by odmierzal 1ms */
	if (SysTick_Config(SystemCoreClock / 1000)) {
		while (1)
			;
	}

	/* glowna petla programu */
	while (1) {
		/* umozliwia zmiane kierunku oborotow silnika napedzajacego */
		if (kierunek_silnik1 != 0) { // jesli nie zero to kierunek w przod
			GPIO_SetBits(GPIOE, GPIO_Pin_7);
			GPIO_ResetBits(GPIOE, GPIO_Pin_9);
		}
		if (kierunek_silnik1 == 0) { // jesli zero to kierunek w tyl
			GPIO_ResetBits(GPIOE, GPIO_Pin_7);
			GPIO_SetBits(GPIOE, GPIO_Pin_9);
		}
		/* umozliwia zmiana kierunku oborotow silnika unoszacego */
		if (kierunek_silnik2 != 0) { // jesli nie zero to kierunek unoszacy
			GPIO_SetBits(GPIOE, GPIO_Pin_12);
			GPIO_ResetBits(GPIOE, GPIO_Pin_10);
		}
		if (kierunek_silnik2 == 0) { // jesli zero to kierunek zasycaj¹cy
			GPIO_ResetBits(GPIOE, GPIO_Pin_12);
			GPIO_SetBits(GPIOE, GPIO_Pin_10);
		}
		/* sprawdza czy licznik przekroczyl 5000 (okolo 5 sekund) i czy odlegosc odczytana z czujnika jest wieksza od 50cm 
		przypisywanie wartosci co 50ms w celu zredukowania spadkow napiec*/
		if (licznik < 5000 && odl > 50) { // jesli spelnione to praca normalna
			TIM4->CCR1 = dane_silnik1; // przypisanie wartosci PWM do silnika napedzajacego
			Delay(50);
			TIM4->CCR2 = dane_silnik2; // przypisanie wartosci PWM do silnika unoszacego
			Delay(50);
			TIM4->CCR3 = dane_serwo; // przypisanie wartosci PWM do serwa
			Delay(50);
		} else { // w przeciwnym wypadku wylacz silniki
			TIM4->CCR1 = 0;
			Delay(50);
			TIM4->CCR2 = 0;
			Delay(50);
			TIM4->CCR3 = 1200;
			Delay(50);
			// hamowanie silnika 2
			GPIO_ResetBits(GPIOE, GPIO_Pin_12);
			GPIO_ResetBits(GPIOE, GPIO_Pin_10);
			// hamowanie silnika 1
			GPIO_ResetBits(GPIOE, GPIO_Pin_9);
			GPIO_ResetBits(GPIOE, GPIO_Pin_7);
		}
		if(licznik < 10000)
		licznik += 150; // zwieksza licznik o ilosc milisekund opoznienia
		odl = (uint8_t)(UB_HCSR04_Distance_cm()); //odczytywanie odleglosci z czujnika odleglosci HC-Sr04
	}
}
