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

#define distanceToCheck 20 //Afstand om te controlleren voor een hand (om te volgen)
#define distanceToFollow 6 //Afstand om op te blijven als iemand word gevolgt
#define dAccuracy 2 //Speling in de afstand van volgen (distanceToFollow +- dAccuracy)
#define TreeDistance 5 //Afstand om te controlleren voor bomen

#define CheckinFrontOfAVRWhileDriving 20 //Afstand om voor te stoppen als er iets op de weg is

#define TimeToWaitBetweenTrees 1200 //Tijd om te wachten tussen bomen


#define minDistance (distanceToFollow - dAccuracy)
#define maxDistance (distanceToFollow + dAccuracy)

#define ModeOff 0
#define Following 1
#define BoomgaardRijden 2
#define BoomgaardBocht 3
#define Noodstop 4

#define Left 0
#define Right 1


//{ Pins
//IR
#define IRPIN PINA
#define FrontIRSensorLeftPin PA0 //pin 22
#define FrontIRSensorRightPin PA1 //pin 23
#define IRSensorLeft PA2 //pin 24
#define IRSensorRight PA3 //pin 25

//Buttons
#define StartButtonPin PC0 //pin 37
#define FollowModeSwitch PC2 //pin 35
#define DriveModeSwitch PC1 //pin 36
#define NoodstopPin PC3 //pin 34
//}


/*
    Defines voor het bepalen wat wel en niet word gedaan
    tijdens het rijden in een pad
*/
//---------------------------------------
#define UseDrivingCorrection //Gebruik padcorrectie tijdens het rijden van het pad

#define UseUltrasone //Gebruik de ultrasone tijdens het rijden in het pad

#define UltrasoneDoublechecking //Maak meerdere scans van de ultrasone voor het bepalen van een object
#define doubleCheckTimeDelay 130
#define amountOfDoubleChecks 2
#define TimeBetweenRechecking 200

#define UltrasoneUseValueComparison //Vergelijkt vorige 3 waardes van de ultrasone met elkaar

#define MaakBochtNaPad //Maak de bocht na het einde van het pad
//---------------------------------------


int doorEerstePadGereden = 0;
int var_inEenPad = 0;

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

    initDisplay();
}

