/*

    Code project AGV.
    2024

 */

#include <avr/io.h>
#include <util/delay.h>
#include "SteppermotorAVRDriver.h"
#include "Ultrasone_sensor.h"

#define distanceToCheck 50
#define distanceToFollow 12
#define dAccuracy 2
#define TreeDistance 10


#define minDistance (distanceToFollow - dAccuracy)
#define maxDistance (distanceToFollow + dAccuracy)

#define ModeOff 0
#define Following 1
#define BoomgaardRijden 2
#define BoomgaardBocht 3
#define Noodstop 4


//{ Pins
//IR
#define FrontIRSensorLeftPin PA0 //pin 22
#define FrontIRSensorRightPin PA1 //pin 23
#define IRSensorLeft PA2 //pin 24
#define IRSensorRight PA3 //pin 25

//LEDS
#define TreeIndicatedLEDLeft PL0 //pin 49
#define TreeIndicatedLEDRight PL1 //pin 48
#define TurnSignalLEDLeft PL2 //pin 47
#define TurnSignalLEDRight PL3 //pin 46
#define Breaklights PL4 //pin 45
#define Headlights PL5 //pin 44
#define NoodstopLEDFront PL6 //pin 43
#define NoodstopLEDBack PL7 //pin 42

//Buttons
#define StartButtonPin PC0 //pin 37
#define FollowModeSwitch PC1 //pin 36
#define DriveModeSwitch PC2 //pin 35
#define NoodstopPinFront PC3 //pin 34
#define NoodstopPinBack PC4 //pin 33
//}

//{ Function list
int checkNoodstop();
int isStartButtonPressed();
int checkModeSwitchState();
void initIRSensors();
void initLEDS();
void initButtons();
int checkFrontIRState();
void followHand();
int filterDistance();
int checkSensors();
//}


void initAGV(){
    agv_ultrasoon_init();
    initSteppermotorAVRDriver();
    initIRSensors();
    initLEDS();
    initButtons();
}

int main(void)
{
    int mode = BoomgaardRijden;
    int FrontDistance;
    initAGV();

    // Insert code

    while(1){

        //Check voor de noodstop.
        if(checkNoodstop()){
            mode = Noodstop;
        }

        //Check voor de uit knop (in dit geval telt de start knop ook als stop knop als de AVG ergens mee bezig is)
        if(mode != Noodstop && mode != ModeOff && isStartButtonPressed()){
            mode = ModeOff;
        }

        switch(mode){

            //Case voor uit.
            case ModeOff:
                setBothStepperMode(Off);

                if(isStartButtonPressed()){
                    int switchState = checkModeSwitchState();
                    switch(switchState){
                    case 1:
                        mode = Following;
                        break;
                    case 2:
                        mode = BoomgaardRijden;
                        break;
                    }
                }

                break;

            //Case voor noodstop
            case Noodstop:
                setBothStepperMode(Off);

                //Noodstop niet meer ingedrukt, ga naar de start modus.
                if(!checkNoodstop()) {
                    mode = ModeOff;
                }
                break;

            //Case voor hand volgen
            case Following:
                FrontDistance = agv_ultrasoon_voor_midden;
                int IRState = checkFrontIRState();

                switch(IRState){
                    case 0: // Allebij de sensors aan dus zet motors uit
                        setBothStepperMode(Off);
                        break;
                    case 1: //Linker IR Sensor activated
                        setStepperMode(rightMotor, BackwardStep);
                        setStepperMode(leftMotor, Off);
                        break;
                    case 2: //Rechter IR Sensor activated
                        setStepperMode(leftMotor, BackwardStep);
                        setStepperMode(rightMotor, Off);
                        break;
                    case 3: //Geen IR sensor activated
                        followHand(filterDistance(FrontDistance));
                        break;
                }
                break;

            //Case voor door de boomgaard rijden
            case BoomgaardRijden:

                int WorldState = checkSensors();
                switch(WorldState){
                    case 0: //Object in front of AGV
                        setBothStepperMode(Off);
                        break;
                    case 1: //Tree left
                        setBothStepperMode(Off);
                        _delay_ms(1000);
                        break;
                    case 2: //Tree right
                        setBothStepperMode(Off);
                        _delay_ms(1000);
                        break;
                    case 3: //Nothing, keep driving
                        setBothStepperMode(ForwardStep);
                        break;
                }

                break;

            //Case voor bochten maken
            case BoomgaardBocht:
                setStepperMode(leftMotor, ForwardStep);
                setStepperMode(rightMotor, BackwardStep);
                break;
        }


    }

    return 0;
}

int checkNoodstop(){
    if(bit_is_clear(PINC, NoodstopPinBack) || bit_is_clear(PINC, NoodstopPinFront)){
        return 1;
    }
    return 0;
}

int isStartButtonPressed(){
    return bit_is_clear(PINC, StartButtonPin);
}

int checkModeSwitchState(){
    if(bit_is_clear(PINC, FollowModeSwitch)){
        return 1; //Switch is in follow mode.
    }
    if(bit_is_clear(PINC, DriveModeSwitch)){
        return 2; //Switch is in drive mode.
    }

    return 0; //Dit zou niet moeten kunnen.

}

void initButtons(){
    for(int i = 0; i < 5; i++){
        DDRC &= ~(1<<i);
        PORTC |= (1<<i);
    }
}

void initLEDS(){
    for(int i = 0; i < 8; i++){
        PORTL |= (1<<i);
    }
}

void initIRSensors(){
    for(int i = 0; i < 4; i++){
        DDRA &= ~(1<<i);
        PORTA |= (1<<i);
    }
}

int checkFrontIRState(){
    if(bit_is_clear(PINA, FrontIRSensorLeftPin) && bit_is_clear(PINA, FrontIRSensorRightPin)){
        return 0;
    }

    if(bit_is_clear(PINA, FrontIRSensorLeftPin)){
        return 1;
    }

    if(bit_is_clear(PINA, FrontIRSensorRightPin)){
        return 2;
    }

    return 3;
}

int checkSensors(){
    static int leftPreviousState = 0;
    static int rightPreviousState = 0;

    if(maxDistance > filterDistance(agv_ultrasoon_voor_midden)){
        return 0;
    }

    if((TreeDistance > filterDistance(agv_ultrasoon_boom_links)) && !leftPreviousState){
        leftPreviousState = 1;
        return 1;
    } else if(leftPreviousState && (TreeDistance < filterDistance(agv_ultrasoon_boom_links)) ){
        leftPreviousState = 0;
    }

    if((TreeDistance > filterDistance(agv_ultrasoon_boom_rechts)) && !rightPreviousState){
        rightPreviousState = 1;
        return 2;
    } else if(rightPreviousState && (TreeDistance < filterDistance(agv_ultrasoon_boom_rechts)) ){
        rightPreviousState = 0;
    }

    return 3;

}

int filterDistance(int distance){
    //Alle waardes boven 200 zijn bs anyways
    if(distance == 561){
        distance = 1;
    } else if(distance > 500){
        distance = 100;
    }
    return distance;
}

void followHand(int distance){

    //Check voor als er iets TE ver weg staat en te negeren.
    if(distance > distanceToCheck){
        setBothStepperMode(Off);
        return;
    }

    //Check voor juiste afstand
    if((distance < maxDistance) && (distance > minDistance)){
            setBothStepperMode(Off);
            return;
    }

    //Check voor dichtbij
    if(distance < minDistance){
        setBothStepperMode(BackwardStep);
        return;
    }

    //check voor verweg
    if(distance > maxDistance){
        setBothStepperMode(ForwardStep);
    }

}

void init_delay_timer(){
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

