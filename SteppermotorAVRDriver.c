/*
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
#include "SteppermotorAVRDriver.h"



//Motor loop's
#define LoopLength 8
int LoopForward[LoopLength] = {
    0b0001, // IN1
    0b0011, // IN1 + IN2
    0b0010, // IN2
    0b0110, // IN2 + IN3
    0b0100, // IN3
    0b1100, // IN3 + IN4
    0b1000, // IN4
    0b1001  // IN4 + IN1
};
int LoopBackwards[LoopLength] = {
    0b1001, // IN4 + IN1
    0b1000, // IN4
    0b1100, // IN3 + IN4
    0b0100, // IN3
    0b0110, // IN2 + IN3
    0b0010, // IN2
    0b0011, // IN1 + IN2
    0b0001  // IN1
};
int (*leftMotorLoop)[LoopLength] = &LoopForward;
int (*rightMotorLoop)[LoopLength] = &LoopForward;



int LeftStepperSpeed = 0;
int RightStepperSpeed = 0;
int leftMotorEnabled = 1;
int rightMotorEnabled = 1;


void setBothStepperMode(int mode){
    setStepperMode(leftMotor, mode);
    setStepperMode(rightMotor, mode);
}

void setStepperMode(int motor, int mode){

    int state;
    if(mode == Off){
        state = 0;
    } else {
        state = 1;
    }


    int** motorLoop;
    switch(motor){
            case leftMotor:
            motorLoop = &leftMotorLoop;
            leftMotorEnabled = state;
            break;
        case rightMotor:
            motorLoop = &rightMotorLoop;
            rightMotorEnabled = state;
            break;
    }

    switch(mode){
        default:
        case ForwardStep:
            *motorLoop = &LoopForward;
            break;
        case BackwardStep:
            *motorLoop = &LoopBackwards;
            break;
    }
}

//Clockspeed = 16.000.000 / 250 / 64 = 1.000
#define Clockspeed 1000
void init_timer(){
    // Use mode 0, clkdiv = 64
    TCCR0A = 0;
    TCCR0B = (0<<CS02) | (1<<CS01) | (1<<CS00);

    // Disable PWM output
    OCR0A = 0;
    OCR0B = 0;

    //Overflow interrupt
    TIMSK0 = (1<<TOIE0);
    TCNT0 = 6;

    sei();
}

void initSteppermotorAVRDriver(){
    initMotorpins();
    init_timer();
    setBothStepperMode(Off);


}


void initMotorpins(){
    for(int i = MotorLeftStartpin; i < MotorLeftStartpin+4; i++){
        MotorLeftDDR |= (1<<i);
    }

    for(int i = MotorRightStartpin; i < MotorRightStartpin+4; i++){
        MotorRightDDR |= (1<<i);
    }
}

void LeftStepperNextStep(){
    static int position = 0;
    MotorLeftPort &= ~(0b1111<< MotorLeftStartpin);
    MotorLeftPort |= ((*leftMotorLoop)[position] << MotorLeftStartpin);
    position++;
    if(position == LoopLength) position = 0;
}

void RightStepperNextStep(){
    static int position = 0;
    MotorRightPort &= ~(0b1111<<MotorRightStartpin);
    MotorRightPort |= ((*rightMotorLoop)[position] << MotorRightStartpin);
    position++;
    if(position == LoopLength) position = 0;
}


ISR(TIMER0_OVF_vect){
    if(leftMotorEnabled) LeftStepperNextStep();
    if(rightMotorEnabled) RightStepperNextStep();

    TCNT0 = 6;
}
