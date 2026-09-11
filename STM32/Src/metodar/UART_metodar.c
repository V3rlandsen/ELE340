//
// Fil: UART_metodar.c
// m.t.170913
//---------------------------------------

//---------------------------------------
// Inklusjonar og definisjonar
//---------------------------------------

#include "cmsis_boot/stm32f30x.h"
#include "cmsis_lib/stm32f30x_gpio.h"
#include "cmsis_lib/stm32f30x_rcc.h"
#include "cmsis_lib/stm32f30x_usart.h"
#include "cmsis_lib/stm32f30x_misc.h"  // NVIC_InitTypeDef og NVIC_Init
#include "pid.h"

#include <extern_dekl_globale_variablar.h>


//---------------------------------------
// Funksjonsprototypar
//---------------------------------------

//void USART1_oppstart(void);
//void USART1_Put(uint8_t ch);
//uint8_t USART1_Get(void);
//void USART1_skriv(uint8_t ch);
//uint8_t USART1_les(void);
//void USART1_skriv_streng(uint8_t *streng);

void USART2_oppstart(void);
void USART2_Put(uint8_t ch);
uint8_t USART2_Get(void);
void USART2_skriv(uint8_t data);

//uint8_t USART2_les(void);
uint8_t USART2_les(uint8_t* data);

void USART2_skriv_streng(uint8_t *streng);

void USART2_parse_kommando(void);

void USART2_send_hex(char prefix, int32_t verdi, uint8_t bytes);
void USART2_send_tid8_og_data16(uint8_t tid, int16_t loggeverdi);
void USART2_send_tid8_og_data16x3(uint8_t tid, int16_t loggeverdi1, int16_t loggeverdi2, int16_t loggeverdi3);
void USART2_handtering(uint8_t loggedata);
void USART2_handtering1(void);
void USART2_handtering2(uint16_t teljar);
void USART2andtering3(void);
void USART2_EXTI26_IRQHandler(void);
//---------------------------------------
// Ringbuffer-definisjonar
//---------------------------------------
#define USART2_TX_BUFFER_SIZE 128
uint8_t USART2_tx_buffer[USART2_TX_BUFFER_SIZE];
volatile uint16_t USART2_tx_index = 0;
volatile uint16_t USART2_tx_length = 0;

#define USART2_RX_BUFFER_SIZE 128
uint8_t USART2_rx_buffer[USART2_RX_BUFFER_SIZE];
volatile uint16_t USART2_rx_index_in = 0;
volatile uint16_t USART2_rx_index_out = 0;


void USART3_oppstart(void);
void USART3_Put(uint8_t ch);
uint8_t USART3_Get(void);
void USART3_skriv(uint8_t ch);
uint8_t USART3_les(uint8_t* data);
void USART3_skriv_streng(uint8_t *streng);
void USART3_handtering(uint8_t loggedata);
void USART3_send_tid8_og_data16(uint8_t tid, int16_t loggeverdi);
void USART3_send_tid8_og_data16x4(uint8_t tid, int16_t loggeverdi1, int16_t loggeverdi2, int16_t loggeverdi3, int16_t loggeverdi4);
void USART3_skriv_blocking(uint8_t data);
uint8_t USART3_parse_pakke(USART3_Pakke_t* pakke);
void USART3_EXTI28_IRQHandler(void);

//---------------------------------------
// Ringbuffer-definisjonar
//---------------------------------------
#define USART3_TX_BUFFER_SIZE 128
uint8_t USART3_tx_buffer[USART3_TX_BUFFER_SIZE];
volatile uint16_t USART3_tx_index = 0;
volatile uint16_t USART3_tx_length = 0;

#define USART3_RX_BUFFER_SIZE 128
uint8_t USART3_rx_buffer[USART3_RX_BUFFER_SIZE];
volatile uint16_t USART3_rx_index_in = 0;
volatile uint16_t USART3_rx_index_out = 0;

