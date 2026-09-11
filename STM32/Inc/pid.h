
#include <stdint.h>
#ifndef PID_H
#define PID_H

typedef struct {
    // Regulatorparametre
    float Kp;      // Proporsjonalforsterkning
    float Ki;      // Integrasjonsforsterkning
    float Kd;      // Derivasjonsforsterkning
    float Ts;      // Samplingsperiode [s]
    float tau;     // Tidskonstant for lavpassfilter på D-ledd [s]

    // Interne variabler (tilstandsvariabler)
    float f_måling_lagret;
    float e_lagret;			// e(k-1)
    float u_p_lagret;
    float u_i_lagret;      // u_I(k-1)
    float u_d_lagret;      // u_D(k-1)

    //anti windup
    float u_i_min;
    float u_i_max;

    // Utgangsbegrensninger
    float u_min;
    float u_max;

    int16_t settpunkt;
} PID;

PID PID_init(float Kp, float Ki, float Kd, float Ts, float tau,int16_t settpunkt);
float PID_Beregn(PID* pid, float måling);
void Aktuator_sett_paadrag(int16_t u_prosent);
float avstand_mv2mm(float adc_mv);

#endif
