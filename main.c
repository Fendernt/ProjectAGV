/*

    Code project AGV.
    2024

 */

#include <avr/io.h>
#include <util/delay.h>
#include "SteppermotorAVRDriver.h"
#include "Ultrasone_sensor.h"

#define distanceToFollow 20
#define dAccuracy 3
#define TreeDistance 20


#define minDistance (distanceToFollow - dAccuracy)
#define maxDistance (distanceToFollow + dAccuracy)

#define ModeOff 0
#define Following 1
#define BoomgaardRijden 2
#define BoomgaardBocht 3


void initAGV(){
    agv_ultrasoon_init();
    initSteppermotorAVRDriver();
}

int main(void)
{
    int mode = BoomgaardRijden;
    int FrontDistance;
    initAGV();

    // Insert code

    while(1){


        switch(mode){
            case ModeOff:
                setBothStepperMode(Off);
                break;

            case Following:
                FrontDistance = agv_ultrasoon_voor_midden;
                followHand(filterDistance(FrontDistance));
                break;

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

            case BoomgaardBocht:

                break;
        }


    }

    return 0;
}


int checkSensors(){
    static int leftPreviousState = 0;
    static int rightPreviousState = 0;

    /*if(maxDistance > filterDistance(agv_ultrasoon_voor_midden)){
        return 0;
    }*/

    if((TreeDistance > filterDistance(agv_ultrasoon_boom_links)) && !leftPreviousState){
        leftPreviousState = 1;
        return 1;
    } else if(leftPreviousState && (TreeDistance < filterDistance(agv_ultrasoon_boom_links)) ){
        leftPreviousState = 0;
    }

    //if(TreeDistance > filterDistance(agv_ultrasoon_boom_rechts)){
      //  return 2;
    //}

    return 3;

}

int filterDistance(int distance){
    //Alle waardes boven 200 zijn bs anyways
    if(distance > 200){
        distance = 1;
    }
    return distance;
}

void followHand(int distance){
    if((distance < maxDistance) && (distance > minDistance)){
            setBothStepperMode(Off);
            return;
    }

    if(distance < minDistance){
        setBothStepperMode(BackwardStep);
        return;
    }

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

