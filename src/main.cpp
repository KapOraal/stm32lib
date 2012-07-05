/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
//      Copyright 2012 Ryabinin Slava
//      ryabinin.v.v@gmail.com
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*
 This file is part of stm32lib.

 stm32lib is free software: you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 stm32lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with stm32lib.  If not, see <http://www.gnu.org/licenses/>.
 */
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>

#include "STM32vldiscovery.h"

#include "timer/timer.h"
#include "timer_hr/timer_hr.h"
#include "timer_mr/timer_mr.h"

#include "uart-dma/uart-dma.h"
#include "uart-irq/uart-irq.h"
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

#define SIZE_BUF			         		(64)
static u8 Rx[SIZE_BUF];
static u8 Tx[SIZE_BUF];

static u8 Rx2[SIZE_BUF];
static u8 Tx2[SIZE_BUF];

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

static inline u32 TimerGetXXX(void)
{
	return (u32) TimerMrGet();
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

#define UART_DMA_PORT	          				(2)
#define UART_IRQ_PORT	          				(1)

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

static BOOL8 SerialInited;

static void SerialInit(void)
{
	TimerMrInit();

	static const UartConst_t UartDmaConst = { 10, //
			{ Rx, SIZE_BUF, }, //
			{ Tx, SIZE_BUF, }, //
			{ TimerGetXXX, TIMER_MR_FREQ, }, //
			0, //
			};

	static UartDma_t UartDma = { (UartConst_t*) &UartDmaConst, //
			115200, //
			};

	UartDmaInit(&UartDma, UART_DMA_PORT);

	//------------------------------------------

	static const UartConst_t UartIrqConst = { 10, //
			{ Rx2, SIZE_BUF, }, //
			{ Tx2, SIZE_BUF, }, //
			{ TimerGetXXX, TIMER_MR_FREQ, }, //
			0, //
			};

	static UartIrq_t UartIrq = { (UartConst_t*) &UartIrqConst, //
			115200, //
			};

	UartIrqInit(&UartIrq, UART_IRQ_PORT);

	SerialInited = TRUE;
}

static void SerialPoll(void)
{
	lu16 len;

	len = UartDmaRxLen(UART_DMA_PORT);

	if (len)
	{
		if (UartDmaTxEmpty(UART_DMA_PORT))
		{
			Tx[0] = '<';
			memcpy(&Tx[1], Rx, len);
			Tx[len + 1] = '>';
			UartDmaTx(UART_DMA_PORT, len + 2);
		}

		UartDmaRxClear(UART_DMA_PORT);
	}

	//------------------------------------------

	len = UartIrqRxLen(UART_IRQ_PORT);

	if (len)
	{
		if (UartIrqTxEmpty(UART_IRQ_PORT))
		{
			Tx2[0] = '[';
			memcpy(&Tx2[1], Rx2, len);
			Tx2[len + 1] = ']';
			UartIrqTx(UART_IRQ_PORT, len + 2);
		}

		UartIrqRxClear(UART_IRQ_PORT);
	}
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

#ifdef  USE_FULL_ASSERT

static void ToDecString(lu16 val, char * s)
{
	lu8 digit;

	static const u32 del[] = { (u32) 1e6, //
			(u32) 1e5, //
			(u32) 1e4, //
			(u32) 1e3, //
			(u32) 1e2, //
			(u32) 1e1, //
			(u32) 1e0, //
			};

#define countof( array ) ( sizeof( array )/sizeof( array[0] ) )

	lu8 z = 0;

	for (lu8 q = 0; q < countof(del); q++)
	{
		digit = val / del[q];
		if (digit != 0)
		{
			s[z] = digit + '0';
			z++;
		}
		val %= del[q];
	}

	s[z] = 0;
}

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(const char* file, u32 line, const char * expr)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	if (!SerialInited)
	{
		return;
	}

	while (!UartDmaTxEmpty(UART_DMA_PORT))
	{
	}

	Tx[0] = 0;

	strcat((char *) Tx, "E>");
	strcat((char *) Tx, file);
	strcat((char *) Tx, ",");
	ToDecString(line, (char *) &Tx[strlen((char *) Tx)]);
	strcat((char *) Tx, ",");
	strcat((char *) Tx, expr);
	strcat((char *) Tx, "\r\n");

	UartDmaTx(UART_DMA_PORT, strlen((char *) Tx));
}
#endif

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

#define LED_GREEN						LED3
#define LED_BLUE						LED4

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

int main(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | //
			RCC_APB2Periph_GPIOB | //
			RCC_APB2Periph_GPIOC | //
			RCC_APB2Periph_GPIOD | //
			RCC_APB2Periph_GPIOE, ENABLE);

	{
		GPIO_InitTypeDef GPIO_InitStructure;

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		GPIO_Init(GPIOE, &GPIO_InitStructure);
	}

	SerialInit();

	STM32vldiscovery_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);

	STM32vldiscovery_LEDInit(LED_GREEN);
	STM32vldiscovery_LEDInit(LED_BLUE);

	TimerInit();
	TimerHrInit();

	while (1)
	{
		SerialPoll();
	}

	exit(0);
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

