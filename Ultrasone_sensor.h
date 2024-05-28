#ifndef AGV_ULTRASOON_H_INCLUDED
#define AGV_ULTRASOON_H_INCLUDED

extern volatile uint16_t agv_ultrasoon_boom_links;
extern volatile uint16_t agv_ultrasoon_boom_rechts;
extern volatile uint16_t agv_ultrasoon_voor_links;
extern volatile uint16_t agv_ultrasoon_voor_midden;
extern volatile uint16_t agv_ultrasoon_voor_rechts;
void agv_ultrasoon_init();

#endif // AGV_ULTRASOON_H_INCLUDED
