//AVGBochten.c

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "AGVBochten.h"
#include "SteppermotorAVRDriver.h"

//1 sec = 30
#define second 30
#define Comp_StartTurn (second*3)
#define Comp_EndTurn (Comp_StartTurn + (second * 3.9))
#define DrivingHeadstart -50

#define Left 0
#define Right 1


volatile int counter = 0;


int turnStarted = 0;
int turnsTaken = 0;

int turnDirection = 0;


void initAGVBochten(){
    init_AGVBochten_timer();
}


int startTurn(int direction){
    if(turnStarted == 1) {
        return checkIfFinished();
    }


    turnStarted = 1;
    turnDirection = direction;
    startTimer();
}

void startTimer(){
    counter = 0;
    TCNT4 = 0;
    TCCR4B = (0<<CS42) | (1<<CS41) | (0<<CS40);
}

void stopTimer(){
     TCCR4B = (0<<CS42) | (0<<CS41) | (0<<CS40);
     turnsTaken++;
}

void checkIfFinished(){
    if(turnsTaken >= 3){
        turnsTaken = 0;
        turnStarted = 0;
        return 1;
    } else return 0;
}

void init_AGVBochten_timer(){
    // Use timer 4 (16bit), clkdiv = 0, 16.000.000 / (2^16) / 8 = 30(.5175
    TCCR4A = 0;
    //TCCR4B = (0<<CS42) | (1<<CS41) | (0<<CS40);

    // Disable PWM output
    OCR4A = 0;
    OCR4B = 0;

    //Compares

    //Overflow interrupt
    TIMSK4 = (1<<TOIE4);
    //TCNT4 = 6;
}

ISR(TIMER4_OVF_vect){
    counter++;

    if(counter == Comp_StartTurn){
        if(turnsTaken == 2) {
            stopTimer();
        } else turn(turnDirection);
    }

    if(counter == Comp_EndTurn){
        setBothStepperMode(ForwardStep);
        if(turnsTaken == 0) counter = DrivingHeadstart;
        turnsTaken++;
    }
}

void turn(int direction){
    switch(direction) {
    case Left:
        setStepperMode(leftMotor, Off);
        setStepperMode(rightMotor, ForwardStep);
        break;
    case Right:
        setStepperMode(leftMotor, ForwardStep);
        setStepperMode(rightMotor, Off);
        break;
    }
}
