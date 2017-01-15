#include <stdint.h>
#include "stm32f30x_conf.h"
#include "usart.h"

static uint8_t min(const uint8_t a, const uint8_t b) {
	return a < b ? a : b;
}

static void NVIC_Config(void) {
	NVIC_InitTypeDef NVIC_InitStructure;

	// Enable the USART1 Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

volatile uint8_t input[256], output[256], input_index, input_limit,
		output_index, output_limit;

void USART1_IRQHandler(void) {

	if (USART1->ISR & USART_ISR_RXNE) {
		//ready to receive
		input[input_limit++] = (uint8_t) USART_ReceiveData(USART1);
	}
	if (USART1->ISR & USART_FLAG_TXE) {
		//ready to send
		if (output_limit != output_index) {
			USART_SendData(USART1, output[output_index++]);
		} else {
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
		}
	}
}

void USART1_Init(uint32_t speed) {

	static USART_InitTypeDef USART_InitStructure;
	static GPIO_InitTypeDef GPIO_InitStructure;
	/*!< At this stage the microcontroller clock setting is already configured,
	 this is done through SystemInit() function which is called from startup
	 file (startup_stm32f37x.s) before to branch to application main.
	 To reconfigure the default setting of SystemInit() function, refer to
	 system_stm32f37x.c file
	 */
	/* USARTx configured as follow:
	 - BaudRate = speed baud
	 - Word Length = 8 Bits
	 - One Stop Bit
	 - No parity
	 - Hardware flow control disabled (RTS and CTS signals)
	 - Receive and transmit enabled
	 */

	/* NVIC configuration */
	NVIC_Config();
	/* Enable GPIO clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	/* Enable USART clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* Connect PXx to USARTx_Tx */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_7);

	/* Connect PXx to USARTx_Rx */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_7);

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART Rx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = speed;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =
	USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	
	/* USART configuration */
	USART_Init(USART1, &USART_InitStructure);

	/* Enable USART */
	USART_Cmd(USART1, ENABLE);

	//enable RX interrupt, disable TX interrupt
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

uint8_t USART1_Read(uint8_t * const ris, const uint8_t len) {
	if (!len) {
		return 0;
	}
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
	uint8_t to_read = min(len, (uint8_t) (input_limit - input_index));
	for (uint8_t i = 0; i < to_read; i++) {
		ris[i] = input[input_index++];
	}
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	return to_read;
}

void USART1_Write(const void * const mess, const uint8_t len) {
	if (!len) {
		return;
	}
	uint8_t *p = (uint8_t*) mess;
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	for (uint8_t i = 0; i < len; i++) {
		output[output_limit] = p[i];
		output_limit++;
	}
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}
