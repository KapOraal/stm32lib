/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
// Ryabinin (c) 2012
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
#include <stm32f10x_usart.h>
#include <stm32f10x_dma.h>
#include "uart-common/uart-common.h"
#include "uart-dma.h"
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

typedef struct
{
	USART_TypeDef *Regs; ///< Адрес блока регистров уарта
	struct
	{
		DMA_Channel_TypeDef *Tx; ///< Адрес DMA для передачи
		DMA_Channel_TypeDef *Rx; ///< Адрес DMA для приема
	} Dma;
} Const_t;

static const Const_t Sett[UARTS_DMA_NUM] = { { USART1, { DMA1_Channel4, DMA1_Channel5, }, }, //
		{ USART2, { DMA1_Channel7, DMA1_Channel6, }, }, //
		{ USART3, { DMA1_Channel2, DMA1_Channel3, }, }, //
		};

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

/// Натсройки пользователя
static UartDma_t * UartDma[UARTS_DMA_NUM];

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

static void DMA_Configuration(lu8 num);

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

void UartDmaInit(UartDma_t * uart, lu8 num)
{
	assert_param(num < UARTS_DMA_NUM);
	assert_param(uart != NULL);

	USART_TypeDef * usart = Sett[num].Regs;
	UartDma[num] = uart;

	UartRCC_Configuration(num, TRUE);
	UartGPIO_Configuration(num, UartDma[num]->Sett->Remap);
	DMA_Configuration(num);

	UartDma[num]->__Silent = (float) UartDma[num]->Sett->Timer.Freq / //
			(UartDma[num]->Baud / 10) * //
			(UartDma[num]->Sett->Silent + 1);

	USART_InitTypeDef init;

	USART_StructInit(&init);

	init.USART_BaudRate = UartDma[num]->Baud;

	USART_Init(usart, &init);

	USART_Cmd(usart, ENABLE);

	UartDmaRxClear(num);
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

lu16 UartDmaRxLen(lu8 num)
{
	assert_param(num < UARTS_DMA_NUM);

	lu16 len = UartDma[num]->Sett->Rx.Max - DMA_GetCurrDataCounter(Sett[num].Dma.Rx);

	if (Sett[num].Dma.Rx->CCR & DMA_CCR1_EN)
	{ // готов принимать
		if (len)
		{
			u32 time_cur = UartDma[num]->Sett->Timer.Func();

			if (len == UartDma[num]->__Rx.Len)
			{ // длина не изменилась
				if (time_cur - UartDma[num]->__Rx.Time >= UartDma[num]->__Silent)
				{ // длина буфера не меняется уже довольно долго
					DMA_Cmd(Sett[num].Dma.Rx, DISABLE); // отключаем прием, чтобы DMA не похерило нам буфер приемника
				}
				else
				{ // прошло недостаточно времени, будем ждать окончания посылки
					len = 0;
				}
			}
			else
			{
				UartDma[num]->__Rx.Time = time_cur;
				UartDma[num]->__Rx.Len = len;

				len = 0; // как будто ничего не приняли
			}
		}
	}

	return len;
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

void UartDmaRxClear(lu8 num)
{
	assert_param(num < UARTS_DMA_NUM);

	UartDma[num]->__Rx.Len = 0;

	DMA_SetCurrDataCounter(Sett[num].Dma.Rx, UartDma[num]->Sett->Rx.Max);

	DMA_Cmd(Sett[num].Dma.Rx, ENABLE);

	USART_DMACmd(Sett[num].Regs, USART_DMAReq_Rx, ENABLE);
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

BOOLX UartDmaTxEmpty(lu8 num)
{
	assert_param(num < UARTS_DMA_NUM);

	return (DMA_GetCurrDataCounter(Sett[num].Dma.Tx) == 0) ? TRUE : FALSE;
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

BOOLX UartDmaTx(lu8 num, lu16 len)
{
	assert_param(num < UARTS_DMA_NUM);

	if (!UartDmaTxEmpty(num))
	{
		return FALSE;
	}

	DMA_Cmd(Sett[num].Dma.Tx, DISABLE);

	DMA_SetCurrDataCounter(Sett[num].Dma.Tx, len);

	USART_ClearFlag(Sett[num].Regs, USART_FLAG_TC);

	DMA_Cmd(Sett[num].Dma.Tx, ENABLE);

	USART_DMACmd(Sett[num].Regs, USART_DMAReq_Tx, ENABLE);

	return TRUE;
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

BOOLX UartDmaTxChar(lu8 num, lu8 c)
{
	assert_param(num < UARTS_DMA_NUM);

	if (UartDmaTxEmpty(num))
	{
		u8 *ptr = (u8 *) UartDma[num]->Sett->Tx.Data;
		ptr[0] = c;

		return UartDmaTx(num, 1);
	}

	return FALSE;
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

ls16 UartDmaRxChar(lu8 num)
{
	assert_param(num < UARTS_DMA_NUM);

	s32 c;

	if (UartDmaRxLen(num))
	{
		u8 * ptr = (u8 *) UartDma[num]->Sett->Rx.Data;

		c = ptr[0];

		UartDmaRxClear(num);
	}
	else
	{
		c = -1;
	}

	return c;
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/

void DMA_Configuration(lu8 num)
{
	DMA_InitTypeDef DMA_InitStructure;

	DMA_DeInit(Sett[num].Dma.Tx);

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &Sett[num].Regs->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) UartDma[num]->Sett->Tx.Data;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(Sett[num].Dma.Tx, &DMA_InitStructure);

	DMA_Cmd(Sett[num].Dma.Tx, ENABLE);

	DMA_DeInit(Sett[num].Dma.Rx);

	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &Sett[num].Regs->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) UartDma[num]->Sett->Rx.Data;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = UartDma[num]->Sett->Rx.Max;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(Sett[num].Dma.Rx, &DMA_InitStructure);

	DMA_Cmd(Sett[num].Dma.Rx, ENABLE);
}

/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
/*\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\*/
