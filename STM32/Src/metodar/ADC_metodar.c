/*
 * ADC_metodar.c
 *
 *  Created on: Oct 16, 2025
 *      Author: Bruker
 */

#include <cmsis_boot/stm32f30x.h>
#include <cmsis_lib/stm32f30x_gpio.h>
#include <cmsis_lib/stm32f30x_adc.h>
#include <cmsis_lib/stm32f30x_rcc.h>
#include <cmsis_lib/stm32f30x_misc.h>

void ADC_oppstart(void);

volatile uint16_t AD_RESULT = 0;

void ADC_oppstart(void)
{
    // Aktiver klokker
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC34, ENABLE);

    // Sett PB1 til analog inngang
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    // ADC init
    ADC_InitTypeDef ADC_InitStructure;
    ADC_StructInit(&ADC_InitStructure);

    // ADC init (SPL)
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    //ADC_InitStructure.ADC_ContinuousConvMode =ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode =DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_OverrunMode = DISABLE;
    ADC_InitStructure.ADC_AutoInjMode = DISABLE;
    ADC_InitStructure.ADC_NbrOfRegChannel = 1;
    ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;

    // Reset ADC3/ADC4 periferi
    RCC_AHBPeriphResetCmd(RCC_AHBPeriph_ADC34, ENABLE);
    RCC_AHBPeriphResetCmd(RCC_AHBPeriph_ADC34, DISABLE);

    // Sett ADC-klokke
    RCC_ADCCLKConfig(RCC_ADC34PLLCLK_Div1);

    // Kalibrer ADC
    ADC_Cmd(ADC3, DISABLE);
    ADC_StartCalibration(ADC3);


    // Initier ADC
    ADC_Init(ADC3, &ADC_InitStructure);
    ADC_RegularChannelConfig(ADC3, ADC_Channel_1, 1, ADC_SampleTime_19Cycles5);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = ADC3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Aktiver ADC
    ADC_Cmd(ADC3, ENABLE);
    while (!ADC_GetFlagStatus(ADC3, ADC_FLAG_RDY)) { __NOP(); }



    // Aktiver avbrudd
    //ADC_ITConfig(ADC3, ADC_IT_EOC, ENABLE);

    // Start første konvertering
    ADC_StartConversion(ADC3);


}
/*
// IRQ-handler
void ADC3_IRQHandler(void)
{
    if (ADC_GetITStatus(ADC3, ADC_IT_EOC))
    {
        AD_RESULT = ADC_GetConversionValue(ADC3);
        ADC_ClearITPendingBit(ADC3, ADC_IT_EOC);
        ADC_StartConversion(ADC3);

    }
}*/
