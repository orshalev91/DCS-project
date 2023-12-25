#ifndef TFC_UART_H_
#define TFC_UART_H_

void Uart0_Br_Sbr(int sysclk, int baud);
void InitUARTs();
void UART0_IRQHandler();
char uart_getchar (UART_MemMapPtr channel);
void uart_putchar (UART_MemMapPtr channel, char ch);
void UARTprintf(UART_MemMapPtr channel,char* str);
void RF433MHz_transmitter(UART_MemMapPtr channel,char* Buffer);
#endif /* TFC_UART_H_ */
