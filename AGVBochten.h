//AVRBochten.h
//1 sec = 30
#define second 30
#define Comp_StartTurn (second*7.5)
#define Comp_EndTurn (Comp_StartTurn + (second * 3.9))
#define DrivingHeadstart -20

int startTurn(int direction);
