/*
 * timer.c
 *
 *  Created on: Oct 5, 2023
 *      Author: schulman
 */

 #include "timer.h"


 void timer_init(TIM_TypeDef* timer)
 {
   // TODO implement this
   //enable the clock:
 
 RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
   //1. stop timer and reset
 //  RCC_APB1ENR1 &= ~(1 << 0);
 
   timer->CR1 &= ~TIM_CR1_CEN;
   RCC->APB1RSTR1 |= RCC_APB1RSTR1_TIM2RST;
   //set reset bit to 0
   RCC->APB1RSTR1 &= ~RCC_APB1RSTR1_TIM2RST;
 
   timer->CNT = 0;
 
 //  RCC->CSR
 //  timer_reset(timer);
 
   //2. set timer to auto-reload when reach max
 
  //  timer_set_ms(timer, 49);

   timer_set_ms(timer, 9999);  //interrupt every 10 seconds
 
   //3. Enable the timer’s interrupt both internally and in the interrupt controller (NVIC)
   NVIC_EnableIRQ(TIM2_IRQn);
   NVIC_SetPriority(TIM2_IRQn, 0);
   timer->DIER |= TIM_DIER_UIE;
 
   //4. slow down clock speed to 1kHz
   timer->PSC = 3999;
 
 
   //5. start the timer
   timer->CR1 |= TIM_CR1_CEN;
 }
 
 void timer_reset(TIM_TypeDef* timer)
 {
   // TODO implement this
   timer->CNT = 0;
 }
 
 void timer_set_ms(TIM_TypeDef* timer, uint16_t period_ms)
 {
   // TODO implement this
   timer->ARR = period_ms;
 }
 