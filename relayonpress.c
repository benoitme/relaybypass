/*
* File:   relayonpress.c
* Author: Benoit
*
* Created on 5/02/2017
* Updated by KSM on 2017/11/03
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xc.h>
#include "header.h"

//------------------------------------------------------------------------------
// Updated by KSM 2017/11/03

// Для частоты таймера 1 = 4MHz
//#define INTS_PER_SECOND 	4      // (4000000/(4*4*65536)) = 3,814697265625

// Для частоты таймера 1 = 8MHz
#define INTS_PER_SECOND 	8      // (8000000/(4*4*65536)) = 7,62939453125

uint8_t lp_counter = 0;	// Счетчик срабатываний таймера 1 (8 раз в секунду)
uint8_t lp_check = 0;	// Флаг контроля длинного нажатия.

// Эта функция возрващает 1, если таймер отработал 1 секунду или больше.
int is_1sec_elapsed(void)
{
	return ( lp_counter >= INTS_PER_SECOND ? 1 : 0 );
}

// Эта функция вызывается каждый раз при срабатывании таймера 1
#INT_TIMER1                        
void clock_isr()                    
{
    // Если разрешен контроль длинного нажатия - считаем тики.
    if ( lp_check ) {
    	if ( is_1sec_elapsed() == 0 ) {
    		lp_counter = lp_counter + 1;
    	}
    }
}

//------------------------------------------------------------------------------
// Initialization
void main( void )
{
	ANSEL   = 0; // no analog GPIOs
	CMCON   = 0x07; // comparator off
	ADCON0  = 0; // Analog to Digital and Digital to Analog convertors off
	TRISIO0 = 0; // output LED
	TRISIO1 = 1; // input footswtich
	TRISIO2 = 0; // output TGP222A
	TRISIO5 = 0; // output activation relay
	TRISIO4 = 0; // output ground relay
	TRISIO3 = 1; // input temporary switch

	GPIO    = 0; // set outputs as low level (0V)

	// Variables definition
	uint8_t state = 0; 			// on-off state of the pedal (1 = on, 0 = off)
	uint8_t changestate = 0; 	// to change status of the pedal
	uint8_t press_switch = 0; 	// variable to detect if the switch is pressed

	// KSM - Настроим таймер 1 для контроля длительности нажатия
	setup_timer_1(T1_INTERNAL | T1_DIV_BY_4);
	enable_interrupts(INT_TIMER1);
	enable_interrupts(GLOBAL);

	// Main loop
	while ( 1 ) 
	{
   		// footswitch pressed 
   		if ( GP1 == 0 ) { 
   			__delay_ms( 15 ); // debouncing
   			if ( GP1 == 0 ) {
   				press_switch = press_switch + 1; // switch is on
   				if ( press_switch > 10 ) press_switch = 10; // max value for press_switch
   			}
   		}
   		if ( press_switch == 1 ) { // switch is pressed : lets turn the pedal on or off
   			changestate = 1;
   			press_switch = 2; // avoid bugs if press_switch stays at 1
   		}

   		// footswitch released
   		if ( GP1 == 1 ) {
   			__delay_ms( 15 ); // debouncing
   			if ( GP1 == 1 ) {
   				press_switch = 0;

   				// KSM - при отпускании кнопки проверяем, если реле было включено...
   				if ( state == 1 ) {
   					// ... и было долгое нажатие - выключим реле
   					if ( is_1sec_elapsed() ) {
   						state = 0;
   					}
   				}

   				// KSM - остановим подсчет (он нам не нужен)
   				lp_check = 0;           
   			}
   		}

		// Changing state of the pedal
		if ( changestate == 1 ) 
		{
			if ( state == 0 ) { // change to on
				GP2 = 1; // photoFET on
				__delay_ms( 20 );
				GP0 = 1; // LED on
				GP5 = 1; // relay on
				GP4 = 0;
				__delay_ms( 20 );
				GP2 = 0; // photoFET off
				state = 1;

				// KSM - при включении реле сбросим счетчик времени и разрешим его подсчет.
				lp_counter = 0;
				lp_check = 1;

			} else { // change to off
				GP2 = 1;
				__delay_ms( 20 );
				GP0 = 0; // LED off
				GP5 = 0; // relay off
				GP4 = 0;
				__delay_ms( 20 );
				GP2 = 0;
				state = 0;

				// KSM - при выключениии реле отключим счетчик времени (таймер продолжает работать)
				lp_check = 0;

			}
			__delay_ms( 10 );
			changestate = 0; // reset changestate
		}

		// To let the pedal in the good state (on or off)
		if ( state == 1 ) { // effect on
			GP0 = 1; // LED on
			GP5 = 1; // relay on
			GP4 = 0;
		} else { // effect off
			GP0 = 0; // LED off
			GP5 = 0; // relay off
			GP4 = 0;
		}
	}
}
