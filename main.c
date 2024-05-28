/*

    Code project AGV.
    2024

 */

#include <avr/io.h>
#include "SteppermotorAVRDriver.h"
#include "Ultrasone_sensor.h"

#define distanceToFollow 20
#define dAccuracy 3


#define minDistance (distanceToFollow - dAccuracy)
#define maxDistance (distanceToFollow + dAccuracy)

int main(void)
{

    DDRB |= (1 << PB7);
    DDRB |= (1 << PB6);

    // Insert code
    agv_ultrasoon_init();
    initSteppermotorAVRDriver();

    int distance;
    while(1){
        distance = agv_ultrasoon_boom_links;

        followHand(filterDistance(distance));

        if(distance > distanceToFollow){
            PORTB |= (1<<PB7);
        } else {
            PORTB &= ~(1<<PB7);
        }

        if(distance < distanceToFollow){
           PORTB |= (1<<PB6);
        } else {
            PORTB &= ~(1<<PB6);
        }
    }

    return 0;
}

int filterDistance(int distance){
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


