//
// Fil: GPIO_metodar.c
// m.t.210714
//---------------------------------------

//---------------------------------------
// Inklusjonar og definisjonar
//---------------------------------------

#include <cmsis_boot/stm32f30x.h>
#include <cmsis_lib/stm32f30x_gpio.h>
#include <cmsis_lib/stm32f30x_rcc.h>

#include <extern_dekl_globale_variablar.h>

//---------------------------------------
// Funksjonsprototypar
//---------------------------------------

void GPIO_oppstart(void);
void GPIO_sett_kompassmoenster(int8_t verdi);
void GPIO_ventesymbol_step(void);
void GPIO_blink(void);
void GPIO_lys_av(void);
void GPIO_lys_paa(void);
void GPIO_sjekk_USER_brytar(void);
void GPIO_brytaravprelling(void);

//---------------------------------------
// Funksjonsdeklarasjonar
//---------------------------------------

//typedef struct
//{
//  uint32_t GPIO_Pin;              /*!< Specifies the GPIO pins to be configured.
//                                       This parameter can be any value of @ref GPIO_pins_define */
//
//  GPIOMode_TypeDef GPIO_Mode;     /*!< Specifies the operating mode for the selected pins.
//                                       This parameter can be a value of @ref GPIOMode_TypeDef   */
//
//  GPIOSpeed_TypeDef GPIO_Speed;   /*!< Specifies the speed for the selected pins.
//                                       This parameter can be a value of @ref GPIOSpeed_TypeDef  */
//
//  GPIOOType_TypeDef GPIO_OType;   /*!< Specifies the operating output type for the selected pins.
//                                       This parameter can be a value of @ref GPIOOType_TypeDef  */
//
//  GPIOPuPd_TypeDef GPIO_PuPd;     /*!< Specifies the operating Pull-up/Pull down for the selected pins.
//                                       This parameter can be a value of @ref GPIOPuPd_TypeDef   */
//}GPIO_InitTypeDef;
//typedef enum
//{
//  GPIO_Mode_IN   = 0x00, /*!< GPIO Input Mode */
//  GPIO_Mode_OUT  = 0x01, /*!< GPIO Output Mode */
//  GPIO_Mode_AF   = 0x02, /*!< GPIO Alternate function Mode */
//  GPIO_Mode_AN   = 0x03  /*!< GPIO Analog In/Out Mode      */
//}GPIOMode_TypeDef;
//typedef enum
//{
//  GPIO_PuPd_NOPULL = 0x00,
//  GPIO_PuPd_UP     = 0x01,
//  GPIO_PuPd_DOWN   = 0x02
//}GPIOPuPd_TypeDef;
//typedef enum
//{
//  GPIO_OType_PP = 0x00,
//  GPIO_OType_OD = 0x01
//}GPIOOType_TypeDef;

void GPIO_oppstart(void)  {

//GPIO-pinnar til lysdiodar og til test
//------------------------------------------
  //Deklarasjon av initialiseringsstrukturen.
    GPIO_InitTypeDef   GPIO_InitStructure;

//GPIO-pinne PA0 som er kobla til USER-brytaren
//---------------------------------------------
  //Slepp foerst til klokka paa GPIOA-modulen
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  //Konfigurer brytarpinnen
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  //Initialiser, dvs. last ned konfigurasjonen i modulen
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	 //Slepp f�rst til klokka paa GPIO-portB.
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    // --- GPIO PB5 som retning (output) ---
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    //Slepp f�rst til klokka paa GPIO-portE.
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);

  //Konfigurer GPIO_pinnane PE8 - 15 som er kobla til LED-ar.
  //Konfigurer ogsaa pinnen PE7 som er brukt til testing.
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;


  //Initialiser, dvs. last ned konfigurasjonen i modulen
    GPIO_Init(GPIOE, &GPIO_InitStructure);


  //Slepp s� til klokka paa GPIO-portC.
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);



  //Konfigurer GPIO_pinnen PC6 som er brukt til testing av avbrotsmetoden for SysTick.
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  //Initialiser, dvs. last ned konfigurasjonen i modulen
	GPIO_Init(GPIOC, &GPIO_InitStructure);

}


void GPIO_blink(void)  {

//    volatile uint32_t i;

  //Snu pinne PC8 og 9

    GPIOC->ODR ^= (GPIO_Pin_8 | GPIO_Pin_9);

  //Pause
  //   for(i=0;i<0x100000;i++);

}

void GPIO_sett_kompassmoenster(int8_t verdi)  {



  //Legg verdi ut p� pinnane PE8-15
    GPIOE->ODR = verdi << 8;  //;
}

void GPIO_ventesymbol_step(void) {
    // Send kun én bit om gangen
    uint8_t mønster = 1 << (kompass_index & 0x07);  // 0–7

    // Bruk eksisterende funksjon uendret
    GPIO_sett_kompassmoenster(mønster);

    // Neste diode neste gang
    kompass_index++;
    if (kompass_index >= 8) kompass_index = 0;
}

void GPIO_lys_av()  {


  //Gi ut laagt nivaa paa pinne PC8 og 9. La dei andre vera.
//	__disable_irq(); // Viss sysTick ogs� k�yrer GPIOC-pinnar
    GPIOC->ODR = GPIOC->ODR & 0xFCFF;
//  __enable_irq();

}


void GPIO_lys_paa()  {


  //Gi ut hoegt nivaa paa pinne PC8 og 9. La dei andre vera.
//	__disable_irq();  // Viss sysTick ogs� k�yrer GPIOC-pinnar
    GPIOC->ODR = GPIOC->ODR | (GPIO_Pin_8 | GPIO_Pin_9);
//  __enable_irq();

}

void GPIO_sjekk_USER_brytar(void) {

	if (GPIOA->IDR & GPIO_Pin_0 ) { //Er brytaren er trykt inn?
       if(USER_brytar_var_nettopp_slept) { //I s� fall:Var brytaren slept ved f�rre lesing?

    	   USER_brytar_var_nettopp_slept = 0;
    	   gyldig_trykk_av_USER_brytar = 1;      //D� godkjennest dette som eit nytt trykk
//    	   send_ny_tidsserie_1000 = 1;
       }
	}

    else {                 // Viss ikkje brytaren er trykt inn,
    	 USER_brytar_var_nettopp_slept = 1; // s� er brytaren slept.
    }
}

void GPIO_brytaravprelling(void) {

	if (GPIOA->IDR & GPIO_Pin_0) {
	         nedtrykt_er_naa_USER_brytar = 1;
	      }
	      else {
	         nedtrykt_er_naa_USER_brytar = 0;
	      }

	if (nedtrykt_er_naa_USER_brytar ) {
	         sleppteljar_USER_brytar = 0;

	         if (klar_for_nytt_trykk_av_USER_brytar ) {
	                trykkteljar_USER_brytar++;

	                if (trykkteljar_USER_brytar > 2 ) {
	                    gyldig_trykk_av_USER_brytar = 1;
	                    trykkteljar_USER_brytar     = 0;
	                    klar_for_nytt_trykk_av_USER_brytar = 0;
	                }

	         }
	 }
	 else {
	         trykkteljar_USER_brytar = 0;
	         sleppteljar_USER_brytar++;

	         if (sleppteljar_USER_brytar > 2 ) {
	                sleppteljar_USER_brytar = 0;
	                klar_for_nytt_trykk_av_USER_brytar = 1;
	         }
	 }
}
