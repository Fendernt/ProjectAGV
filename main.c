/*

    Code project AGV.
    2024

 */

#include <avr/io.h>
#include "SteppermotorAVRDriver.h"

int main(void)
{

    // Insert code
    initSteppermotorAVRDriver();

    setStepperMode(leftMotor, ForwardStep);
    setStepperMode(rightMotor, ForwardStep);

    while(1){

    }

    return 0;
}
