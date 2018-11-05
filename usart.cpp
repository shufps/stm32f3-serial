#include <stdint.h>
#include "stm32f30x_conf.h"
#include "usart.h"

static uint8_t min(const uint8_t a, const uint8_t b) {
	return a < b ? a : b;
}
static void NVIC_Config(void) {
	NVIC_InitTypeDef NVIC_InitStructure;

	// Enable the USART2 Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

volatile uint8_t input[256], output[256], input_index, input_limit,
		output_index, output_limit;

volatile int uartcnt = 0;

extern "C" void USART2_IRQHandler(void) {

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
		//ready to receive
		while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET) {
			input[input_limit % sizeof(input)] = (uint8_t) USART_ReceiveData(USART2);
			input_limit++;
		}
		//USART_ClearFlag(USART2, USART_FLAG_RXNE);
	}

	if (USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET) {
		USART_ClearFlag(USART2, USART_FLAG_ORE);
	//	USART_ReceiveData(USART2);
	}
	uartcnt++;
/*	if (USART2->ISR & USART_FLAG_TXE) {
		//ready to send
		if (output_limit != output_index) {
			USART_SendData(USART2, output[output_index % sizeof(output)]);
			output_index++;
		} else {
			USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
		}
	}*/
}

void USART2_Init(uint32_t speed) {

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
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* Connect PXx to USARTx_Tx */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_7);

	/* Connect PXx to USARTx_Rx */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_7);

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART Rx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = speed;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =
	USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	
	/* USART configuration */
	USART_Init(USART2, &USART_InitStructure);

	/* Enable USART */
	USART_Cmd(USART2, ENABLE);
	//enable RX interrupt, disable TX interrupt
//	USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

uint8_t USART2_Read(uint8_t * const ris, const uint8_t len) {
	if (!len) {
		return 0;
	}
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
	uint8_t to_read = min(len, (uint8_t) (input_limit - input_index));
	for (uint8_t i = 0; i < to_read; i++) {
		ris[i] = input[input_index % sizeof(input)];
		input_index++;
	}
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	return to_read;
}

bool USART2_isAvailable() {
//	return USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET;
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
	uint8_t to_read = min(1, (uint8_t) (input_limit - input_index));
	bool ret = to_read;
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	return ret;
}

char USART2_readChar() {
//	return USART_ReceiveData(USART2);
	uint8_t ret = 0;
	USART2_Read(&ret, 1);
	return ret;
}

void USART2_Write(const uint8_t* mess, const uint8_t len) {
	if (!len) {
		return;
	}
	for (int i=0;i<len;i++) {
		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) ;
		USART_SendData(USART2, mess[i]);
	}
/*	uint8_t *p = (uint8_t*) mess;
	USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	for (uint8_t i = 0; i < len; i++) {
		output[output_limit % sizeof(output)] = p[i];
		output_limit++;
	}
	USART_ITConfig(USART2, USART_IT_TXE, ENABLE);*/
}
