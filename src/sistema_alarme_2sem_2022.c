#define F_CPU 16000000UL
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include "serial.h"
#include "lcd_display.h"

#define BETA 3950

//PINOS
#define LED_ZONA_A 	(1 << PORTB3)
#define LED_ZONA_B 	(1 << PORTB2)
#define BZR_ALARME 	(1 << PORTB4)
#define BTN_ZN_A		(1 << PINB0)
#define BTN_ZN_B		(1 << PINB1)
#define VL_BTN_A		(PINB & (1 << PINB0))
#define VL_BTN_B		(PINB & (1 << PINB1))
#define BTN1_INPUT	(PINC & (1 << PORTC1))
#define BTN2_INPUT	(PINC & (1 << PORTC3))
#define BTN3_INPUT	(PINC & (1 << PORTC4))
#define BTN4_INPUT	(PINC & (1 << PORTC5))

char valorA, valorB, anteriorA=0, anteriorB=0, estadoA=0, estadoB=0,valorBtnAlarme;
char estadoAlarme=0;
char isZonaAAtiva=0, isZonaBAtiva=0;
int temperatura, tempCelsius=24;//já começa com um padrão de tempr. ambiente
char situacaoPortaSala=0,situacaoJanSala=0,situacaoQuarto1=0,situacaoQuarto2=0;
char strTemperatura[20]={""};
uint8_t estadoPortaSala=0,estadoJanSala=0,estadoQuarto1=0,estadoQuarto2=0;

int main(){
	inicializaUSART();
	//Serial.begin(115200);
	LCD_Init();

	//CONFIG PORTAS/PINOS
	//=====================================================================
	// Sáidas DIGITAIS
	//DDRB PINOS 8, 9 e 13 do arduino uno
	//DDRC PINOS "Analógicos" A5,A4,A3 e A1
		//OBS sensor NTC temperatura está no A0

	// Entradas DIGITAIS
	//DDRB PINOS 10, 11 e 12
	//=====================================================================

   DDRB |= (1 << DDB2) | (1<<DDB3) | (1 << DDB4); //
   DDRB &= ~(1 << DDB0) & ~(1 << DDB1) & ~(1 << DDB5);
   DDRC &= ~(1<<PC5) & ~(1<<PC4) & ~(1<<PC3) & ~(1<<PC1);

	/*EICRA |= (1 << ISC00) | (1 << ISC01) | (1 << ISC10) | (1 << ISC11);
   //EICRA |= (1 << ISC00) | (1 << ISC01)| (1 << ISC10)| (1 << ISC11);
   EIMSK |= (1 << INT0) | (1 << INT1);*/   //PIND 3 

	PCICR |= (1 << PCIE0) | (1 << PCIE1);

	//=====================================================================
	//PINOS INTERRUPÇÔES
	// PCMSK0 pinos 8 e 9 respectivamente
	// PCMSK1 pinos A1, A5, A4 e A3
	//=====================================================================

 	PCMSK0 |= (1 << PCINT0) | (1 << PCINT1);
	PCMSK1 |= (1 << PCINT9) | (1 << PCINT11)| (1 << PCINT12) | (1 << PCINT13);
  SREG |= (1 << SREG_I);  //HABILITANDO INTERRUP.

  //REGISTRADORES ADC
	ADMUX = (1 << REFS0);
  ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0)| (1 << ADEN);
  _delay_ms(100);

   while(1){
		 reseta();
		 exibeTudo(741);
	 }
}


//ZONAS
ISR(PCINT0_vect) {
	//Zona A
	 valorA = VL_BTN_A == BTN_ZN_A;//! lê pino 8
		if (valorA == 1 && anteriorA == 0) {
			estadoA=!estadoA;
		}
		
	 if (estadoA == 1){
		PORTB |= LED_ZONA_A;
		isZonaAAtiva=1;
		transmiteBytesString("Zona A está ativa\n");
		_delay_ms(25);
	 }else{
		PORTB &= ~LED_ZONA_A;
		isZonaAAtiva=0;
	 }anteriorA = valorA;
	 _delay_ms(50);
	 
	 //Zona B
	 valorB = VL_BTN_B == BTN_ZN_B;// lê pino 9
			if (valorB == 1 && anteriorB == 0) {
				estadoB=!estadoB;
			}
		 if (estadoB == 1){
			PORTB |= LED_ZONA_B;
			isZonaBAtiva=1;
			transmiteBytesString("Zona B está ativa\n");
			_delay_ms(25);
		 }else{
			PORTB &= ~LED_ZONA_B;
			isZonaBAtiva=0;
		 }anteriorB = valorB;
		 _delay_ms(50);
		 
}

