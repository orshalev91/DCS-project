/*
 * adc.h
 *
 *  Created on: Nov 25, 2014
 *      Author: Manuel Alejandro
 */

#ifndef ADC_H_
#define ADC_H_

#include "derivative.h"

/* Prototypes */
void adc_init(void);
int adc_cal(void);
void InitDAC();
unsigned short adc_read(unsigned char ch);
void dma_adc_init(void);
void dma_dac_init (void);

#endif /* ADC_H_ */