//----------------------------------------------------------------------------
void USART2_oppstart(void)
{

  //Deklarasjon av initialiseringsstrukturane.
    USART_InitTypeDef USART2_InitStructure;
    USART_ClockInitTypeDef  USART2_ClockInitStructure;

    // USART-delen m� fiksast slik at den virkar her dvs. p� STM32F3-kortet
    // der en bruker USART2 i staden for USART1 og med nye GPIO-pinnar, sj� manualen
    // p� stm32f3 disc. shield samt brukarmanualen p� f3-kortet.
    // Det kan ogs� vera at strukturdefinisjonane er noko endra, sj� stm32f30x_usart.h

    //Slepp til klokka
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    USART_ClockStructInit(&USART2_ClockInitStructure);
    USART_ClockInit(USART2, &USART2_ClockInitStructure);

    USART2_InitStructure.USART_BaudRate = 115200; //19200;//57600;//19200; //9600;
    USART2_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART2_InitStructure.USART_StopBits = USART_StopBits_1;
    USART2_InitStructure.USART_Parity =  USART_Parity_No ; //USART_Parity_Odd;
    USART2_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART2_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    //Legg inn konfigurasjonen i modulen
    USART_Init(USART2, &USART2_InitStructure);


   //GPIO-delen m� fiksast slik at den virkar her dvs. p� STM32F3-kortet
   //P� STM32F3: GPIO-pinnane PA2 og 3 brukt mot intern USART2-modul
   //------------------------------------------
  //Deklarasjon av initialiseringsstrukturen.
    GPIO_InitTypeDef GPIO_InitStructure_UART2;

  //Slepp foerst til klokka paa GPIOA-modulen
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  //Sett USART2 Tx (PA2) som AlternativFunksjon og "push-pull" (vanleg totempaale)
    GPIO_InitStructure_UART2.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure_UART2.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure_UART2.GPIO_Speed = GPIO_Speed_Level_1;
    GPIO_InitStructure_UART2.GPIO_OType = GPIO_OType_PP;

  //Initialiser, dvs. last ned konfigurasjonen i modulen
    GPIO_Init(GPIOA, &GPIO_InitStructure_UART2);

  //Knytt pinnen til AF */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_7);// Sj� stm32f30x_gpio.h

  //Sett USART2 Rx (PA3) som flytande inngang ("input floating")
    GPIO_InitStructure_UART2.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure_UART2.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure_UART2.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  //Initialiser, dvs. last ned konfigurasjonen i modulen
    GPIO_Init(GPIOA, &GPIO_InitStructure_UART2);

  //Knytt pinnen til AF */
  	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_7);

  //Aktiver s� USART1
    USART_Cmd(USART2, ENABLE);

    // --- NYE LINJER: Aktiver RX interrupt ---
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

  //Send til slutt her velkomst via UART/USB-modul

    //USART2_skriv('k');
	USART2_skriv_streng((uint8_t *)"--\nSTM32F3 er klar!\n\r");  // Ny linje og retur til linjestart etterp�.


}

void USART2_Put(uint8_t ch)
{
    USART_SendData(USART2, (uint8_t) ch); //Loop until the end of transmission
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
        {
        ;
        }

}

uint8_t USART2_Get(void)
{
    while ( USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET)
        {
        ;
        }
    return (uint8_t)USART_ReceiveData(USART2);
}

/*
uint8_t USART2_les(void)
{
   if ( USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET)
        {
	   return (uint8_t)USART_ReceiveData(USART2);
   }
   else {
       return (uint8_t)0x00;
   }
}
*/

uint8_t USART2_les(uint8_t* data)
{
    if (USART2_rx_index_in != USART2_rx_index_out) {
        *data = USART2_rx_buffer[USART2_rx_index_out];
        USART2_rx_index_out = (USART2_rx_index_out + 1) % USART2_RX_BUFFER_SIZE;
        return 1;
    }
    return 0;
}
/*
void USART2_skriv(uint8_t data)
{
    USART_SendData(USART2, (uint8_t) data); //Loop until the end of transmission
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
        {
        ;
        }

}*/
void USART2_skriv(uint8_t data)
{

    USART2_tx_buffer[USART2_tx_length++] = data;
    USART_ITConfig(USART2, USART_IT_TXE, ENABLE);

}

