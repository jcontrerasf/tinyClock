/*
* Name: main.c
* Project: tinyClock
* Author: Julio Contreras - https://github.com/jcontrerasf
* Creation date: March 2020 - Quarantine
* Description: Clock that uses just 8 leds to display the time. Uses main power frequency for timekeeping.
* The first 4 leds represent the hour in binary, the next 3 represent minute tens (also en binary) and the last adds 5 minutes.
* Example: 09:35	=	⬛⬜⬜⬛ ⬜⬛⬛ ⬛ 	  =   1*2^3 + 0*2^2 + 0*2^1 + 1*2^0 : 0*2^2 + 1*2^1 + 1*2^0 + 1*5
* Example: 02:10	=	⬜⬜⬛⬜ ⬜⬜⬛ ⬜ 	  =   0*2^3 + 0*2^2 + 1*2^1 + 0*2^0 : 0*2^2 + 0*2^1 + 1*2^0 + 0*5
* Useful links:
* https://www.cypress.com/file/157911/download
* https://en.wikipedia.org/wiki/Utility_frequency#Long-term_stability_and_clock_synchronization
* http://www.cdec-sing.cl/pls/portal/cdec.pck_web_cdec_sing.sp_pagina?p_reporte=ctrl_frec_indices&p_id=5119
* Based on Atmel aplication note AVR134.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define MAINS_FREQ  50    // Mains or line frequency
#define CLOCK_PIN   4     // Pin to connect to 74HC595 clock pin 
#define DATA_PIN    1     // Pin to connect to 74HC595 data pin
#define LATCH_PIN   3     // Pin to connect to 74HC595 latch pin
#define OUT_CMP     20    // Value that uses the timer to create the PWM (0 to 255)
                          // Due to inverting PWM mode is set, lower value means lower brightness

//  Function prototypes
static void init(void);
static void init_timer(void);
static void shift(uint8_t);
static uint8_t five(void);

volatile uint8_t counter = 0;

typedef struct{
	unsigned char hours;
	unsigned char minutes;
	unsigned char seconds;
	}time;
	
	time t = {12,0,0}; //  Set time

// Interrupt Service Routine
ISR(INT0_vect) // INT0 is PB2
{
  counter++;
  if(counter == MAINS_FREQ)
  {
  	counter = 0;
  	t.seconds++;
  	//PORTB ^= 1<<PORTB1; // Toggles pin PB1 each second
  	if(t.seconds == 60)
  	{
  		t.seconds = 0;
  		t.minutes++;
  		if(t.minutes == 60)
  		{
  			t.minutes = 0;
  			t.hours++;
  			if(t.hours == 13)
  			{
  				t.hours = 1;
  			}
  		}
  		shift(t.hours<<4|(t.minutes/10)<<1|five()); // Calculates and send the byte to show in the 8 LEDs
  	}
  }
}

void init(void)
{
  MCUCR |= (1<<ISC01 | 1<<ISC00);		// The rising edge of INT0 generates an interrupt request
  GIMSK |= 1<<INT0;				// Activate the INT0
  DDRB 	|= (1<<DDB1 | 1<<DDB3 | 1<<DDB4); 	// Set pins 1, 3 and 4 of port B as outputs
  sei();					// Set the Global Interrupt Enable Bit
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);	    	// Selecting power save mode as the sleep mode to be used
  sleep_enable();		                // Enabling sleep mode
}

void init_timer(void)
{
  DDRB   |= 1<<DDB0;                  // Set pin 0 as output
  TCCR0B |= 1<<CS00;                  // No prescaling and Waveform Generation Mode
  TCCR0A |= (1<<COM0A0 | 1<<COM0A1);  // Inverting PWM mode
  TCCR0A |= (1<<WGM01 | 1<<WGM00);    // Fast PWM
  OCR0A   = OUT_CMP;                  // Set the comparation value (0 to 255)
}

// Function that sends a byte to 74HC595
void shift(uint8_t send_byte)
{
  uint8_t send_bit;
  uint8_t i;

  for(i = 0; i < 8; i++)
  {
    PORTB &= ~(1<<CLOCK_PIN);           // Clock pin low
    send_bit = !!(send_byte & (1<<i));  // Prepare bit to send
    if(send_bit)
    {
      PORTB |= 1<<DATA_PIN;             // If send bit is 1 then set data pin as 1
    }
    else
    {
      PORTB &= ~(1<<DATA_PIN);          // Else set as 0
    }
    PORTB |= 1<<CLOCK_PIN;              // Clock pin high
  }
  PORTB |= 1<<LATCH_PIN;                // Pulse to set the
  PORTB &= ~(1<<LATCH_PIN);             // output latch (see 74HC595 datasheet)
}


uint8_t five(void)
{
  if(t.minutes%10 < 5)
  {
    return 0;          // If minute units are lower than five return 0
  }
  else
  {
    return 1;          // If minute units are higher return 1
  }
}

int main()
{
  init_timer();	  // Initialize timer for PWM generation
  init();	  // Initialize registers and configurations
  shift(t.hours<<4|(t.minutes/10)<<1|five()); // Initial display

  while(1)
  {
  	sleep_mode();	// Enter sleep mode
  }

  return 0;
}
