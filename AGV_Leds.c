#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "AGV_Leds.h"

void init_Leds(void){
    DDRL = 0xff;
    DDRB |= (1 << PB2) | (1 << PB3);
    init_delay_led();
}

void LedTreeIndictorLeftToggle(){
        PORTB ^= (1 << TreeIndicatedLEDLeft);
}
void LedTreeIndictorRightToggle(){
        PORTB ^= (1 << TreeIndicatedLEDRight);
}

void LedTurnSignalLeftToggle(){
    PORTL ^= (1 << TurnSignalLEDLeft);
}
void LedTurnSignalRightToggle(){
        PORTL ^= (1 << TurnSignalLEDRight);
}

void LedBreakLightLeft(int x){
    if(x){
        PORTL |= (1 << BreaklightsLeft);
    }
    else{
        PORTL &= ~(1 << BreaklightsLeft);
    }
}
void LedBreakLightRight(int x){
    if(x){
        PORTL |= (1 << BreaklightRight);
    }
    else{
        PORTL &= ~(1 << BreaklightRight);
    }
}

void setBreaklights(int x){
    LedBreakLightLeft(x);
    LedBreakLightRight(x);
}

void LedHeadlightLeft(int x){
    if(x){
        PORTL |= (1 << HeadlightsLeft);
    }
    else{
        PORTL &= ~(1 << HeadlightsLeft);
    }
}
void LedHeadlightRight(int x){
    if(x){
        PORTL |= (1 << HeadlightsRight);
    }
    else{
        PORTL &= ~(1 << HeadlightsRight);
    }
}

void setHeadlights(int x){
    LedHeadlightLeft(x);
    LedHeadlightRight(x);
}

void LedNoodstopFront(int x){
    if(x){
        PORTL |= (1 << NoodstopLEDFront);
    }
    else{
        PORTL &= ~(1 << NoodstopLEDFront);
    }
}
void LedNoodstopBack(int x){
    if(x){
        PORTL |= (1 << NoodstopLEDBack);
    }
    else{
        PORTL &= ~(1 << NoodstopLEDBack);
    }
}


volatile int TurnSignalLeft = 0;
volatile int TurnSignalRight = 0;
volatile int TreeSignalLeft = 0;
volatile int TreeSignalRight = 0;

#define blinkspeed 300
volatile int counter = 0;
ISR(TIMER2_OVF_vect){
    counter++;
    if(counter == blinkspeed){
        if(TurnSignalLeft) {
                LedTurnSignalLeftToggle();
        } else PORTL &= ~(1<<TurnSignalLEDLeft);
        if(TurnSignalRight){
                LedTurnSignalRightToggle();
        } else PORTL &= ~(1 << TurnSignalLEDRight);
        if(TreeSignalLeft) {
                LedTreeIndictorLeftToggle();
        } else PORTB &= ~(1 << TreeIndicatedLEDLeft);
        if(TreeSignalRight) {
                LedTreeIndictorRightToggle();
        } else PORTB &= ~(1 << TreeIndicatedLEDRight);
        counter = 0;
    }
}


void init_delay_led(){
    // Use mode 0, clkdiv = 64
    TCCR2A = 0;
    TCCR2B = (0<<CS22) | (1<<CS21) | (1<<CS20);

    // Disable PWM output
    OCR2A = 0;
    OCR2B = 0;

    //Overflow interrupt
    TIMSK2 = (1<<TOIE2);
    TCNT2 = 6;
}
