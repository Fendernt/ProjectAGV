/*

    Code project AGV.
    2024

 */

#include <avr/io.h>
#include <util/delay.h>
#include "SteppermotorAVRDriver.h"
#include "Ultrasone_sensor.h"
#include "AGVBochten.h"
#include "AGV_Leds.h"

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
#define IRPIN PINA
#define FrontIRSensorLeftPin PA0 //pin 22
#define FrontIRSensorRightPin PA1 //pin 23
#define IRSensorLeft PA2 //pin 24
#define IRSensorRight PA3 //pin 25

//Buttons
#define StartButtonPin PC0 //pin 37
#define FollowModeSwitch PC1 //pin 36
#define DriveModeSwitch PC2 //pin 35
#define NoodstopPin PC3 //pin 34
//}


//Define voor als we wel of niet correctie willen gebruiken tijdens
//het rijden van de AGV
//#define UseDrivingCorrection

//{ Function list
int checkNoodstop();
int isStartButtonPressed();
int checkModeSwitchState();
void initIRSensors();
void initButtons();
int checkFrontIRState();
void followHand();
int filterDistance();
int checkSensors();
//}


//Inits
void initAGV(){
    init_Leds();
    agv_ultrasoon_init();
    initSteppermotorAVRDriver();
    initIRSensors();
    initButtons();
    initAGVBochten();
}

int main(void)
{
    //Variables
    int mode = ModeOff; //Active mode van de AGV
    int FrontDistance; //Var voor de afstand vam objecten voor de AGV
    initAGV(); //Init

    setBothStepperMode(ForwardStep);

    //Zet koplampen aan
    setHeadlights(1);

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

                //Als de startbutton is ingedrukt, kijk in welke mode de modeswitch is, en activeer die mode.
                if(isStartButtonPressed()){
                    int switchState = checkModeSwitchState();
                    switch(switchState){
                    case 1: //Volg mode
                        mode = Following;
                        break;
                    case 2: //Rij mode
                        mode = BoomgaardRijden;
                        break;
                    case 0: //Switch staat in het midden
                        //do nothin lol
                        break;
                    }
                }

                break;

            //Case voor noodstop
            case Noodstop:
                setBothStepperMode(Off);

                //Noodstop niet meer ingedrukt, ga naar de start modus.
                //Start mode zodat opnieuw de startknop moet worden ingedrukt.
                if(!checkNoodstop()) {
                    mode = ModeOff;
                }
                break;

            //Case voor hand volgen
            case Following:
                //Check de voorafstand
                FrontDistance = agv_ultrasoon_voor_midden;
                int IRState = checkFrontIRState();

                switch(IRState){
                    case 0: // Allebij de sensors aan dus zet motors uit
                        setBothStepperMode(Off);
                        TurnSignalLeft = 1;
                        TurnSignalRight = 1;
                        break;
                    case 1: //Linker IR Sensor activated
                        TurnSignalLeft = 1;
                        setStepperMode(rightMotor, BackwardStep);
                        setStepperMode(leftMotor, Off);
                        break;
                    case 2: //Rechter IR Sensor activated
                        TurnSignalRight = 1;
                        setStepperMode(leftMotor, BackwardStep);
                        setStepperMode(rightMotor, Off);
                        break;
                    case 3: //Geen IR sensor activated
                        followHand(filterDistance(FrontDistance));
                        TurnSignalLeft = 0;
                        TurnSignalRight = 0;
                        break;
                }
                break;

            //Case voor door de boomgaard rijden
            case BoomgaardRijden:
                #ifdef UseDrivingCorrection
                if(!bit_is_clear(IRPIN, IRSensorLeft) && !bit_is_clear(IRPIN, IRSensorRight)){
                    //We zijn nog in de boomgaard aan het rijden dus wacht totdat we erin zitten.
                    break;
                }
                #endif

                int WorldState = checkSensors();
                switch(WorldState){
                    case 0: //Object in front of AGV
                        setBothStepperMode(Off);
                        setBreaklights(1);
                        break;
                    case 1: //Tree left
                        setBothStepperMode(Off);
                        TreeSignalLeft = 1;
                        setBreaklights(1);
                        _delay_ms(1000);
                        TreeSignalLeft = 0;
                        break;
                    case 2: //Tree right
                        setBothStepperMode(Off);
                        TreeSignalRight = 1;
                        setBreaklights(1);
                        _delay_ms(1000);
                        TreeSignalRight = 0;
                        break;
                    case 3: //Nothing, keep driving
                        #ifdef UseDrivingCorrection
                        int correction = needCorrection();
                        switch(correction){
                            case 0: //Geen afwijking
                                setBothStepperMode(ForwardStep);
                                break;
                            case 1: //Afwijking naar Rechts
                                setStepperMode(leftMotor, BackwardStep);
                                setStepperMode(rightMotor, Off);
                                break;
                            case 2: //Afwijking naar Links
                                setStepperMode(rightMotor, BackwardStep);
                                setStepperMode(leftMotor, Off);
                                break;
                        }
                        #else
                        setBothStepperMode(ForwardStep);
                        setBreaklights(0);
                        #endif
                        break;
                }

                break;

            //Case voor bochten maken
            case BoomgaardBocht:
                static int direction = 0;

                if(startTurn(direction)){
                    direction = !direction;
                };

                break;
        }


    }

    return 0;
}

