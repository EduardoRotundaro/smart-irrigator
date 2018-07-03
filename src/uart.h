//UART.h

//Declaracao das funcoes do UART.c

void ConfigUART (unsigned int baudrate);
void UARTReceive(char *rx,unsigned int tam);
void UARTSend (char tx[]) ;