void USART2_skriv_streng(uint8_t *streng)
{
    while( *streng != 0) {   // Skriv ut ein 0-terminert tekststreng
    	USART2_skriv(*streng);
    	streng++;
    }
}

void USART2_parse_kommando(void)
{
    char ch;  // ← VIKTIG: bruk char, ikke uint8_t!
    static char buffer[16];
    static uint8_t idx = 0;
    static uint8_t reading_param = 0;
    static char param_type = 0;

    while (USART2_les((uint8_t*)&ch))  // ← cast til uint8_t* for lesing
    {
        // Debug: blink LED for hvert mottatt tegn


        if (reading_param)
        {
            if (ch >= '0' && ch <= '9')
            {
                if (idx < sizeof(buffer)-1)
                    buffer[idx++] = ch;
            }
            else if (ch == '\n' || ch == '\r')
            {
                buffer[idx] = '\0';
                int verdi = atoi(buffer);

                switch(param_type)
                {
                    case 'p':
                        aktuator_pid.Kp = (float)verdi/1000.0f;
                        USART2_skriv_streng((uint8_t*)"Kp\r\n");
                        USART2_send_hex(' ', (int32_t)(aktuator_pid.Kp*1000.0f), 4);

                        break;
                    case 'i':
                        aktuator_pid.Ki = (float)verdi / 1000.0f;
                        USART2_skriv_streng((uint8_t*)"Ki\r\n");
                        USART2_send_hex(' ', (int32_t)(aktuator_pid.Ki*1000.0f), 4);

                        break;
                    case 'd':
                        aktuator_pid.Kd = (float)verdi / 1000.0f;
                        USART2_skriv_streng((uint8_t*)"Kd\r\n");
                        USART2_send_hex(' ', (int32_t)(aktuator_pid.Kd*1000.0f), 4);

                        break;
                    case 'a':
                        aktuator_pid.settpunkt = (int16_t)verdi;
                        USART2_skriv_streng((uint8_t*)"Settpunkt\r\n");
                        USART2_send_hex(' ', (int16_t)(aktuator_pid.settpunkt), 2);

                        break;
                    case 't':
                        aktuator_pid.tau = (float)verdi / 1000.0f;
                        USART2_skriv_streng((uint8_t*)"tau\r\n");
                        USART2_send_hex(' ', (int32_t)(aktuator_pid.tau*1000.0f), 4);
                        break;

                }



                reading_param = 0;
                idx = 0;
                param_type = 0;
            }
        }
        else
        {
            switch(ch)  // ← nå er ch en char → 'p' matches perfekt!
            {
                case 'k':
                	USART3_skriv('k');
                	system_aktivt = 1;
                    legg_til_meldingshovud = 1;

                    USART2_skriv_streng((uint8_t*)"START\r\n");
                    break;

                case 's':
                	USART3_skriv('s');
                	system_aktivt = 0;
                    legg_til_meldingshale = 1;
                    USART2_skriv_streng((uint8_t*)"STOP\r\n");
                    break;

                case 'p':
                case 'i':
                case 'd':
                case 'a':
                case 't':
                    reading_param = 1;
                    idx = 0;
                    param_type = ch;
                    break;
            }
        }

    }
}

void USART2_send_hex(char prefix, int32_t verdi, uint8_t bytes)
{
	uint8_t nibbles = bytes * 2;
    uint8_t i;

    //Send prefix
    USART2_skriv(prefix);

    //Send nibbles (MSB først)
    for (i = 0; i < nibbles; i++)
    {
        int shift = (nibbles - 1 - i) * 4;             // 28,24,...,4,0 for 32-bit osv.
        uint8_t nibble = ((uint32_t)verdi >> shift) & 0x0F;
        USART2_skriv(hex2ascii_tabell[nibble]);       //Send som ascii
    }
}



