
// Hovedprogram
// Fil: main.c

//---------------------------------------
// Inklusjonar og definisjonar
//---------------------------------------
#include <cmsis_boot/stm32f30x.h>
#include <dekl_globale_variablar.h>
#include <cmsis_lib/stm32f30x_gpio.h>
#include "stm32f3_discovery/stm32f3_discovery_lsm303dlhc.h"
#include "pid.h"
#include <cmsis_lib/stm32f30x_adc.h>

#include <stdbool.h>

//---------------------------------------
// Variabler
//---------------------------------------
int16_t a;
uint8_t USART2_kommando;
uint8_t USART3_kommando;
//---------------------------------------
// Funksjonsprototypar
//---------------------------------------

void maskinvare_init(void);

void GPIO_sjekk_USER_brytar(void);
void GPIO_sett_kompassmoenster(int8_t verdi);
void GPIO_ventesymbol_step(void);

void USART2_skriv(uint8_t data);
uint8_t USART2_les(uint8_t* data);
void USART2_send_hex(char prefix, int32_t verdi, uint8_t bytes);

uint8_t USART3_les(uint8_t* data);
void USART3_skriv(uint8_t ch);
void USART3_send_tid8_og_data16x4(uint8_t tid, int16_t loggeverdi1, int16_t loggeverdi2, int16_t loggeverdi3, int16_t loggeverdi4);
uint8_t USART3_parse_pakke(USART3_Pakke_t* pakke);



//---------------------------------------
// Funksjonsdeklarasjonar
//---------------------------------------



