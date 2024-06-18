#include <avr/interrupt.h>
#include <avr/io.h>

volatile uint16_t agv_ultrasoon_boom_links = 0; //pin A10
volatile uint16_t agv_ultrasoon_boom_rechts = 0; //pin A9
volatile uint16_t agv_ultrasoon_voor_midden = 0; //pin A12
volatile uint8_t agv_ultrasoon_current_sensor = 2;


#define ValueArrayLength 3
//Laatste array index word gebruikt als return nummer
int LeftSensorArray[ValueArrayLength];
int RightSensorArray[ValueArrayLength];


#define MaxDistanceOffset 10
int getLeftSensorValue(){
    int highestValue = LeftSensorArray[0];
    int lowestValue = LeftSensorArray[0];
    for(int i = 0; i < ValueArrayLength; i++){
        if(LeftSensorArray[i] > highestValue) highestValue = LeftSensorArray[i];
        if(LeftSensorArray[i] < lowestValue) lowestValue = LeftSensorArray[i];
    }

    if((highestValue - lowestValue) <= MaxDistanceOffset) {
        return lowestValue;
    }
    return 100;
}

int getRightSensorValue(){
    int highestValue = RightSensorArray[0];
    int lowestValue = RightSensorArray[0];
    for(int i = 0; i < ValueArrayLength; i++){
        if(RightSensorArray[i] > highestValue) highestValue = RightSensorArray[i];
        if(RightSensorArray[i] < lowestValue) lowestValue = RightSensorArray[i];
    }

    if((highestValue - lowestValue) <= MaxDistanceOffset) {
        return lowestValue;
    }
    return 100;
}



/* untested code check previous index
int isArrayValueValid(int* array){
    int isValid = 1;
    //Loop through array and check previous value
    //For loop starts at index 1 to compare it to index 0
    for(int i = 1; i < ValueArrayLength; i++){
        //Check Higher value
        if( (array[i-1] + MaxDistanceOffset) > array[i] ){
            isValid = 0;
        }
        //Check Lower Value
        if( (array[i-1] - MaxDistanceOffset) < array[i]){
            isValid = 0;
        }
        if(isValid = 0) break;
    }
    return isValid;
}
*/


void agv_ultrasoon_init()
{
    cli();
    TCCR3A |= (1<<COM3A1) | (1<<WGM31);
    TCCR3B |= (1<<WGM33) | (1<<CS31) | (1<<WGM32);//wgm33 wgm32 wgm31
    ICR3 = 32768;
    OCR3A = 20;//10microsec
    OCR3B = 4800;// (10microsec + 480microsec+ marge) *2
    DDRE |= (1<<PE3);
    TIMSK3 |= (1<<ICIE3) | (1<<OCIE3B);
    //pin change interrupt:
    PCICR |= (1<<PCIE2);//enables pci 16 tot 23
    sei();
}

ISR(TIMER3_COMPB_vect)
{
    agv_ultrasoon_current_sensor = (agv_ultrasoon_current_sensor<<1);
    if (agv_ultrasoon_current_sensor == (1<<5))//0b00100000
    {
        agv_ultrasoon_current_sensor = 2;

    }
    if(agv_ultrasoon_current_sensor == (1<<3)) agv_ultrasoon_current_sensor = (1<<4);
    PCMSK2 = agv_ultrasoon_current_sensor;
    //PORTA = agv_ultrasoon_current_sensor;
    TIMSK3 |= (1<<OCIE3B);
}

volatile int arrayPositionLeft = 0;
volatile int arrayPositionRight = 0;
ISR(TIMER3_CAPT_vect)
{
    //zet max naar sensorwaarde
    if(agv_ultrasoon_current_sensor == 0b00000010)
    {
        agv_ultrasoon_boom_rechts = (ICR3 - 4454)/4*0.0343;
        if(agv_ultrasoon_boom_rechts != 0) RightSensorArray[arrayPositionRight++] = agv_ultrasoon_boom_rechts;
    }
    else if(agv_ultrasoon_current_sensor == 0b00000100)
    {
        agv_ultrasoon_boom_links = (ICR3 - 4454)/4*0.0343;
        if(agv_ultrasoon_boom_links != 0) LeftSensorArray[arrayPositionLeft++] = agv_ultrasoon_boom_links;

    }
    else if(agv_ultrasoon_current_sensor == 0b00010000)
    {
        agv_ultrasoon_voor_midden = (ICR3 - 4454)/4*0.0343;
    }
    PCMSK2 = 0;

    if(arrayPositionLeft >= ValueArrayLength) arrayPositionLeft = 0;
    if(arrayPositionRight >= ValueArrayLength) arrayPositionRight = 0;

}

ISR(PCINT2_vect)
{
    if(agv_ultrasoon_current_sensor == 0b00000010)
    {
        agv_ultrasoon_boom_rechts = (TCNT3 - 4454)/4*0.0343;
        if(agv_ultrasoon_boom_rechts != 0) RightSensorArray[arrayPositionRight++] = agv_ultrasoon_boom_rechts;
        PORTA &= ~(0b00000001);
    }
    else if(agv_ultrasoon_current_sensor == 0b00000100)
    {
        agv_ultrasoon_boom_links = (TCNT3 - 4454)/4*0.0343;
        if(agv_ultrasoon_boom_links != 0) LeftSensorArray[arrayPositionLeft++] = agv_ultrasoon_boom_links;
        PORTA &= ~(0b00000010);
    }
    else if(agv_ultrasoon_current_sensor == 0b00010000)
    {
        agv_ultrasoon_voor_midden = (TCNT3 - 4454)/4*0.0343;
        PORTA &= ~(0b00001000);
    }
    //check welke sensor en schrijf timer3 waarde naar sensorwaarde
    TIMSK3 &= ~(1<<ICIE3);
    //zet timer3 overflow flag uit
    PCMSK2 = 0;
}
