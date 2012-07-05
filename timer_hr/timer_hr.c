/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
// Ryabinin (c) 2012
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
#include <stm32f10x.h>
#include <system_stm32f10x.h>

#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>

#include "timer_hr.h"
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

static volatile TimerHr_t Time;

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

/// Проверка флага переполнения
static BOOLX TimerIsOvf(void);
/// Тело обработчика прерывания
static void TimerIsr(void);

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

void TimerHrInit(void)
{
	Time = 0;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	static const TIM_TimeBaseInitTypeDef init = { //
			.TIM_Prescaler = 0, //
			.TIM_CounterMode = TIM_CounterMode_Up, //
			.TIM_Period = 0xFFFF, //
			.TIM_ClockDivision = TIM_CKD_DIV1, };

	TIM_TimeBaseInit(TIM2, (TIM_TimeBaseInitTypeDef*) &init);

	static const NVIC_InitTypeDef nvic = {
			.NVIC_IRQChannel = TIM2_IRQn,
			.NVIC_IRQChannelPreemptionPriority = 0,
			.NVIC_IRQChannelSubPriority = 1,
			.NVIC_IRQChannelCmd = ENABLE,
	};

	NVIC_Init((NVIC_InitTypeDef*)&nvic);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	TIM_Cmd(TIM2, ENABLE);
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

volatile u32 Counter[3];

TimerHr_t TimerHrGet(void)
{
	lu16 lo;
	TimerHr_t hi;
	TimerHr_t rez;
	BOOLX ovf[2];

	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);

	ovf[0] = TimerIsOvf();

	lo = TIM_GetCounter(TIM2);

	ovf[1] = TimerIsOvf();

	if(!ovf[0] && !ovf[1])
	{ // не было
		hi = Time;

		Counter[0]++;
	}
	else if(!ovf[0] && ovf[1])
	{ // появилось между 0 и 1
		if( lo < 0x8000 )
		{
			hi = Time + 1;
		}
		else
		{
			hi = Time;
		}

		TimerIsr();

		Counter[1]++;
	}
	else if(ovf[0] && ovf[1])
	{ // появилось между запрещением прерывания и 0
		hi = Time + 1;

		TimerIsr();

		Counter[2]++;
	}

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	rez = hi;
	rez <<= 16;
	rez |= lo;

	return rez;
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

static BOOLX TimerIsOvf(void)
{
	return (TIM2->SR & TIM_IT_Update);
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

static void TimerIsr(void)
{
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    Time++;
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

void TimerHrTick(void)
{
	if( TimerIsOvf() )
	{
		TimerIsr();
	}
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
