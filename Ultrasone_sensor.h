#ifndef AGV_ULTRASOON_H_INCLUDED
#define AGV_ULTRASOON_H_INCLUDED


//Trigger pin allemaal op pin 5 via de connector
extern volatile uint16_t agv_ultrasoon_boom_links; //ECHO op Pin A10
extern volatile uint16_t agv_ultrasoon_boom_rechts; //ECHO op Pin A9
extern volatile uint16_t agv_ultrasoon_voor_midden; //ECHO op Pin A12
void agv_ultrasoon_init();

#endif // AGV_ULTRASOON_H_INCLUDED
