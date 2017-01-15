/*
 * usart.h
 *
 *  Created on: 24 set 2016
 *      Author: mauro
 */


#ifndef USART_H_
#define USART_H_

void USART1_Init(uint32_t speed);
uint8_t USART1_Read(uint8_t * const ris, const uint8_t len);
void USART1_Write(const void * const mess, const uint8_t len);

#endif /* USART_H_ */