int main(void)
{
    //Variables
    int mode = ModeOff; //Active mode van de AGV
    int FrontDistance; //Var voor de afstand vam objecten voor de AGV
    initAGV(); //Init

    display(0);

    //Zet koplampen aan
    setHeadlights(1);

    while(1){


        display(agv_ultrasoon_boom_rechts);
        //display(agv_ultrasoon_boom_links);

        //Check voor de noodstop.
        if(checkNoodstop()){
            mode = Noodstop;
        }

        //Check voor de uit knop (in dit geval telt de start knop ook als stop knop als de AVG ergens mee bezig is)
        /*
        if(mode != Noodstop && mode != ModeOff && isStartButtonPressed()){
            mode = ModeOff;
        }
        */

        switch(mode){

            //Case voor uit.
            case ModeOff:
                setBothStepperMode(Off);

                LedNoodstopBack(0);
                LedNoodstopFront(0);

                TurnSignalLeft = 0;
                TurnSignalRight = 0;
                TreeSignalLeft = 0;
                TreeSignalRight = 0;
                setBreaklights(0);

                //Als de startbutton is ingedrukt, kijk in welke mode de modeswitch is, en activeer die mode.
                if(isStartButtonPressed()){
                    int switchState = checkModeSwitchState();
                    switch(switchState){
                    case 1: //Volg mode
                        mode = Following;
                        _delay_ms(10); //debounce
                        break;
                    case 2: //Rij mode
                        mode = BoomgaardRijden;
                        _delay_ms(10); //debounce
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
                LedNoodstopBack(1);
                LedNoodstopFront(1);
                TurnSignalLeft = 1;
                TurnSignalRight = 1;
                TreeSignalLeft = 1;
                TreeSignalRight = 1;
                setBreaklights(1);
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
                        TurnSignalRight = 0;
                        setStepperMode(rightMotor, BackwardStep);
                        setStepperMode(leftMotor, ForwardStep);
                        break;
                    case 2: //Rechter IR Sensor activated
                        TurnSignalRight = 1;
                        TurnSignalLeft = 0;
                        setStepperMode(leftMotor, BackwardStep);
                        setStepperMode(rightMotor, ForwardStep);
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
                int WorldState = checkSensors();
                static int alBochtGemaakt = 0;
                static int previousWorldState = -1;
                if(previousWorldState != WorldState){
                    previousWorldState = WorldState;
                    // if(WorldState == 3) _delay_ms(TimeBetweenRechecking);
                }

                #ifdef MaakBochtNaPad
                if(nietInEenPad() && !alBochtGemaakt){
                    _delay_ms(50);
                    if(nietInEenPad() && !alBochtGemaakt){
                        alBochtGemaakt = 1;
                        mode = BoomgaardBocht;
                    }
                } else if(nietInEenPad()){
                    _delay_ms(200);
                    if(nietInEenPad()){
                        mode = ModeOff;
                    }
                }
                #endif // MaakBochtNaPad

                switch(WorldState){
                    case 0: //Object in front of AGV
                        setBothStepperMode(Off);
                        setBreaklights(1);
                        break;
                    case 1: //Tree left
                        setBothStepperMode(Off);
                        TreeSignalLeft = 1;
                        setBreaklights(1);
                        _delay_ms(TimeToWaitBetweenTrees);
                        TreeSignalLeft = 0;
                         break;
                    case 2: //Tree right
                        setBothStepperMode(Off);
                        TreeSignalRight = 1;
                        setBreaklights(1);
                        _delay_ms(TimeToWaitBetweenTrees);
                        TreeSignalRight = 0;
                        break;
                    case 3: //Nothing, keep driving
                        #ifdef UseDrivingCorrection
                        int correction = needCorrection();
                        setBreaklights(0);
                        switch(correction){
                            case 0: //Geen afwijking
                                setBothStepperMode(ForwardStep);
                                break;
                            case 1: //Afwijking naar Rechts
                                setStepperMode(rightMotor, Off);
                                setStepperMode(leftMotor, ForwardStep);
                                break;
                            case 2: //Afwijking naar Links
                                setStepperMode(leftMotor, Off);
                                setStepperMode(rightMotor, ForwardStep);
                                break;
                            case 3: //Allebij uit
                                TreeSignalLeft = 1;
                                TreeSignalRight = 1;
                                TurnSignalLeft = 1;
                                TurnSignalRight = 1;
                                setBothStepperMode(Off);
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
                static int bochtGemaakt = 0;
                static int direction = 0;

                if(startTurn(direction) && (bochtGemaakt == 0)){
                    bochtGemaakt = 1;
                }

                if(bochtGemaakt){
                    if(bit_is_clear(IRPIN, IRSensorLeft) || bit_is_clear(IRPIN, IRSensorRight)){
                        bochtGemaakt = 0;
                        _delay_ms(300);
                        mode = BoomgaardRijden;
                    }
                }

                break;
        }


    }

    return 0;
}

int nietInEenPad(){
    return ((!bit_is_clear(IRPIN, IRSensorLeft)) && !bit_is_clear(IRPIN, IRSensorRight));
}

/*
    Functie om te kijken of we van het pad aan het afdwalen zijn.
    return codes:
    0-Geen afwijking
    1-Afwijking naar Rechts
    2-Afwijking naar links
    3-Allebij uit? wtf? help? pls wtf help??!!?!??!?!!?
*/

#define testDelay 0
int needCorrection(){

    //Linker bit is niet geactiveerd, we hebben een afwijking naar Rechts //Bijsturen naar links
    if(!bit_is_clear(IRPIN, IRSensorLeft)){
        _delay_ms(testDelay);
        return 1;
    }
    //Rechter bit is niet geactiveerd, we hebben een afwijking naar Links //bijsturen naar rechts
    if(!bit_is_clear(IRPIN, IRSensorRight)){
        _delay_ms(testDelay);
        return 2;
    }

    return 0;
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
    //return 3; //for testing without IR sensors

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
    return 3; //Geen IR sensor's geactiveerd
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
    static int doubleCheckLeft = 0;
    static int doubleCheckRight = 0;

    #ifndef UltrasoneUseValueComparison
    int valueLeft = filterDistance(agv_ultrasoon_boom_links);
    int valueRight = filterDistance(agv_ultrasoon_boom_rechts);
    #else
    int valueLeft = getLeftSensorValue();
    int valueRight = getRightSensorValue();
    #endif

    #ifndef UseUltrasone
    return 3;
    #endif // UseUltrasone

    //Kijken of er iets voor de AGV staat.
    if(CheckinFrontOfAVRWhileDriving > filterDistance(agv_ultrasoon_voor_midden)){
        return 0;
    }

    //Kijken of er iets voor de AGV staat, en er nog niks is gemeten
    if((TreeDistance > valueLeft) && !leftPreviousState){
        //Variable zetten om te onthouden dat deze al is gemeten.
        #ifdef UltrasoneDoublechecking
        if(doubleCheckLeft < amountOfDoubleChecks){
            doubleCheckLeft++;
            _delay_ms(doubleCheckTimeDelay);
            return 3;
        } else {
            doubleCheckLeft = 0;
            leftPreviousState = 1;
            //display(1000);
            return 1;
        }
        #else
        leftPreviousState = 1;
        return 1;
        #endif // UltrasoneDoublechecking

    } else if(leftPreviousState && (TreeDistance < valueLeft) ){
        //Er word geen boom meer gemeten dus we zijn er voorbij gereden, variable weer uitzetten om de te zoeken naar de volgende boom.
        //_delay_ms(100);
        if((doubleCheckLeft < amountOfDoubleChecks)){
            doubleCheckLeft++;
            _delay_ms(doubleCheckTimeDelay);
        } else {
            leftPreviousState = 0;
            doubleCheckLeft = 0;
            //display(2000);
            _delay_ms(TimeBetweenRechecking);
        }
    }

    //Werkt hetzelfde als hierboven maar dan voor de rechterkant van de AGV
    if((TreeDistance > valueRight) && !rightPreviousState ){
        #ifdef UltrasoneDoublechecking
        if((doubleCheckRight < amountOfDoubleChecks)){
            doubleCheckRight++;
            _delay_ms(doubleCheckTimeDelay);
            return 3;
        } else {
            doubleCheckRight = 0;
            rightPreviousState = 1;
            //display(10);
            return 2;
        }
        #else
        rightPreviousState = 1;
        return 2;
        #endif // UltrasoneDoublechecking

    } else if(rightPreviousState && (TreeDistance < valueRight) ){
        //_delay_ms(100);
        if((doubleCheckRight < amountOfDoubleChecks)){
            doubleCheckRight++;
            _delay_ms(doubleCheckTimeDelay);
        } else {
            rightPreviousState = 0;
            doubleCheckRight = 0;
            //display(20);
            _delay_ms(TimeBetweenRechecking);
        }
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


//Code voor het volgen van de hand op de juiste afstand
void followHand(int distance){

    //Check voor als er iets TE ver weg staat en te negeren.
    if(distance > distanceToCheck){
        setBothStepperMode(Off);
        setBreaklights(0);
        return;
    }

    //Check voor juiste afstand met speelruimte
    if((distance < maxDistance) && (distance > minDistance)){
            setBothStepperMode(Off);
            setBreaklights(0);
            return;
    }

    //Check voor dichtbij
    if(distance < minDistance){
        setBothStepperMode(BackwardStep);
        setBreaklights(1);
        return;
    }

    //check voor verweg
    if(distance > maxDistance){
        setBothStepperMode(ForwardStep);
        setBreaklights(0);
    }

}


