/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
// Ryabinin (c) 2012
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
#include <stm32f10x.h>
#include <system_stm32f10x.h>

#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>

#include "timer_mr.h"
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

static volatile TimerMr_t Time;

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

void TimerMrInit(void)
{
	Time = 0;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	static const TIM_TimeBaseInitTypeDef init = { //
			.TIM_Prescaler = 0, //
			.TIM_CounterMode = TIM_CounterMode_Down, //
			.TIM_Period = 2400-1, //
			.TIM_ClockDivision = TIM_CKD_DIV1, };

	TIM_TimeBaseInit(TIM3, (TIM_TimeBaseInitTypeDef*) &init);

	static const NVIC_InitTypeDef nvic = {
			.NVIC_IRQChannel = TIM3_IRQn,
			.NVIC_IRQChannelPreemptionPriority = 0,
			.NVIC_IRQChannelSubPriority = 1,
			.NVIC_IRQChannelCmd = ENABLE,
	};

	NVIC_Init((NVIC_InitTypeDef*)&nvic);

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	TIM_Cmd(TIM3, ENABLE);
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

TimerMr_t TimerMrGet(void)
{
	TimerMr_t val;

	TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);

	val = Time;

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	return val;
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

void TimerMrTick(void)
{
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
	    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

	    Time++;
	}
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
