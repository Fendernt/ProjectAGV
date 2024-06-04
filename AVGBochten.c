//AVGBochten.c

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "AGVBochten.h"
#include "SteppermotorAVRDriver.h"

/*
    Werking:

    Zet timer aan en rij vooruit
    compA om de bocht te beginnen
    CompB om de bocht te eindigen

    reset timer en rij vooruit
    CompA voor begin bohct
    CompB voor eind bocht

    rij vooruit

    halve draai gemaakt
*/

//1 sec = 30
#define second 30
#define Comp_StartTurn (second*3)
#define Comp_EndTurn (Comp_StartTurn + (second * 3.9))
#define DrivingHeadstart -50

#define Left 0
#define Right 1

//Counter
volatile int AVGBochtenCounter = 0;


int turnStarted = 0; //Check voor als we in een bocht zitten
int turnsTaken = 0; //Is er om volgorde bij te houden.

//Direction om bocht te maken
int turnDirection = 0;

//Init
void initAGVBochten(){
    init_AGVBochten_timer();
}


//Start turn
int startTurn(int direction){
    if(turnStarted == 1) { //Check of we niet al een bocht aan het maken zijn
        return checkIfFinished();
    }


    turnStarted = 1;
    turnDirection = direction;
    startTimer(); //Start de bocht
}

//Set timer aan
void startTimer(){
    AVGBochtenCounter = 0;
    TCNT4 = 0;
    TCCR4B = (0<<CS42) | (1<<CS41) | (0<<CS40);
}

//Stop timer
void stopTimer(){
     TCCR4B = (0<<CS42) | (0<<CS41) | (0<<CS40);
     turnsTaken++;
}

//Check of de bocht af is
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
    AVGBochtenCounter++;

    //Begin bocht
    if(AVGBochtenCounter == Comp_StartTurn){
        if(turnsTaken == 2) { //Als we al 2 bochten hebben gemaakt stop de timer
            stopTimer();
        } else turn(turnDirection); //Zoniet, maak de bocht
    }

    //Stop de bocht
    if(AVGBochtenCounter == Comp_EndTurn){
        setBothStepperMode(ForwardStep);
        if(turnsTaken == 0) AVGBochtenCounter = DrivingHeadstart; //Als dit het einde is van de eerste bocht, reset de timer om het te herhalen
        turnsTaken++; //increase aantal bochten
    }
}

//Zet de wielen goed voor de bocht
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
