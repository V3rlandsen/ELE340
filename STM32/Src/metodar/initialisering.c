//
// Fil: initialisering.c
// m.t.160913
//---------------------------------------

//---------------------------------------
// Inklusjonar og definisjonar
//---------------------------------------

#include <cmsis_boot/stm32f30x.h>

//#include <stm32f3_discovery/stm32f3_discovery_lsm303dlhc.h>


//---------------------------------------
// Funksjonsprototypar
//---------------------------------------

void maskinvare_init(void);
void GPIO_oppstart(void);

void SysTick_oppstart(void);
void TIM_oppstart(void);
void SPI1_oppstart(void);
void SPI2_oppstart(void);
void Exp_click_sokkel1_oppstart(void);
void LinMot_oppstart(void);
void aks_oppstart(void);
void gyro_oppstart(void);
void avbrot_oppstart(void);
void USART2_oppstart(void);
void USART3_oppstart(void);
void ADC_oppstart(void);
void TIM3_oppstart(void);
void TIM4_oppstart(void);

void CAN_oppstart(void);
//---------------------------------------
// Funksjonsdeklarasjonar
//---------------------------------------

void maskinvare_init(void) {
	GPIO_oppstart();
	SPI1_oppstart();
	SPI2_oppstart();
	Exp_click_sokkel1_oppstart();
	aks_oppstart();
	gyro_oppstart();
	//TIM_oppstart(); Tim2 ble kun brukt til testing
	SysTick_oppstart();
	avbrot_oppstart();

	USART2_oppstart();
	USART3_oppstart();

	TIM3_oppstart();
	TIM4_oppstart();
	ADC_oppstart();



}





