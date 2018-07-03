#include<msp430g2553.h>
#include<stdio.h>
#include "uart.h"
#include "timer.h"
#include "ad.h"

/*** maquina estados ***/
#define DIA_QUENTE 1
#define DIA_FRIO 2
#define NOITE_QUENTE 3
#define NOITE_FRIO 4

/*** sensor luz ***/
#define A0 INCH_0               //Canal 0 para conversao (pino P1.0=A0)
#define ADIN0 BIT0                //Pino P1.0 como entrada analogica

/*** sensor de temperatura ***/
#define A4 INCH_4               //Canal 4 para conversao (pino P1.4=A4)
#define ADIN4 BIT4               //Pino P1.4 como entrada analogica

/*** circuito ***/
#define MOTOR BIT6                      //pino 1.6 para acionar circuito
#define MOTOR_ON P1OUT|=MOTOR          //Liga circuito
#define MOTOR_OFF P1OUT&=(~MOTOR)      //Desliga circuito

/*** interrupcao externa pelo botao ***/
#define BOTAO BIT3

/****************************************************************************/

/*** sensor luz ***/
unsigned int luz;
unsigned long int aux;
unsigned int tensao;

/*** sensor temperatura ***/
float valor_lm35;
float soma_lm;

/*** timer ***/
extern char hora,minuto,segundo,milisegundo; //declaradas em timer.c
extern unsigned int tempo_limite; //declarada em timer.c
char relogio_texto[3];

/*** uart ***/
unsigned char mensagem[20];

/*** maq estados ***/
unsigned char estado = DIA_QUENTE;

/***************************************************************************/


void configurar(){
	
        BCSCTL1=CALBC1_1MHZ;            //calibra para 1MHz
        DCOCTL=CALDCO_1MHZ;
        //Intervalo=100ms. Se clock=1MHz, T=1us -> contagem=50000 divisor=2
        //50000 x 1us/2 = 50000 x 2us = 100000us = 100ms
        __delay_cycles(100000);
        ConfigTimer0(50000,2);         //Inicia o relogio para funcionar
  
        ConfigAD(ADIN0 + ADIN4);        //Configura AD no pino P1.0 e P1.4
        
	P1DIR |= MOTOR;                //pino p1.6 configurado para saida
	MOTOR_OFF;                     //desliga a saida do circuito
	
	_BIS_SR(GIE);                  //habilita interrupcao global
	
        ConfigUART(9600);             //Config taxa de dados p/ 9600bps
        
	P1REN |= BOTAO;               //habilita o resistor interno
        P1OUT |= BOTAO;               //config o resistor interno como Pull-Up
	P1IES |= BOTAO;               //config para transição de descida
	P1IFG = 0;                    //limpa as interrupções
	P1IE |= BOTAO;                //habilita a interrupção do pino
	
}

void main (void){
	
        int i;
        
	WDTCTL = WDTPW + WDTHOLD;
	
	configurar();
        StartTimer0();
        
        UARTSend("--- SMART IRRIGATOR ---\n\r");
	
	for(;;){
          
          /***** lm35 *****/
          soma_lm = 0;
          for(i=0;i<1000;i++){
            valor_lm35 = (LeAD(A4) * 650) / 1023;
            soma_lm += valor_lm35;
          }
          soma_lm = soma_lm/1000;
          
          /***** ldr *****/
          luz = LeAD(A0);    
          aux = (unsigned long int) luz;
          aux = (aux*3300)/1023;         
          tensao=(unsigned int)aux;  
          
          sprintf(relogio_texto,"%.2d",hora);
          UARTSend(relogio_texto);
          UARTSend (":");
          sprintf(relogio_texto,"%.2d",minuto);
          UARTSend (relogio_texto);
          UARTSend (":");
          sprintf(relogio_texto,"%.2d",segundo);
          UARTSend (relogio_texto);
          UARTSend ("\n\r");
	 
          switch(estado){
            
          case DIA_QUENTE:
              MOTOR_ON;
              UARTSend("\n\nDia\n\r");
              UARTSend("Irrigação ligada\n\r");
              UARTSend("Temperatura = ");
              sprintf(mensagem, "%.2f\r", soma_lm);
              UARTSend(mensagem);
              __delay_cycles(1000000000);
              UARTSend("\nIrrigação desligada\n\n\r");
              MOTOR_OFF;
              
              if(tensao >= 2600 && soma_lm < 28.0 ){
                 estado = DIA_FRIO;
              }
              else if(tensao < 2600 && soma_lm > 28.0 ){
                 estado = NOITE_QUENTE;
              }
              else if(tensao < 2600 && soma_lm < 28.0 ){
                estado = NOITE_FRIO;
              }
              
              break;
          
          case DIA_FRIO:
              UARTSend("\n\nDia\n\r");
              UARTSend("Irrigação desligada\n\r");
              UARTSend("Temperatura = ");
              sprintf(mensagem, "%.2f\r", soma_lm);
              UARTSend(mensagem);
              
              if(tensao >= 2600 && soma_lm > 28.0 ){
                 estado = DIA_QUENTE;
              }
              else if(tensao < 2600 && soma_lm > 28.0 ){
                 estado = NOITE_QUENTE;
              }
              else if(tensao < 2600 && soma_lm < 28.0 ){
                estado = NOITE_FRIO;
              }
              
              break;
              
          case NOITE_FRIO:
              UARTSend("\n\nNoite\n\r");
              UARTSend("Irrigação desligada\n\r");
              UARTSend("Temperatura = ");
              sprintf(mensagem, "%.2f\r", soma_lm);
              UARTSend(mensagem);
              
              if(tensao >= 2600 && soma_lm > 28.0 ){
                 estado = DIA_QUENTE;
              }
              else if(tensao >= 2600 && soma_lm < 28.0 ){
                 estado = DIA_FRIO;
              }
              else if(tensao < 2600 && soma_lm > 28.0 ){
                 estado = NOITE_QUENTE;
              }
              
              break;
              
          case NOITE_QUENTE:
              MOTOR_ON;
              UARTSend("\n\nNoite\n\r");
              UARTSend("Irrigação ligada\n\r");
              UARTSend("Temperatura = ");
              sprintf(mensagem, "%.2f\r", soma_lm);
              UARTSend(mensagem);
              __delay_cycles(1000000000);
              UARTSend("\nIrrigação desligada\n\n\r");
              MOTOR_OFF;
              
              if(tensao >= 2600 && soma_lm > 28.0 ){
                 estado = DIA_QUENTE;
              }
              else if(tensao >= 2600 && soma_lm < 28.0 ){
                 estado = DIA_FRIO;
              }
              else if(tensao < 2600 && soma_lm < 28.0 ){
                estado = NOITE_FRIO;
              }
              
              break;
          }
          
	}
	
}

#pragma vector=PORT1_VECTOR
__interrupt void portal1_interrupcao (void){
        
        __delay_cycles(250000);
	
	P1IFG &= (~BOTAO);   //desliga a flag de pedido
	
	P1OUT ^= MOTOR;   //ou exclusiva para ligar ou desligar o circuito
}
