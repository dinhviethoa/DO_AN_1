/*
 * Timer0.h
 *
 * Created: 3/10/2023 2:26:50 PM
 *  Author: Dinh Viet Hoa
 */ 


#ifndef TIMER0_H_
#define TIMER0_H_

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

void timer1_init();
uint32_t second();
bool delay(uint32_t start_time, uint32_t delay_time);


#endif /* TIMER0_H_ */