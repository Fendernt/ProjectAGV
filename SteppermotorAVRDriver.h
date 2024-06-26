//Stepper motor driver header file

#ifndef SteppermotorAVRDriver
#define SteppermotorAVRDriver




#define MotorLeftPort PORTF
#define MotorLeftDDR DDRF
#define MotorLeftStartpin PF0 //A0

#define MotorRightPort PORTF
#define MotorRightDDR DDRF
#define MotorRightStartpin PF4 //A4


//Enum's for names, numbers are made up.
enum Mode{
    ForwardStep = 0,
    Off = 1,
    BackwardStep = 2,
};
enum Motor{
    leftMotor = 10,
    rightMotor = 11
};

void setBothStepperMode(int mode);
void setStepperMode(int motor, int mode);

void initSteppermotorAVRDriver();

void SetStepperSpeed(int speed);

void SetSteppermotorSpeed(int motor, int speed);


#endif