void USART2_send_tid8_og_data16(uint8_t tid, int16_t loggeverdi)  {
	uint8_t tid0, tid1;
    int16_t data0, data1, data2, data3;

    tid0 = tid;
    tid1 = tid0 >> 4;

	USART2_skriv('T');
	USART2_skriv((uint8_t)(hex2ascii_tabell[(tid1 & 0x0F)]));   // Send MS Hex-siffer av ein tidsbyten
	USART2_skriv((uint8_t)(hex2ascii_tabell[(tid0 & 0x0F)])); // Send LS Hex-siffer av ein tidsbyten

	data0 = loggeverdi; //
	data1 = data0 >> 4; // Under skifting er det viktig at forteiknet blir med, difor int.
	data2 = data1 >> 4;
	data3 = data2 >> 4;

	USART2_skriv('L');            // L for loggedata
	USART2_skriv((uint8_t)(hex2ascii_tabell[(data3 & 0x000F)])); // Send MS Hex-siffer av 16-bitsdata
	USART2_skriv((uint8_t)(hex2ascii_tabell[(data2 & 0x000F)]));
	USART2_skriv((uint8_t)(hex2ascii_tabell[(data1 & 0x000F)]));
	USART2_skriv((uint8_t)(hex2ascii_tabell[(data0 & 0x000F)])); // Send LS Hex-siffer av dei 16 bitane
}

