//
// Fil: TIM_metodar.c
// m.t.311013
//---------------------------------------

//---------------------------------------
// Inklusjonar og definisjonar
//---------------------------------------

#include <cmsis_boot/stm32f30x.h>
#include <cmsis_lib/stm32f30x_gpio.h>
#include <cmsis_lib/stm32f30x_rcc.h>
#include <cmsis_lib/stm32f30x_tim.h>

#include <extern_dekl_globale_variablar.h>

//---------------------------------------
// Funksjonsprototypar
//---------------------------------------
void TIM_oppstart(void);
void PWM_sett_vidde_TIM2_CH2(uint32_t vidde);

void TIM3_oppstart(void);
void TIM3_sett_frekvens(uint32_t frekvens);
void TIM4_oppstart(void);


//---------------------------------------
// Funksjonsdeklarasjonar
//---------------------------------------

void TIM_oppstart(void) {
//
// TIM2 CH2 er satt opp som en PWM utgang på pinne PA1.

 //Deklarasjon av initialiseringsstrukturane.
   TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   TIM_OCInitTypeDef        TIM_OCInitStructure;

// Oppsett av TIM2 CH2 som PWM-utgang

 //Slepp f�rst til klokka paa TIM2.
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);


   /* Time base configuration */
   TIM_TimeBaseStructure.TIM_Period = PWM_periode; //
   TIM_TimeBaseStructure.TIM_Prescaler = PWM_preskalering; // 0
   TIM_TimeBaseStructure.TIM_ClockDivision = 0;
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

   TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

   /* PWM1 Mode configuration: Channel2 */
   TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
   TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
   TIM_OCInitStructure.TIM_Pulse = PWM_vidde;
   TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

   TIM_OC2Init(TIM2, &TIM_OCInitStructure);

//   TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);



//Så oppsett av GPIO-pinnen PA1 som blir brukt av TIM2-modulen
//------------------------------------------
//Deklarasjon av initialiseringsstrukturen.
	GPIO_InitTypeDef GPIO_InitStructure_TIM2;

  //Slepp til klokka paa GPIO-portA.
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE); // | RCC_AHBPeriph_AFIO

  //Konfigurer PA1.
	GPIO_InitStructure_TIM2.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure_TIM2.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure_TIM2.GPIO_Speed = GPIO_Speed_Level_1;
	GPIO_InitStructure_TIM2.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  //Initialiser, dvs. last ned konfigurasjonen i modulen
	GPIO_Init(GPIOA, &GPIO_InitStructure_TIM2);

  //Knytt TIM2-pinnane til AF */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_1);



	// Aktiver til slutt TIM2

    TIM_Cmd(TIM2, ENABLE);
}


//
void PWM_sett_vidde_TIM2_CH2(uint32_t vidde)  {

    TIM_OCInitTypeDef        TIM_OCInitStructure;

    /* PWM1 Mode configuration: Channel2 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = vidde;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC2Init(TIM2, &TIM_OCInitStructure);

    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);

    /* TIM2 enable counter */
    TIM_Cmd(TIM2, ENABLE);
}
// SETT OPP TIM3 til PWM på PB4
void TIM3_oppstart(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    // --- KLokke ---
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    // --- TIM3 Base: 16 kHz PWM ---
    TIM_TimeBaseStructure.TIM_Period = 0;        // 0..999
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    // --- PWM på CH1 (PB4) ---
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;                         // Start med 0%
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);

    // --- GPIO PB4 (TIM3_CH1) ---
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_2);  // TIM3_CH1



    // --- Aktiver TIM3 ---
    TIM_Cmd(TIM3, ENABLE);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
}

void TIM3_sett_frekvens(uint32_t frekvens)
{


    // VIKTIG: Timer kjører på 72 MHz (ingen prescaler!)
    // Formel: ARR = 72 000 000 / (2 × ønsket frekvens) - 1
    // Gir nøyaktig 50% duty og riktig frekvens
	uint32_t period = 72000000UL / frekvens;
    uint32_t arr = period-1;
    uint32_t ccr = period / 2;        // nøyaktig 50% (pga. avrunding opp)

    // Stopp timeren kort
    //TIM_Cmd(TIM3, DISABLE);

    //TIM3->PSC  = 0;        // ingen prescaler → teller på 72 MHz
    TIM3->ARR  = arr;
    TIM3->CCR1 = ccr;      // 50% duty

    TIM3->EGR = TIM_EGR_UG;   // tving oppdatering nå
    TIM_Cmd(TIM3, ENABLE);
}



/*
 * Setter opp TIM4 til å lage et 4 kHz 50% duty klokkesignal til SCF filteret. Dette signalet skal
 * ut på pinne PD12. Bruker kanal 1, CH1, i TIM4 for å lage dette.
 * I denne funksjonen skriver vi direkte til registerne i TIM4.
 * Vi bruker driverfunksjonene/bibliotekene for å sette opp GPIO PD12.
 */
void TIM4_oppstart(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;
    GPIO_InitTypeDef        GPIO_InitStructure;

    /* --- Klokker --- */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);

    /* --- GPIO PD12 som TIM4_CH1 --- */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_2);

    /* --- Timer base: 4 kHz --- */
    TIM_TimeBaseStructure.TIM_Prescaler     = 0;            // 72 MHz
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period        = 18000 - 1;    // 4 kHz
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    /* --- PWM CH1 --- */
    TIM_OCInitStructure.TIM_OCMode       = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState  = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse        = 9000;           // 50 % duty
    TIM_OCInitStructure.TIM_OCPolarity   = TIM_OCPolarity_High;
    TIM_OC1Init(TIM4, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM4, ENABLE);

    /* --- Start timer --- */
    TIM_Cmd(TIM4, ENABLE);
}




