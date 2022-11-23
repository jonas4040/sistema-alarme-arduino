#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#ifndef F_COM
#define F_COM 16UL
#endif
#ifndef BAUD
#define BAUD 115200
#endif

#define BAUD_PRESCALE ((F_CPU / (F_COM * BAUD)) - 1)

#include <avr/io.h>
#include <stdio.h>

void inicializaUSART() {
  //Serial.begin(9600);
  UBRR0L = BAUD_PRESCALE;
  UBRR0H = (BAUD_PRESCALE >> 8);
  
  UCSR0B = (1 << TXEN0) | (1 << RXEN0);//! transmissão e recepção ao mesmo tempo
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);//! setando comunicação em 8bits
} 
void transmiteBit(unsigned char dado) {
  while (!(UCSR0A & (1<<UDRE0)));
  UDR0 = dado;
}
void transmiteBytesString (char *texto) {//equivalente ao Serial.print()
  while (*texto != '\0')
    transmiteBit (*texto++);
  //transmiteBit('\n');
}

void transmiteBytesInteger (int valor) {//equivalente ao Serial.print()
  char strValor[10];
  sprintf(strValor, "%d", valor); // converte valor para uma cadeia de caracteres
  transmiteBytesString(strValor);

}
char recebeBit() {
  while (!(UCSR0A & (1<<RXC0)));
    return (UDR0);
}
