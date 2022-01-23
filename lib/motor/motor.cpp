#include "motor.h"


motor::motor(uint8_t A, uint8_t AN, uint8_t B, uint8_t BN, int motorSteps){
	this->motorPins[0] = A;
	this->motorPins[1] = AN;
	this->motorPins[2] = B;
	this->motorPins[3] = BN;
	
	// Set microcontroller pin modes
	pinMode(A, OUTPUT);
	pinMode(AN, OUTPUT);
	pinMode(B, OUTPUT);
	pinMode(BN, OUTPUT);
	this->motorSteps = motorSteps;
	setSpeed(30);
	// Initialize rest of variables
	this->currentPos = 0;		//Current position
	this->dir = CW;				//Current direction 
	this->stepDelay = 100000;
	setSpeed(1);		  
	}

/*
 * Sets the speed of the stepper motor in RPM (rotations per minute). In current software
 * implementation it is limited to approx. 600 RPM.
 */
void motor::setSpeed(int newSpeedRPM){
		stepDelay = 1000000/(motorSteps*newSpeedRPM*4);
}

/*
 * Private method to execute a coil sequence to rotate the shaft by one step. Called by makeStep().
 */
void motor::makeCoilSequence(bool A, bool AN, bool B, bool BN){
	if (dir)
	{
		digitalWrite(motorPins[0],A);
		digitalWrite(motorPins[1],AN);
		digitalWrite(motorPins[2],B);
		digitalWrite(motorPins[3],BN);
		delayMicroseconds(stepDelay);
	}
	else{
		digitalWrite(motorPins[3],BN);
		digitalWrite(motorPins[2],B);
		digitalWrite(motorPins[1],AN);
		digitalWrite(motorPins[0],A);
		delayMicroseconds(stepDelay);
	}
}

/*
 * Private method with further function calls required to rotate the shaft by one step. Called by moveBy()
 */
void motor::makeStep(){
		if(dir){ //CW
			makeCoilSequence(1,0,0,1);
			makeCoilSequence(0,1,0,1);
			makeCoilSequence(0,1,1,0);
			makeCoilSequence(1,0,1,0);
			currentPos = currentPos-1;
		}
		else{ //CCW

			makeCoilSequence(1,0,1,0);
			makeCoilSequence(0,1,1,0);
			makeCoilSequence(0,1,0,1);
			makeCoilSequence(1,0,0,1);
			currentPos = currentPos+1;
		}
		
		//Trim position indicator
		if (currentPos > motorSteps){
			currentPos = currentPos - motorSteps;
		}
		if (currentPos < 0){
			currentPos = motorSteps;
		}
}

/*
 * Public method which moves the shaft by n steps. Sign in nSteps determines whether the shaft moves clock-wise
 * or counter clock-wise direction.
 */
void motor::moveBy(int nSteps){
	//Determine direction
	if (nSteps<0)
	this->dir = CCW;
	else
	this->dir = CW;
	
	//Do n steps
	nSteps = abs(nSteps);
	while(nSteps > 0)
	{
		makeStep();
		nSteps = nSteps-1;
	}
	
}
