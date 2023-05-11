#include "Timer1.h"

volatile static uint32_t count = 0;

ISR(TIMER1_COMPA_vect)
{
	cli();
	count++;
	sei();
}

uint32_t second()
{
	return count;
}
bool delay(uint32_t start_time, uint32_t delay_time)
{
	return ((second() - start_time) > delay_time);
}
void timer1_init()
{
	OCR1A = 62499;
	TCCR1B |= (1 << WGM12) | (1 << CS12);
	TIMSK1 |= (1 << OCIE1A);
}


