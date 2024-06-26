#ifndef AFV_LED_H_INCLUDED
#define AFV_LED_H_INCLUDED

#define TreeIndicatedLEDLeft PB2 //pin 51
#define TreeIndicatedLEDRight PB3 //pin 50

#define TurnSignalLEDLeft PL0 //pin 49
#define TurnSignalLEDRight PL1 //pin 48
#define BreaklightsLeft PL2 //pin 47
#define BreaklightRight PL3// pin 46
#define HeadlightsLeft PL4 //pin 45
#define HeadlightsRight PL5 // pin 44
#define NoodstopLEDFront PL6 //pin 43
#define NoodstopLEDBack PL7 //pin 42

void init_Leds(void);

void LedBreakLightLeft(int x);
void LedBreakLightRight(int x);

void LedHeadlightLeft(int x);
void LedHeadlightRight(int x);

void LedNoodstopFront(int x);
void LedNoodstopBack(int x);

extern volatile int TurnSignalLeft;
extern volatile int TurnSignalRight;
extern volatile int TreeSignalLeft;
extern volatile int TreeSignalRight;

#endif
