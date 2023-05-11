/*
 * ADC_H.h
 *
 * Created: 5/7/2023 10:49:24 PM
 *  Author: Dinh Viet Hoa
 */ 


#ifndef ADC_H_H_
#define ADC_H_H_

#define F_CPU 16000000UL
#include <avr/io.h>


void adc_init();
uint16_t adc_read(uint8_t channel);



#endif /* ADC_H_H_ */