void USART2_send_tid8_og_data16x3(uint8_t tid, int16_t loggeverdi1, int16_t loggeverdi2, int16_t loggeverdi3)  {
	uint8_t tid0, tid1;
    int16_t data0, data1, data2, data3;

    tid0 = tid;
    tid1 = tid0 >> 4;

	USART2_skriv('T');
	USART2_skriv((uint8_t)(hex2ascii_tabell[(tid1 & 0x0F)]));   // Send MS Hex-siffer av ein tidsbyten
	USART2_skriv((uint8_t)(hex2ascii_tabell[(tid0 & 0x0F)])); // Send LS Hex-siffer av ein tidsbyten

	data0 = loggeverdi1; //
	data1 = data0 >> 4; // Under skifting er det viktig at forteiknet blir med, difor int.
	data2 = data1 >> 4;
	data3 = data2 >> 4;

	USART2_skriv('X');
	USART2_skriv((uint8_t)(hex2ascii_tabell[(data3 & 0x000F)])); // Send MS Hex-siffer av 16-bitsdata
	USART2_skriv((uint8_t)(hex2ascii_tabell[(data2 & 0x000F)]));
	USART2_skriv((uint8_t)(hex2ascii_tabell[(data1 & 0x000F)]));
	USART2_skriv((uint8_t)(hex2ascii_tabell[(data0 & 0x000F)])); // Send LS Hex-siffer av dei 16 bitane

	data0 = loggeverdi2; //
	data1 = data0 >> 4; // Under skifting er det viktig at forteiknet blir med, difor int.
	data2 = data1 >> 4;
	data3 = data2 >> 4;

	USART2_skriv('Y');
	USART2_skriv((uint8_t)(hex2ascii_tabell[(data3 & 0x000F)])); // Send MS Hex-siffer av 16-bitsdata
	USART2_skriv((uint8_t)(hex2ascii_tabell[(data2 & 0x000F)]));
	USART2_skriv((uint8_t)(hex2ascii_tabell[(data1 & 0x000F)]));
	USART2_skriv((uint8_t)(hex2ascii_tabell[(data0 & 0x000F)])); // Send LS Hex-siffer av dei 16 bitane

	data0 = loggeverdi3; //
	data1 = data0 >> 4; // Under skifting er det viktig at forteiknet blir med, difor int.
	data2 = data1 >> 4;
	data3 = data2 >> 4;

	USART2_skriv('Z');
	USART2_skriv((uint8_t)(hex2ascii_tabell[(data3 & 0x000F)])); // Send MS Hex-siffer av 16-bitsdata
	USART2_skriv((uint8_t)(hex2ascii_tabell[(data2 & 0x000F)]));
	USART2_skriv((uint8_t)(hex2ascii_tabell[(data1 & 0x000F)]));
	USART2_skriv((uint8_t)(hex2ascii_tabell[(data0 & 0x000F)])); // Send LS Hex-siffer av dei 16 bitane
}
void USART2_handtering(uint8_t loggedata)  {
    uint8_t data;
	USART2_skriv('D');
	USART2_skriv(':');
    data = loggedata >> 4;
	USART2_skriv((uint8_t)(hex2ascii_tabell[data]));   // Send MSB av maalinga
    data = loggedata & 0x0F;
	USART2_skriv((uint8_t)(hex2ascii_tabell[data])); // Send LSB
	USART2_skriv(' ');
}
void USART2_handtering1(void)  {


//	uint8_t ny_kommando;

	//	USART1_skriv(((maaling_teljar-1)/100)+0x30);    // Skriving av m�lingsnr. via UART/USB-modulen.


//	    ny_kommando = USART1_les();  // Sjekk om det er ny kommando fr� tastatur
//		if(ny_kommando != 0)  {      // I s� fall, oppdater kommando, ellers kommando som f�r
//			gyldig_trykk_av_USERbrytar = 1;
//		}


//		USART1_skriv(teikn++);    // Skriving av ASCII-koda teikn via UART/USB-modulen.
//		USART1_skriv('\r');       // retur til same felt.
//		if(teikn >'z')
//		   {
//		   teikn = 0x30;
//		   }

//	USART1_skriv(((maaling_teljar-1)/100)+0x30);    // Skriving av m�lingsnr. via UART/USB-modulen.


//	USART2_skriv((uint8_t)(loggedata >> 8));   // Send MSB av maalinga
//	USART2_skriv((uint8_t)(loggedata & 0xFF)); // Send LSB
//
//	USART1_skriv((uint8_t)(testmaaling >> 8));   // Send MSB av maalinga
//	USART1_skriv((uint8_t)(testmaaling & 0xFF)); // Send LSB


    testmaaling += 10;
    if(testmaaling >= 500) {
    	testmaaling = 0;
    }

    maaling_teljar++;
    if(maaling_teljar > 2000) {

    	maaling_teljar = 1;
//    	send_ny_tidsserie_1000 = 0;

    }

    USART1_skriv((uint8_t)(maaling_teljar >> 8));   // Send MSB av maalinga
    	USART1_skriv((uint8_t)(maaling_teljar & 0xFF)); // Send LSB

    	USART1_skriv((uint8_t)(testmaaling >> 8));   // Send MSB av maalinga
    	USART1_skriv((uint8_t)(testmaaling & 0xFF)); // Send LSB


        testmaaling += 10;
        if(testmaaling >= 500) {
        	testmaaling = 0;
        }

        maaling_teljar++;
        if(maaling_teljar > 2000) {

        	maaling_teljar = 1;
//       	send_ny_tidsserie_1000 = 0;

        }
//	USART1_skriv((uint8_t)(maaling_teljar >> 8));   // Send MSB av maalinga
//		USART1_skriv((uint8_t)(maaling_teljar & 0xFF)); // Send LSB
//
//		USART1_skriv((uint8_t)(testmaaling >> 8));   // Send MSB av maalinga
//		USART1_skriv((uint8_t)(testmaaling & 0xFF)); // Send LSB
//
//
//		testmaaling += 10;
//		if(testmaaling >= 500) {
//			testmaaling = 0;
//		}
//
//		maaling_teljar++;
//		if(maaling_teljar > 1000) {
//
//			maaling_teljar = 1;
//			send_ny_tidsserie_1000 = 0;
//
//		}
//	USART1_skriv((uint8_t)(maaling_teljar >> 8));   // Send MSB av maalinga
//		USART1_skriv((uint8_t)(maaling_teljar & 0xFF)); // Send LSB
//
//		USART1_skriv((uint8_t)(testmaaling >> 8));   // Send MSB av maalinga
//		USART1_skriv((uint8_t)(testmaaling & 0xFF)); // Send LSB
//
//
//		testmaaling += 10;
//		if(testmaaling >= 500) {
//			testmaaling = 0;
//		}
//
//		maaling_teljar++;
//		if(maaling_teljar > 1000) {
//
//			maaling_teljar = 1;
//			send_ny_tidsserie_1000 = 0;
//
//		}

}

