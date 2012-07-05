/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
// Ryabinin (c) 2012
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
#include <stm32f10x_usart.h>
#include "uart-common/uart-common.h"
#include "uart-irq.h"
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

typedef struct
{
	USART_TypeDef * Regs; ///< Адрес блока регистров уарта
} Const_t;

static const Const_t Sett[UARTS_IRQ_NUM] = { { USART1, }, //
		{ USART2, }, //
		{ USART3, }, //
		};

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

/// Настройки пользователя
static UartIrq_t * UartIrq[UARTS_IRQ_NUM];

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

static void NVIC_Configuration(lu8 num);

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

void UartIrqInit(UartIrq_t * uart, lu8 num)
{
	assert_param(num < UARTS_IRQ_NUM);
	assert_param(uart != NULL);

	USART_TypeDef * usart = Sett[num].Regs;
	UartIrq[num] = uart;

	UartRCC_Configuration(num, FALSE);
	UartGPIO_Configuration(num, UartIrq[num]->Sett->Remap);
	NVIC_Configuration(num);

	UartIrq[num]->__Silent = (float) UartIrq[num]->Sett->Timer.Freq / //
			(UartIrq[num]->Baud / 10.0f) * //
			(UartIrq[num]->Sett->Silent + 1);

	USART_InitTypeDef init;

	USART_StructInit(&init);

	init.USART_BaudRate = UartIrq[num]->Baud;

	USART_Init(usart, &init);

	USART_ITConfig(usart, USART_IT_RXNE, ENABLE);

	USART_Cmd(usart, ENABLE);
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

lu16 UartIrqRxLen(lu8 num)
{
	assert_param(num < UARTS_IRQ_NUM);

	static u16 count_last = 0;

	if (UartIrq[num]->__Rx.Len)
	{ // сообщение уже есть
		return UartIrq[num]->__Rx.Len;
	}
	else
	{
		if (UartIrq[num]->__Rx.Counter == 0)
		{ // ничего не принял и не начинал даже
			return 0;
		}
		else
		{
			u32 time_cur = UartIrq[num]->Sett->Timer.Func();

			if (UartIrq[num]->__Rx.Counter == count_last)
			{ // длина не изменилась
				if (time_cur - UartIrq[num]->__LastByteTime >= UartIrq[num]->__Silent)
				{
					UartIrq[num]->__Rx.Len = UartIrq[num]->__Rx.Counter;
				}
			}

			count_last = UartIrq[num]->__Rx.Counter;
		}
	}

	return UartIrq[num]->__Rx.Len;
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

void UartIrqRxClear(lu8 num)
{
	assert_param(num < UARTS_IRQ_NUM);

	UartIrq[num]->__Rx.Counter = 0;
	UartIrq[num]->__Rx.Len = 0;
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

BOOLX UartIrqTxEmpty(lu8 num)
{
	assert_param(num < UARTS_IRQ_NUM);

	return (UartIrq[num]->__Tx.Len == 0) ? TRUE : FALSE;
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

BOOLX UartIrqTx(lu8 num, lu16 len)
{
	assert_param(num < UARTS_IRQ_NUM);

	if (!UartIrqTxEmpty(num))
	{
		return FALSE;
	}

	USART_TypeDef * usart = Sett[num].Regs;

	u8 * ptr = (u8 *) UartIrq[num]->Sett->Tx.Data;

	UartIrq[num]->__Tx.Counter = 1;
	UartIrq[num]->__Tx.Len = len;

	USART_SendData(usart, ptr[0]);

	USART_ITConfig(usart, USART_IT_TXE, ENABLE);

	return TRUE;
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

BOOLX UartIrqTxChar(lu8 num, lu8 c)
{
	assert_param(num < UARTS_IRQ_NUM);

	if (!UartIrqTxEmpty(num))
	{
		return FALSE;
	}

	USART_TypeDef * usart = Sett[num].Regs;

	USART_SendData(usart, c);

	return TRUE;
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

ls16 UartIrqRxChar(lu8 num)
{
	assert_param(num < UARTS_IRQ_NUM);

	s32 c;

	if (UartIrq[num]->__Rx.Counter)
	{
		u8 * ptr = (u8 *) UartIrq[num]->Sett->Rx.Data;

		c = ptr[0];

		UartIrq[num]->__Rx.Counter = 0;
		UartIrq[num]->__Rx.Len = 0;
	}
	else
	{
		c = -1;
	}

	return c;
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

void UartIrqISR(lu8 num)
{
	USART_TypeDef * usart = Sett[num].Regs;

	if (USART_GetITStatus(usart, USART_IT_RXNE) != RESET)
	{
		UartIrq[num]->__LastByteTime = UartIrq[num]->Sett->Timer.Func();

		lu8 c = USART_ReceiveData(usart);

		if (UartIrq[num]->__Rx.Len == 0)
		{ // только если буфер приема не занят
			if (UartIrq[num]->__Rx.Counter < UartIrq[num]->Sett->Rx.Max)
			{
				u8 * ptr = (u8 *) UartIrq[num]->Sett->Rx.Data;

				ptr[UartIrq[num]->__Rx.Counter] = c;

				++UartIrq[num]->__Rx.Counter;
			}
		}
	}

	if (USART_GetITStatus(usart, USART_IT_TXE) != RESET)
	{
		if (UartIrq[num]->__Tx.Len)
		{
			if (UartIrq[num]->__Tx.Counter < UartIrq[num]->__Tx.Len)
			{
				u8 * ptr = (u8 *) UartIrq[num]->Sett->Tx.Data;
				lu8 c = ptr[UartIrq[num]->__Tx.Counter];

				USART_SendData(usart, c);

				++UartIrq[num]->__Tx.Counter;
			}
			else
			{
				UartIrq[num]->__Tx.Len = 0;

				USART_ITConfig(usart, USART_IT_TXE, DISABLE);
			}
		}
	}
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

static void NVIC_Configuration(lu8 num)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	switch (num)
	{
	case 0:
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		break;

	case 1:
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		break;

	case 2:
		NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		break;

	default:
		break;
	}
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
