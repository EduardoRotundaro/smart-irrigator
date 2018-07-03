/********************************************************************/
/* FUNCOES ACESSO UART                                              */
/* DEVICE:MSP430G2553 (TEXAS INSTRUMENTS)                           */
/*                                                                  */
/* INTERFACE: IAR EMBEDDED WORKBENCH IDE FOR MSP430 V.6             */
/*                                                                  */
/* HARDWARE :                                             	    */
/* TX do MCU = P1.2                                                 */
/* RX do MCU = P1.1                                                 */
/* OBS: lembrar dos jumpers do kit launchpad!!                      */
/********************************************************************/
#include "uart.h"
#include <msp430g2553.h>

/* Configura o canal UART para operar
  - TX no pino P1.2 e RX no pino P1.1
  - Configura o modo de operacao em 8N1 (padrao dos registros)
  - Configura para usar o clock SMCLK como gerador de baudrate
  - Deve-se ajustar a constante do valor do SMCLK para calculo do baudrate
  - Nao utiliza nenhuma modulacao para gerar o baudrate
  - Entrada = baudrate desejado em bps (9600, 38400, etc)
*/
#define SMCLK 1000000

void ConfigUART (unsigned int baudrate)    // Função que configura a UART com a taxa desejada
{
  P1SEL = BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
  P1SEL2 = BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK como clock (ajustar definicao)
  UCA0BR0 = (SMCLK/baudrate)%256;           //taxa de comunicacao
  UCA0BR1 = (SMCLK/baudrate)/256;           //taxa de comunicacao
  UCA0MCTL = UCBRS0;                        // Sem modulação para gerar a taxa
  UCA0CTL1 &= ~UCSWRST;                     // Inicia o canal serial
}
/******************************************************************************/

/* Transmite um vetor pelo canal serial
  - Parametro de Entrada = vetor de dados a ser enviado, terminado em 0x00
*/
/******************************************************************************/
void UARTSend (char tx[])              
{
  int i=0;
  while(tx[i]!=0)
  {
      while (!(IFG2&UCA0TXIFG));                //Não está transmitindo?
      UCA0TXBUF = tx[i];                        //Transmite o dado
      i++;  
  }
}
/******************************************************************************/

/* Recebe um vetor pelo canal serial
  - Parametro de Entrada = vetor de dados para salvar e numero de bytes a serem
    recebidos. Se tam=0, nao aguarda receber e retorna 0 se nao recebeu dados
    Se tam>0, aguarda receber a quantidade de dados indicada, nao retornando da
    funcao
*/
void UARTReceive(char *rx,unsigned int tam)
{
  int i=0;
  if(tam==0)            //se nao aguarda receber, realiza a leitura
  {
    if((IFG2&UCA0RXIFG)!=0)
    {
      IFG2=IFG2&(~UCA0RXIFG);              //Desliga flag indicador de recepcao
      *rx=UCA0RXBUF;                       //salva o dado disponivel
      rx++;
    }
  }
  else
  {
    while(i<tam)
    {
        while ((IFG2&UCA0RXIFG)==0);            //Aguarda receber o dado
        IFG2=IFG2&(~UCA0RXIFG);                 //Desliga flag indicador de recepcao
        *rx=UCA0RXBUF;                          //salva o dado
        rx++;
        i++;  
    }
  }  
  *rx=0;                                        //finaliza o dado recebido
}