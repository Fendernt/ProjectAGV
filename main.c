/*

    Code project AGV.
    2024

 */

#include <avr/io.h>
#include "Ultrasone_sensor.h"



#define distanceToFollow 20
#define dAccuracy 3


#define minDistance (distanceToFollow - dAccuracy)
#define maxDistance (distanceToFollow + dAccuracy)


int main(void)
{

    // Insert code
    agv_ultrasoon_init();


    DDRB |= (1 << PB7);
    DDRB |= (1 << PB6);

    int distance;

    while(1) {

        distance = agv_ultrasoon_boom_links;

        //followHand(distance);

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
