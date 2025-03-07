#ifndef _DELAY_H_
#define _DELAY_H_

#include "stm32f10x.h" 




void Delay_us(void);
void Delay_nus(uint32_t n);
void Delay_ms(void);
void Delay_nms(uint32_t n);

#endif

