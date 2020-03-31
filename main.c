/*
* Name: main.c
* Project: tinyClock
* Author: Julio Contreras - https://github.com/jcontrerasf
* Creation date: March 2020 - Quarantine
* Description: Clock that uses just 8 leds to display the time. Uses main power frequency for timekeeping.
* The first 4 leds represent the hour in binary, the next 3 represent minute tens and the last adds 5 minutes.
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

#define MAINS_FREQ 	50
#define CLOCK_PIN   0
#define DATA_PIN    1
#define LATCH_PIN   3


volatile uint8_t counter = 0;

//Function prototypes
static void init(void);
static void shift(uint8_t);
static uint8_t five(void);

typedef struct{
	unsigned char seconds;
	unsigned char minutes;
	unsigned char hours;
	}time;
	
	time t = {0,0,9};

ISR(INT0_vect) // PB2
{
  counter++;
  if(counter == MAINS_FREQ)
  {
  	counter = 0;
  	t.seconds++;
  	//PORTB ^= 1<<PORTB1; 
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
  		//PORTB = t.minutes/15;
  	}
  }
  shift(t.hours<<4|(t.minutes/10)<<1|five());
}

void init(void)
{
  MCUCR |= (1<<ISC01 | 1<<ISC00);		// The rising edge of INT0 generates an interrupt request
  GIMSK |= 1<<INT0;                 // Activate the INT0
  DDRB 	|= (1<<DDB0 | 1<<DDB1 | 1<<DDB3); 		// Set pins 0, 1 and 3 of port B as outputs
  sei();								            //Set the Global Interrupt Enable Bit
  //set_sleep_mode(SLEEP_MODE_PWR_SAVE);	//Selecting power save mode as the sleep mode to be used
  //sleep_enable();						//Enabling sleep mode
}

void shift(uint8_t send_byte)
{
  uint8_t send_bit;
  uint8_t i;

  for(i = 0; i < 8; i++)
  {
    PORTB &= ~(1<<CLOCK_PIN);
    send_bit = !!(send_byte & (1<<i));
    if(send_bit)
    {
      PORTB |= 1<<DATA_PIN;
    }
    else
    {
      PORTB &= ~(1<<DATA_PIN);
    }
    PORTB |= 1<<CLOCK_PIN;
  }
  PORTB |= 1<<LATCH_PIN;
  PORTB &= ~(1<<LATCH_PIN);
}

uint8_t five(void)
{
  if(t.minutes%10 < 5)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

int main()
{
  init();			// Initialize registers and configurations

  while(1)
  {
  	//sleep_mode();	//Enter sleep mode
  }

  return 0;
}