void USART2_handtering2(uint16_t teljar)  {


	uint8_t data, i = 98;

    data = teljar;   // sender LSB av teljaren f�rst
	USART1_skriv(data);  // Skriving av data via UART/USB-modulen.

    data = (teljar>>8);   // sender s� MSB av teljaren
	USART1_skriv(data);  // Skriving av data via UART/USB-modulen.

	data = 0;
	while(i>0)  // S� skriving av 98 nullar.
		{
		USART1_skriv(data);
		i--;
	    }

}

void USART2_handtering3(void)  {

	uint8_t teikn = 0x30;
	uint8_t ny_kommando;
	uint8_t kommando ='2';


    //Send velkomst via UART/USB-modul
	USART1_skriv_streng((uint8_t *)"--\nVelkommen!\n\r");  // Ny linje og retur til linjestart etterp�.

	while(1)   // Endelaus l�kkje
		{
	    ny_kommando = USART1_les();  // Sjekk om det er ny kommando fr� tastatur
		if(ny_kommando != 0)  {      // I s� fall, oppdater kommando, ellers kommando som f�r
			kommando = ny_kommando;
		}

		switch(kommando) {           // Tolk kommando
		case 'g': //GPIO_snu_PC9();
			break; // Gr�n blink
		case 'b': //GPIO_snu_PC8();
			break; // Bl� blink
		}

//		venting_vhja_nedteljing(1000000);  // Det m� g� litt tid mellom blinka

		USART1_skriv(teikn++);    // Skriving av ASCII-koda teikn via UART/USB-modulen.
		USART1_skriv('\r');       // retur til same felt.
		if(teikn >'z')
		   {
		   teikn = 0x30;
		   }

    }
}
//---------------------------------------
// USART2 interrupt
//---------------------------------------
void USART2_EXTI26_IRQHandler(void)
{
    // --- Mottak ---
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        uint8_t mottatt = (uint8_t)USART_ReceiveData(USART2);
        uint16_t neste = (USART2_rx_index_in + 1) % USART2_RX_BUFFER_SIZE;
        if (neste != USART2_rx_index_out)
        {
        	USART2_rx_buffer[USART2_rx_index_in] = mottatt;
            USART2_rx_index_in = neste;
        }
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }

    // --- Sending ---
    if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
    {
        if (USART2_tx_index < USART2_tx_length)
        {
            USART_SendData(USART2, USART2_tx_buffer[USART2_tx_index++]);
        }
        else
        {
        	USART2_tx_index = 0;
        	USART2_tx_length = 0;
            USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
        }
        USART_ClearITPendingBit(USART2, USART_IT_TXE);
    }
}



//---------------------------------------
// USART3 oppstart (SPL)
//---------------------------------------
void USART3_oppstart(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // --- Klokker ---
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);    // GPIO D
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); // USART3

    // --- GPIO-konfig for PD8 (TX) og PD9 (RX) ---
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_7); // TX
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_7); // RX

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStruct);

    // --- USART3-konfig ---
    USART_StructInit(&USART_InitStruct);        // Setter defaultverdier
    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART3, &USART_InitStruct);

    // Aktiver USART3 RX interrupt
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    // --- NVIC-konfig ---
    NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // --- Aktiver USART3 ---
    USART_Cmd(USART3, ENABLE);
}



