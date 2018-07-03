#ifndef TIMER_H
#define TIMER_H

void ConfigTimer0(unsigned int intervalo,unsigned char divisor);

void StartTimer0(void);

void StopTimer0(void);

#pragma vector=TIMER0_A0_VECTOR
__interrupt void timer (void);

#endif
