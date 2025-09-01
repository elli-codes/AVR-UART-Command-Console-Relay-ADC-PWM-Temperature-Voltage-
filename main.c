#include <mega32.h>
#include <stdio.h>
#include <string.h>
#include <delay.h> 
char data[17]; unsigned char i = 0; int flag = 0; float v, t; int p;
interrupt [USART_RXC] void usart_rx_isr(void) { 
char c = UDR; if (c == '\r' || c == '\n') { 
data[i] = '\0'; // End string if (strstr(data, "Relay on")) { flag = 1; } 
else if (strstr(data, "Relay off")) { flag = 2; } 
else if (strstr(data, "Volt?")) { flag = 3; } 
else if (strstr(data, "Temp?")) { flag = 4; } 
else if (strstr(data, "Pwm=")) { flag = 5; } i = 0; // reset buffer for next command } 
else { if (i < 16) data[i++] = c; } }
unsigned int adc_data; // Voltage Reference: AVCC #define ADC_VREF_TYPE ((0<<REFS1) | (1<<REFS0) | (0<<ADLAR)) interrupt [ADC_INT] void adc_isr(void) { 
adc_data = ADCW; } 
unsigned int read_adc(unsigned char adc_input) { 
ADMUX = adc_input | ADC_VREF_TYPE; 
delay_us(10);
ADCSRA |= (1<<ADSC); // Start conversion
while ((ADCSRA & (1<<ADIF))==0); ADCSRA |= (1<<ADIF); // Clear flag return ADCW; } 
// Timer 0 overflow interrupt service routine interrupt [TIM0_OVF] void timer0_ovf_isr(void) { }
void main(void) {
DDRC=0xff; // Relay pin = output 
DDRB=0xff; // pwm = output
PORTB=0x00; 
// USART init 9600 baud
UCSRB=(1<<RXCIE)|(1<<RXEN)|(1<<TXEN); 
UCSRC=(1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0); 
UBRRL=0x33;
// ADC init 
ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
#asm("sei") 
  printf("UART consule \r\n"); 
printf("Type command:");
while (1) {
//OCR0=0; blinking switch (flag) { 
case 1: PORTC |= (1<<0); // Relay ON 
printf("Relay is ON\r\n"); 
printf("Command :");
  flag = 0;
  break; 
//////
case 2:
  PORTC &= ~(1<<0); // Relay OFF 
printf("Relay is OFF\r\n"); 
  printf("Command :"); 
  flag = 0; 
  break;
/////
case 4: 
v = (float)read_adc(1) * 500 / 1023; 
  printf("Temp= %.2f C\r\n", v); 
  printf("Command :"); flag = 0;
  break;
///////
case 5: 
TCCR0=(1<<WGM00) | (1<<COM01) | (0<<COM00) | (1<<WGM01) | (0<<CS02) | (1<<CS01) | (1<<CS00); 
TCNT0=0x00; 
OCR0=0x00; 
if( (data[0]=='P')&& (data[1]=='w')&& (data[2]=='m') && (data[3]=='=') ) {
p = (data[4]-'0')*10 + (data[5]-'0'); // two digits
OCR0 = (p * 255) / 100; }
printf("pwm is %d%%\r\n", p); 
  printf("Command :"); 
  flag = 0; break;
} 
} 
}