//---------------------------------------
// Send data (legger i buffer, start interrupt)
//---------------------------------------
void USART3_skriv(uint8_t data)
{
    USART3_tx_buffer[USART3_tx_length++] = data;
    USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
}


//---------------------------------------
// Les data fra ringbuffer
//---------------------------------------
uint8_t USART3_les(uint8_t* data)
{
    if (USART3_rx_index_in != USART3_rx_index_out) {
        *data = USART3_rx_buffer[USART3_rx_index_out];
        USART3_rx_index_out = (USART3_rx_index_out + 1) % USART3_RX_BUFFER_SIZE;
        return 1;
    }
    return 0;
}

//---------------------------------------
// Send tid + 8-bit tid + 16-bit data (hex ASCII)
//---------------------------------------
void USART3_send_tid8_og_data16(uint8_t tid, int16_t loggeverdi)
{
    uint8_t t1 = tid >> 4;
    uint8_t t0 = tid & 0x0F;

    int16_t d = loggeverdi;
    uint8_t d3 = (d >> 12) & 0x0F;
    uint8_t d2 = (d >> 8)  & 0x0F;
    uint8_t d1 = (d >> 4)  & 0x0F;
    uint8_t d0 = d & 0x0F;

    USART3_skriv('T');
    USART3_skriv(hex2ascii_tabell[t1]);
    USART3_skriv(hex2ascii_tabell[t0]);
    USART3_skriv('L');
    USART3_skriv(hex2ascii_tabell[d3]);
    USART3_skriv(hex2ascii_tabell[d2]);
    USART3_skriv(hex2ascii_tabell[d1]);
    USART3_skriv(hex2ascii_tabell[d0]);
}



//---------------------------------------
// Send tid + tre 16-bit data
//---------------------------------------
void USART3_send_tid8_og_data16x4(uint8_t tid, int16_t loggeverdi1, int16_t loggeverdi2, int16_t loggeverdi3, int16_t loggeverdi4)
{
    uint8_t tid0 = tid;
    uint8_t tid1 = tid0 >> 4;
    int16_t data0, data1, data2, data3;

    // --- Tid: 'T' + to hex-sifre ---
    USART3_skriv('T');
    USART3_skriv(hex2ascii_tabell[tid1 & 0x0F]);
    USART3_skriv(hex2ascii_tabell[tid0 & 0x0F]);

    // --- Verdi 1: 'X' + fire hex-sifre ---
    data0 = loggeverdi1;
    data1 = data0 >> 4;
    data2 = data1 >> 4;
    data3 = data2 >> 4;
    USART3_skriv('X');
    USART3_skriv(hex2ascii_tabell[data3 & 0x0F]);
    USART3_skriv(hex2ascii_tabell[data2 & 0x0F]);
    USART3_skriv(hex2ascii_tabell[data1 & 0x0F]);
    USART3_skriv(hex2ascii_tabell[data0 & 0x0F]);

    // --- Verdi 2: 'Y' + fire hex-sifre ---
    data0 = loggeverdi2;
    data1 = data0 >> 4;
    data2 = data1 >> 4;
    data3 = data2 >> 4;
    USART3_skriv('Y');
    USART3_skriv(hex2ascii_tabell[data3 & 0x0F]);
    USART3_skriv(hex2ascii_tabell[data2 & 0x0F]);
    USART3_skriv(hex2ascii_tabell[data1 & 0x0F]);
    USART3_skriv(hex2ascii_tabell[data0 & 0x0F]);

    // --- Verdi 3: 'Z' + fire hex-sifre ---
    data0 = loggeverdi3;
    data1 = data0 >> 4;
    data2 = data1 >> 4;
    data3 = data2 >> 4;
    USART3_skriv('Z');
    USART3_skriv(hex2ascii_tabell[data3 & 0x0F]);
    USART3_skriv(hex2ascii_tabell[data2 & 0x0F]);
    USART3_skriv(hex2ascii_tabell[data1 & 0x0F]);
    USART3_skriv(hex2ascii_tabell[data0 & 0x0F]);

    // --- Verdi 4: 'AVSTAND' + fire hex-sifre ---
    data0 = loggeverdi4;
    data1 = data0 >> 4;
    data2 = data1 >> 4;
    data3 = data2 >> 4;
    USART3_skriv('A');  // Bruker 'A' for avstandsverdi
    USART3_skriv(hex2ascii_tabell[data3 & 0x0F]);
    USART3_skriv(hex2ascii_tabell[data2 & 0x0F]);
    USART3_skriv(hex2ascii_tabell[data1 & 0x0F]);
    USART3_skriv(hex2ascii_tabell[data0 & 0x0F]);
}

