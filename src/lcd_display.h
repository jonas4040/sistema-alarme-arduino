#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#include <avr/io.h>
#include <util/delay.h>
#ifndef LCD_Port
#define LCD_Port PORTD
#endif
#ifndef LCD_DPin
#define LCD_DPin DDRD
#endif
#ifndef RSPIN
#define RSPIN PD2
#endif
#ifndef ENPIN
#define ENPIN PD3
#endif
void LCD_StartRW() {
  LCD_Port |= (1 << ENPIN);
  _delay_us(1);
  LCD_Port &= ~ (1 << ENPIN);
}
void LCD_Cmd( unsigned char cmd ) {
  LCD_Port = (LCD_Port & 0x0F) | (cmd & 0xF0);
  LCD_Port &= ~(1 << RSPIN);
  LCD_StartRW();
  _delay_us(200);
  LCD_Port = (LCD_Port & 0x0F) | (cmd << 4);
  LCD_StartRW();
  _delay_ms(2);
}
void LCD_Init() {
  LCD_DPin |= (1 << PD7) | (1 << PD6) | (1 << PD5) | (1 << PD4) | (1 << PD3) | (1 << PD2);
  _delay_ms(15);
  LCD_Cmd(0x03);
  _delay_ms(4.1);
  LCD_Cmd(0x03);
  _delay_us(100);
  LCD_Cmd(0x03);
  LCD_Cmd(0x02);
  LCD_Cmd(0x0C);
  LCD_Cmd(0x06);
  LCD_Cmd(0x01);
  _delay_ms(2);
}
void LCD_Clear() {
  LCD_Cmd (0x01);
  _delay_ms(2);
  LCD_Cmd (0x80);
}
void LCD_Print (char *str) {
  for(int i = 0; str[i] != 0; i++) {
    LCD_Port = (LCD_Port & 0x0F) | (str[i] & 0xF0);
    LCD_Port |= (1 << RSPIN);
    LCD_StartRW();
    _delay_us(200);
    LCD_Port = (LCD_Port & 0x0F) | (str[i] << 4);
    LCD_StartRW();
    _delay_ms(2);
  }
}
void LCD_PrintXY (unsigned char col, unsigned char lin, char *str) {
  unsigned char end_lin[] = {0x80, 0xC0};
  if (lin < 2 && col < 16)
    LCD_Cmd((col & 0x0F) | end_lin[lin]);
  LCD_Print(str);
}
