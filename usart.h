/*
 * usart.h
 *
 *  Created on: 24 set 2016
 *      Author: mauro
 */


#ifndef USART_H_
#define USART_H_

void USART2_Init(uint32_t speed);
uint8_t USART2_Read(uint8_t * const ris, const uint8_t len);
void USART2_Write(const uint8_t* mess, const uint8_t len);
bool USART2_isAvailable();
char USART2_readChar();

#endif /* USART_H_ */