void USART3_skriv_blocking(uint8_t data)
{
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
    USART_SendData(USART3, data);
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
}
uint8_t USART3_parse_pakke(USART3_Pakke_t* pakke)
{
    static uint8_t idx = 0;
    static char buffer[23];
    uint8_t ch;

    pakke->valid = 0;

    while (USART3_les(&ch)) {
        // Sjekk forventet bokstav
        if (idx == 0 && ch != 'T') continue;
        if (idx == 3 && ch != 'X') { idx = 0; continue; }
        if (idx == 8 && ch != 'Y') { idx = 0; continue; }
        if (idx == 13 && ch != 'Z') { idx = 0; continue; }
        if (idx == 18 && ch != 'A') { idx = 0; continue; }

        buffer[idx++] = ch;

        if (idx == 23) {
            // === konverterer hex til ascii direkte
            pakke->samplenr =
                ((ch = buffer[1]) >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0)) << 4 |
                ((ch = buffer[2]) >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0));

            pakke->ax = 
                ((ch = buffer[4])  >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0)) << 12 |
                ((ch = buffer[5])  >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0)) << 8  |
                ((ch = buffer[6])  >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0)) << 4  |
                ((ch = buffer[7])  >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0));

            pakke->ay = 
                ((ch = buffer[9])  >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0)) << 12 |
                ((ch = buffer[10]) >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0)) << 8  |
                ((ch = buffer[11]) >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0)) << 4  |
                ((ch = buffer[12]) >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0));

            pakke->az = 
                ((ch = buffer[14]) >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0)) << 12 |
                ((ch = buffer[15]) >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0)) << 8  |
                ((ch = buffer[16]) >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0)) << 4  |
                ((ch = buffer[17]) >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0));

            pakke->avstand = 
                ((ch = buffer[19]) >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0)) << 12 |
                ((ch = buffer[20]) >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0)) << 8  |
                ((ch = buffer[21]) >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0)) << 4  |
                ((ch = buffer[22]) >= '0' && ch <= '9' ? ch - '0' : (ch >= 'A' && ch <= 'F' ? ch - 'A' + 10 : 0));

            pakke->valid = 1;
            idx = 0;
            return 1;
        }
    }
    return 0;
}

//---------------------------------------
// USART3 interrupt
//---------------------------------------
void USART3_EXTI28_IRQHandler(void)
{
    // --- Mottak ---
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        uint8_t mottatt = (uint8_t)USART_ReceiveData(USART3);
        uint16_t neste = (USART3_rx_index_in + 1) % USART3_RX_BUFFER_SIZE;
        if (neste != USART3_rx_index_out)
        {
        	USART3_rx_buffer[USART3_rx_index_in] = mottatt;
            USART3_rx_index_in = neste;
        }
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }

    // --- Sending ---
    if (USART_GetITStatus(USART3, USART_IT_TXE) != RESET)
    {
        if (USART3_tx_index < USART3_tx_length)
        {
            USART_SendData(USART3, USART3_tx_buffer[USART3_tx_index++]);
        }
        else
        {
        	USART3_tx_index = 0;
        	USART3_tx_length = 0;
            USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
        }
        USART_ClearITPendingBit(USART3, USART_IT_TXE);
    }
}



