

#include "pid.h"
#include <stdbool.h>
#include <cmsis_lib/stm32f30x_gpio.h>

void TIM3_sett_frekvens(uint32_t frekvens);
#define N_PUNKTER 19
const float avstand_tabell_mm[N_PUNKTER] = {0, 50  , 100 , 150 , 200 , 250 , 300 , 350 , 400 , 450 , 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000};
const float adc_tabell_mv[N_PUNKTER] = 	   {0, 1488, 2272, 2419, 2273, 1950, 1663, 1359, 1166, 1030, 914, 891, 814, 738, 573, 539, 442, 327, 258, 176, 129};
float avstand_mv2mm(float adc_mv);

//PID struct
PID PID_init(float Kp, float Ki, float Kd, float Ts, float tau,int16_t settpunkt) {
    PID pid = {
        .Kp = Kp,
        .Ki = Ki,
        .Kd = Kd,
        .Ts = Ts,
        .tau = tau,

		.f_måling_lagret=0.0f,
        .e_lagret = 0.0f,
		.u_p_lagret = 0.0f,
        .u_i_lagret = 0.0f,
        .u_d_lagret = 0.0f,

		.u_i_min = -0.3f,
        .u_i_max = 0.3f,

        .u_min = -1.0f,
        .u_max = 1.0f,

		.settpunkt=settpunkt
    };
    return pid;}


float PID_Beregn(PID* pid, float måling)
{
	float deadsone=10.0f; // dødsone

    float e = pid->settpunkt - måling; //Beregn error


    if (fabsf(e) < deadsone){
    	e = 0.0f;
    }


    float u_p = pid->Kp * e; //proposjonalt pådrag

    float u_i = 0.0f; //integral pådrag
    // Integrasjon (trapes). Beregn kandidat u_i først
    if (pid->Ki != 0.0f) {
    	u_i = pid->u_i_lagret + 0.5f * pid->Ki * pid->Ts * (e + pid->e_lagret);
    }

    //anti windup
    if(u_i>pid->u_i_max) u_i=pid->u_i_max;
    if(u_i<pid->u_i_min) u_i=pid->u_i_min;




    // Beregn derivat (med filter). øk tau hvis støy
    float alfa=0.7;
    float f_måling=pid->f_måling_lagret+(1-alfa)*måling;
    float u_d =-pid->Kd*pid->tau*(f_måling-pid->f_måling_lagret);

    // Summer pådrag
    float u = u_p + u_i + u_d;


    // Clipping utgang hvis u er over grensen
    if (u > pid->u_max) u = pid->u_max;
    if (u < pid->u_min) u = pid->u_min;

    // Oppdater lagrede verdier
    pid->f_måling_lagret=f_måling;
    pid->e_lagret = e;
    pid->u_p_lagret = u_p;
    pid->u_i_lagret = u_i;
    pid->u_d_lagret = u_d;

    return u; //returner u
}



void Aktuator_sett_paadrag(int16_t u_prosent) {

	uint16_t abs_prosent;
	uint16_t laveste_prosent=1;

	//Clipping av prosent
    if (u_prosent > 100) u_prosent = 100;
    if (u_prosent < -100) u_prosent = -100;


    //Dersom pådrag er under 1 prosent skrus timeren av
    if (u_prosent > -laveste_prosent && u_prosent < laveste_prosent) {  // Juster laveste_prosent etter behov
        GPIO_ResetBits(GPIOB, GPIO_Pin_5);  // Begge pinner AV
        TIM_Cmd(TIM3, DISABLE);           // Sett frekvens til 0
        return;
    }
    else{
		if (u_prosent > 0) {
			GPIO_SetBits(GPIOB, GPIO_Pin_5); //sett retningssignal
			abs_prosent = u_prosent;
		} else {
			GPIO_ResetBits(GPIOB, GPIO_Pin_5);//sett retningssignal motsatt vei
			abs_prosent = -u_prosent;
		}

		const uint32_t min_frekvens = 5000UL; //Minste tillate hastighet til aktuatoren
		const uint32_t max_frekvens = 250000UL;	//Største tillate hastighet til aktuatoren

		//omregn fra prosent til frekvens
		uint32_t frekvens = min_frekvens + ((uint32_t)abs_prosent * (max_frekvens- min_frekvens)) / 100UL;
		//sett pådrag til aktuator
		TIM3_sett_frekvens(frekvens);
    }
}



float avstand_mv2mm(float adc_mv) {


    if(adc_mv >= 2296) return 200; // setter minste avstand til 200 mm. Unngår å gå for nærme sensor.

    // Interpoler mellom punktene i tabellen
    for(int i = 0; i < N_PUNKTER - 1; i++) {
        if(adc_mv <= adc_tabell_mv[i] && adc_mv >= adc_tabell_mv[i+1]) {
            // lineær interpolasjon
            float slope = (float)(avstand_tabell_mm[i+1] - avstand_tabell_mm[i]) /
                          (float)(adc_tabell_mv[i+1] - adc_tabell_mv[i]);
            return avstand_tabell_mm[i] + slope * (adc_mv - adc_tabell_mv[i]);
        }
    }

    // Hvis adc_mv er utenfor tabellen, returner maksimalt målte punkt
    return 1000;
}
