/* 
 * File:   relayonpress.c
 * Author: Benoit
 *
 * Created on 5/02/2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <xc.h>
#include "header.h"

// Initialization
void main(void) {
   ANSEL = 0; // no analog GPIOs
   CMCON = 0x07; // comparator off
   ADCON0 = 0; // Analog to Digital and Digital to Analog convertors off
   TRISIO0 = 0; // output LED
   TRISIO1 = 1; // input footswtich
   TRISIO2 = 0; // output TGP222A
   TRISIO5 = 0; // output activation relay
   TRISIO4 = 0; // output ground relay
   TRISIO3 = 1; // input temporary switch

   GPIO = 0; // set outputs as low level (0V)
   
   // Variables definition
   uint8_t state; // on-off state of the pedal (1 = on, 0 = off)
   state = 0;
   
   uint8_t changestate; // to change status of the pedal
   changestate = 0;
   
   uint8_t temporary; // define the mode of the pedal : classic of temporary activation
   temporary = 0;
   
   uint8_t press_switch; // variable to detect if the switch is pressed
   press_switch = 0;
   
   // Main loop
   while(1) {
       if(GP3 == 0) { // If the "hold" mode is activated through the switch
          temporary = 0;
      }
      else { // If not, normal bypass switching
          temporary = 1;
      }
       
       // Normal mode
       if(temporary == 0) { 
           if(GP1 == 0) { // footswitch pressed
               __delay_ms(15); // debouncing
               if(GP1 == 0) { 
                  press_switch = press_switch + 1; // switch is on
                  if(press_switch>10) {
                      press_switch = 10; // max value for press_switch
                  } 
               }
           }
           if(press_switch == 1) { // switch is pressed : lets turn the pedal on or off
               changestate = 1;
               press_switch = 2; // avoid bugs if press_switch stays at 1
           }
           if(GP1 == 1) {
               __delay_ms(15); // debouncing
               if(GP1 == 1) { 
                  press_switch = 0; 
               }
           }
       }
       
       // Temporary mode
       if(temporary == 1) {
           if(GP1 == 0) { // if switch is pressed : temporary activation
               __delay_ms(15); // debouncing
               if(GP1 == 0) { 
                  press_switch = press_switch + 1; // switch is on
                  if(press_switch>10) {
                      press_switch = 10; // max value for press_switch
                  } 
               }
           }
           if(GP1 == 1) { // if switch is not pressed, turn it off
               __delay_ms(15); // debouncing
               if(GP1 == 1) { 
                   state = 0;
                   press_switch = 0;
               }
           }
           if(press_switch == 1) {
               changestate = 1; // if switch is pressed, turn the pedal on
               press_switch = 2; // avoids bug
           }
       }
       
       // Changing state of the pedal
       if(changestate == 1) {
           if(state == 0) { // change to on
               GP2 = 1; // photoFET on
               __delay_ms(20);
               GP0 = 1; // LED on
               GP5 = 1; // relay on
               GP4 = 0;
               __delay_ms(20);
               GP2 = 0; // photoFET off
               state = 1;
           }
          else { // change to off
               GP2 = 1;
               __delay_ms(20);
               GP0 = 0; // LED off
               GP5 = 0; // relay off
               GP4 = 0;
               __delay_ms(20);
               GP2 = 0;
               state = 0;
           }
          __delay_ms(10);
          changestate=0; // reset changestate
       }
       
       // To let the pedal in the good state (on or off)
       if (state == 1) { // effect on
            GP0 = 1; // LED on
            GP5 = 1; // relay on
            GP4 = 0; 
       }
        else { // effect off
            GP0 = 0; // LED off
            GP5 = 0; // relay off
            GP4 = 0; 
       }
   }
}