//ABRIR e FECHAR JANELAS/PORTAS
ISR(PCINT1_vect){
	//leTemperatura();

	//PINOS ANAlÓGICOS USADOS COMO DIGITAIS CITADOS ACIMA
	
	situacaoPortaSala = BTN1_INPUT;
	situacaoJanSala = BTN2_INPUT;
	situacaoQuarto1 = BTN3_INPUT;
	situacaoQuarto2 = BTN4_INPUT;
	_delay_ms(50);
	
	if(situacaoPortaSala) 	estadoPortaSala=!estadoPortaSala;
	if(situacaoJanSala) 	estadoJanSala=!estadoJanSala;  
	if(situacaoQuarto1) 	estadoQuarto1=!estadoQuarto1;  
	if(situacaoQuarto2) 	estadoQuarto2=!estadoQuarto2;  
	

	if((isZonaAAtiva==1 && (estadoPortaSala==1 || estadoJanSala==1))){
		acionaAlarme();
	}
	
	if((isZonaBAtiva==1 && (estadoQuarto1==1 || estadoQuarto2==1))){
		acionaAlarme();
	}
}

int leTemperatura(){
	
	ADCSRA |= (1 << ADSC);
  while ((ADCSRA & (1 << ADSC)) != 0);
  temperatura = ADC;
	tempCelsius = 1 / (log(1 / (1023.0 / temperatura - 1)) / BETA + 1.0 / 298.15) - 273.15;
	
	if(tempCelsius>30.0){
		acionaAlarme();
	}

	transmiteBytesString("Temperatura: ");
	transmiteBytesInteger(tempCelsius);
	transmiteBytesString(" C\n");
	return tempCelsius;
}

void acionaAlarme(){
	PORTB |= BZR_ALARME;//liga alarme pino 12
	transmiteBytesString("ALARMEEEEEEEE!\n");
	estadoAlarme=1;
	//_delay_ms(25);
}

void espera_ms(unsigned char ms){
	if(ms>0){
		_delay_ms(1);
		ms--;
	}
}

void printaTemperatura(unsigned int tempo_ms){
	//TODO estas duas funções de conversão chamam o monitor serial sem eu mandar
	sprintf(strTemperatura,"%d",leTemperatura()); //TALVEZ NAO SEJA A MELHOR FX
	//dtostrf(leTemperatura(),2,2,strTemperatura);
	
	LCD_PrintXY(0,0,"Temp ");
	//sprintf(strTemperatura, "%c", 0b11011111); // '°' character
	LCD_Print(strTemperatura);
	LCD_PrintXY(8,0," C");
}

//aperte/segure o botão até desligar o alarme 
	//(exceto se a temperatura ainda estiver muito alta)
void reseta(){
	//reseta os estados e para o alarme
	valorBtnAlarme = ((PINB & (1 << PINB5)) == (1 << PINB5));
	_delay_ms(50);

	if(valorBtnAlarme){
		PORTB &= ~BZR_ALARME;
		_delay_ms(50);

		estadoA=0;
		estadoB=0;
		estadoPortaSala=0;
		estadoJanSala=0;
		estadoQuarto1=0;
		estadoQuarto2=0;
		estadoAlarme=0;
	}
}

void exibeTudo(unsigned int ms){
	LCD_Clear();
	printaTemperatura(341);
	_delay_ms(ms);

	LCD_Clear();
	printaZonas();
	_delay_ms(ms);
	
	LCD_Clear();
	printaComodos();
	_delay_ms(ms);
}
//===============================================================
//monitoramento portas e janelas On = ativo , Off = desativado
// porta ou janela  A = aberta  F = fechada
//===============================================================
void printaZonas(){
	char *znA = "-";
	char *znB = "-";
	if(isZonaAAtiva){
		znA=" On";
	}else if(!isZonaAAtiva){
		znA=" Off";
	}
	
	if(isZonaBAtiva){
		znB=" On";
	}else if(!isZonaBAtiva){
		znB=" Off";
	}
	LCD_PrintXY(0,0,"zA: ");
	LCD_PrintXY(4,0,znA);
	LCD_PrintXY(0,1,"zB: ");
	LCD_PrintXY(4,1,znB);
}

void printaComodos(){
	char *sala = "-";
	char *sala2 = "-";
	char *quarto1="-";
	char *quarto2="-";
	if(estadoPortaSala){
		sala=" A";
	}else if(!estadoPortaSala){
		sala=" F";
	}
	
	if(estadoJanSala){
		sala2=" A";
	}else if(!estadoJanSala){
		sala2=" F";
	}
	
	if(estadoQuarto1){
		quarto1=" A";
	}else if(!estadoQuarto1){
		quarto1=" F";
	}
	
	if(estadoQuarto2){
		quarto2=" A";
	}else if(!estadoQuarto2){
		quarto2=" F";
	}
	
	LCD_PrintXY(0,0,"pSl ");
	LCD_PrintXY(0,1,sala);
	LCD_PrintXY(4,0,"jSl ");
	LCD_PrintXY(4,1,sala2);
	LCD_PrintXY(8,0,"Q1 ");
	LCD_PrintXY(8,1,quarto1);
	LCD_PrintXY(11,0,"Q2 ");
	LCD_PrintXY(11,1,quarto2);
}