int main(void)  {

	float Kp=0.004f;
	float Ki=0.3f;
	float Kd=0.08f;
	float Ts=0.001f;
	float tau=0.05f;
	int16_t settpunkt=350;
	aktuator_pid = PID_init(Kp, Ki, Kd,Ts,tau,settpunkt);

    maskinvare_init();


	while(1) {
		// ===================================================================
        // SENSORNODE (unit_id == 1)
        // ===================================================================
		if (unit_id == 1) {

			if (USART3_les(&USART3_kommando)) {

			    if (USART3_kommando == 'k') {
			    	system_aktivt = 1;
			    }

			    if (USART3_kommando == 's') {
			    	system_aktivt = 0;
			    }
			}

			if(system_aktivt)  {
				if(ny_maaling){

			        LSM303DLHC_Read(ACC_I2C_ADDRESS, LSM303DLHC_OUT_X_L_A, buffer, 6); // OUT_X_L_A ligg på lågaste adresse
			        																   // i XYZ-registerblokka i kretsen
				 // Sett saman akselerometerdata
	                a   = (buffer[1] << 8) | buffer[0]; //Buffer 1 er MSByte i flg. databladet.
			        a_x = ((int16_t)(a)) >> 4;          //Gjer om til int der bare dei 12 teljande bitane er med
					a   = (buffer[3] << 8) | buffer[2];	//Dette gir omr�det -2048 til 2047. Oppl�ysinga er
					a_y = ((int16_t)(a)) >> 4;			// 1mg pr. LSb ved +/-2g omr�de i flg. databladet.
					a   = (buffer[5] << 8) | buffer[4];	// Verdien 1000 gir d� 1 g.
					a_z = ((int16_t)(a)) >> 4;

				 // Filtrer m�lingane
					a_xf_k = (a1*a_xf_k_1 + b1*a_x)/100; //Nedsamplingsfilter, sj� deklarasjonsfila
					a_xf_k_1 = a_xf_k;

					a_yf_k = (a1*a_yf_k_1 + b1*a_y)/100; //Nedsamplingsfilter, sj� deklarasjonsfila
					a_yf_k_1 = a_yf_k;

					a_zf_k = (a1*a_zf_k_1 + b1*a_z)/100; //Nedsamplingsfilter, sj� deklarasjonsfila
					a_zf_k_1 = a_zf_k;

					//Hent ADC-verdi
				    ADC_StartConversion(ADC3);
					while (!ADC_GetFlagStatus(ADC3, ADC_FLAG_RDY)) { __NOP(); }
					AD_RESULT = ADC_GetConversionValue(ADC3);

					//Send målinger til styrenode.
					USART3_send_tid8_og_data16x4(samplenr, a_xf_k, a_yf_k, a_zf_k, AD_RESULT);

					//Visualiser avstandsmåling på kompassdioder
					uint8_t num_leds = (AD_RESULT * 8) / 4096;
					uint8_t diode_moenster = (1 << num_leds) - 1;
					GPIO_sett_kompassmoenster(diode_moenster);

					//Ny samplenr
					samplenr++;
					ny_maaling=0;
				}
			}
			else if (!system_aktivt) {
				if(oppdater_diodar){
					GPIO_ventesymbol_step();
					oppdater_diodar=0;
				}
			}

		}
		// ===================================================================
        // STYRENODE (unit_id == 2)
        // ===================================================================
		else if (unit_id == 2) {
			USART2_parse_kommando();
			if(gyldig_trykk_av_USER_brytar) { //Er brytaren trykt ned sidan sist?
				system_aktivt ^= 1;   // Toggle
			    if (system_aktivt)
			    {
			        USART3_skriv('k');                    // Send "kjør" til sensornoden
			        legg_til_meldingshovud = 1;           // Start ny meldingspakke til PC
			        USART2_skriv_streng((uint8_t*)"START\r\n");
			    }
			    else
			    {
			        USART3_skriv('s');                    // Send "stopp" til sensornoden
			        legg_til_meldingshale = 1;            // Avslutt meldingspakke til PC
			        USART2_skriv_streng((uint8_t*)"STOP\r\n");

			    }
			    gyldig_trykk_av_USER_brytar=0;
			}


			if(system_aktivt)  { //Burde endre variabelnavn til system_aktivt?
				if (USART3_parse_pakke(&pakke)){ //Pakker ut målinger fra sensornoden
					if(pakke.valid){	//Bruker kun måling der all data er mottat

						float avstand_mv=(float)pakke.avstand*3000/4096; //Regn om digital verdi til mv
						float avstand_mv_filt = avstand_mv_filt + 0.5f * (avstand_mv - avstand_mv_filt); //Filter for å fjerne støy
						float avstand_mm = avstand_mv2mm(avstand_mv_filt); //Utfør lineær interpolasjon for å finne estimert avstand

						float u = PID_Beregn(&aktuator_pid, avstand_mm); //Beregn pådraget u
						float u_prosent = u * 100.0f;	//Gjør om til prosent

						Aktuator_sett_paadrag((int16_t)u_prosent);	//Sett pådrag med retning basert på u


						//meldinsstart til pc
						if (legg_til_meldingshovud) {
							USART2_skriv(STX);
							legg_til_meldingshovud = 0;
						}


						if(send_maaling){	//send hver 10. maaling til pc
							//Kan enkelt utvide sending av data
							USART2_send_hex('T', (int16_t)(pakke.samplenr), 1); //samplenr

							USART2_send_hex('x', (int16_t)(pakke.ax), 2); //akselerasjon x
							USART2_send_hex('y', (int16_t)(pakke.ay), 2); //akselerasjon y
							USART2_send_hex('z', (int16_t)(pakke.az), 2); //akselerasjon z

							USART2_send_hex('p', (int32_t)(aktuator_pid.u_p_lagret*1000.0f), 4); //pådrag up
							USART2_send_hex('i', (int32_t)(aktuator_pid.u_i_lagret*1000.0f), 4); //pådrag ui
							USART2_send_hex('d', (int32_t)(aktuator_pid.u_d_lagret*1000.0f), 4); //pådrag ud


							USART2_send_hex('s', (uint16_t)aktuator_pid.settpunkt, 2);	//settpunkt
							USART2_send_hex('a', (uint32_t)(avstand_mm*1000.0f), 4);	//avstandsmåling

							USART2_send_hex('t', (int32_t)(aktuator_pid.tau*1000.0f), 4);	//tau

							send_maaling = 0; //reset flagg når måling er sendt
						}

						//Visualiser avstand på kompassdioder
						uint8_t num_leds = (pakke.avstand * 8) / 4096;
						uint8_t diode_moenster = (1 << num_leds) - 1;
						GPIO_sett_kompassmoenster(diode_moenster);

					}
				}
			}
			/
			else if (!system_aktivt) {
				TIM_Cmd(TIM3, DISABLE); //Stopp pådrag til aktuator

				if(legg_til_meldingshale){
				USART2_skriv(ETX); //meldinsslutt til pc
				legg_til_meldingshale = 0;
				}

				if(oppdater_diodar){
					GPIO_ventesymbol_step();	//ventesymbol når regulering ikke er aktiv
					oppdater_diodar=0;
				}


			}


		}



		// ===================================================================
        // TEST (unit_id == 3)
        // ===================================================================

		else if (unit_id == 3) {

		}
	}
}




























