// motor.h

#ifndef _MOTOR_h
#define _MOTOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif


#endif

 
#define CW 0
#define CCW 1

class motor
{
public: 
	int currentPos;
	int motorSteps;
	motor(uint8_t A, uint8_t AN, uint8_t B, uint8_t BN, int motorSteps);
	~motor(){};
	void setSpeed(int RPM);
	void moveBy(int nSteps);
	

	void makeCoilSequence(bool A, bool AN, bool B, bool BN);
	void makeStep();
	uint8_t motorPins[3];
	bool dir; 
	int stepDelay;
 
};