/*
    Functie om te kijken of we van het pad aan het afdwalen zijn.
    return codes:
    0-Geen afwijking
    1-Afwijking naar Rechts
    2-Afwijking naar links
*/
int needCorrection(){
    int returnValue = 0;
    //Linker bit is niet geactiveerd, we hebben een afwijking naar Rechts
    if(!bit_is_clear(PINA, IRSensorLeft)){
        returnValue = 1;
    }
    //Rechter bit is niet geactiveerd, we hebben een afwijking naar Links
    if(!bit_is_clear(PINA, IRSensorRight)){
        returnValue = 2;
    }

    return returnValue;
}

//Check of noodstop is ingedrukt
int checkNoodstop(){
    if(bit_is_clear(PINC, NoodstopPin)){
        return 1;
    }
    return 0;
}

//Check of start knop is ingedrukt
int isStartButtonPressed(){
    return bit_is_clear(PINC, StartButtonPin);
}

/*Check in welke positie de mode switch staat.
    Return codes:
    0- Switch staat in het midden
    1- Volgen
    2- Rijden
*/
int checkModeSwitchState(){
    if(bit_is_clear(PINC, FollowModeSwitch)){
        return 1; //Switch is in follow mode.
    }
    if(bit_is_clear(PINC, DriveModeSwitch)){
        return 2; //Switch is in drive mode.
    }

    return 0; //Switch staat in het midden, geen van beide geactiveerd.

}

//Init buttons
void initButtons(){
    //Loop voor alle buttons
    for(int i = 0; i < 5; i++){
        DDRC &= ~(1<<i);
        PORTC |= (1<<i);
    }
}

//Init IR sensors
void initIRSensors(){
    //Loop voor alle IR sensors
    for(int i = 0; i < 4; i++){
        DDRA &= ~(1<<i);
        PORTA |= (1<<i);
    }
}

/*Check de IR sensors aan de voorkant
    Return codes:
    0- Allebij
    1- Links
    2- Rechts
    3- Geen
*/
int checkFrontIRState(){
    //Allebij detecteren iets, return 0
    if(bit_is_clear(PINA, FrontIRSensorLeftPin) && bit_is_clear(PINA, FrontIRSensorRightPin)){
        return 0;
    }

    //Linker sensor detecteerd iets, return 1
    if(bit_is_clear(PINA, FrontIRSensorLeftPin)){
        return 1;
    }

    //Rechter sensor detecteert iets, return 2
    if(bit_is_clear(PINA, FrontIRSensorRightPin)){
        return 2;
    }

    //Return 3
    return 3;
}


/*
    Check de waarde van de sonic sensors voor het controlleren van bomen.
    Return codes:
    0-Er staat iets voor de AGV
    1-Er is een boom links van de AGV
    2-Er is een boom rechts van de AGV
    3-Er is niks gemeten
*/
int checkSensors(){
    //Variable om te kijken of er al iets is gemeten
    static int leftPreviousState = 0;
    static int rightPreviousState = 0;

    //Kijken of er iets voor de AGV staat.
    if(maxDistance > filterDistance(agv_ultrasoon_voor_midden)){
        return 0;
    }

    //Kijken of er iets voor de AGV staat, en er nog niks is gemeten
    if((TreeDistance > filterDistance(agv_ultrasoon_boom_links)) && !leftPreviousState){
        //Variable zetten om te onthouden dat deze al is gemeten.
        leftPreviousState = 1;
        return 1;
    } else if(leftPreviousState && (TreeDistance < filterDistance(agv_ultrasoon_boom_links)) ){
        //Er word geen boom meer gemeten dus we zijn er voorbij gereden, variable weer uitzetten om de te zoeken naar de volgende boom.
        leftPreviousState = 0;
    }

    //Werkt hetzelfde als hierboven maar dan voor de rechterkant van de AGV
    if((TreeDistance > filterDistance(agv_ultrasoon_boom_rechts)) && !rightPreviousState){
        rightPreviousState = 1;
        return 2;
    } else if(rightPreviousState && (TreeDistance < filterDistance(agv_ultrasoon_boom_rechts)) ){
        rightPreviousState = 0;
    }

    //Er is niks gemeten
    return 3;

}

/*
    Filter de waardes van de sonic sensors

    Als er iets te dichtbij de sonic sensor is word er 561 gegeven,
    verander dit naar 1 zodat de rest van de code weet dat er iets dichtbij staat.

    Als er iets te ver weg van de sonic sensor is de waarde tussen de 500 & 660,
    dus verander dit naar een groot waarde, in dit geval 100.

    Anders return de originele waarde.
*/
int filterDistance(int distance){
    //Alle waardes boven 200 zijn bs anyways
    if(distance == 561){
        distance = 1;
    } else if(distance > 500){
        distance = 100;
    }
    return distance;
}

//Code voor het volgen van de hand op de juiste afstand
void followHand(int distance){

    //Check voor als er iets TE ver weg staat en te negeren.
    if(distance > distanceToCheck){
        setBothStepperMode(Off);
        return;
    }

    //Check voor juiste afstand met speelruimte
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


