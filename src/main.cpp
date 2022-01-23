#include <Arduino.h>
#include "motor.h"
#include <Wire.h>
#include <string.h> 
#include <VL53L0X.h> 
#include <Stepper.h>
/**
* Notes:
* Each instruction cycle takes 250ns                                       
*/
// Pin definition
#define A	4
#define AN	5
#define B	6
#define BN	7
#define TransOptToLED	8
#define TransOptFromOptocopuler	9
#define motorEnable 10

VL53L0X sensor;
motor m(A,AN,B,BN,400);
Stepper mot(400, 4,5,6,7);
int setSpeed;
int stepSkip;
int timingBudget;
// Main() function / initialization
void setup() {
	setSpeed = 60;
	stepSkip = 8;
	timingBudget = 20; 

  	Serial.begin(57600); //Start serial communication 
	Serial.setTimeout(50);
	Wire.begin(); 
	mot.setSpeed(setSpeed);
	
	//while (!Serial);		//Wait until USB serial communication is established
	
	delay(100);
	

	//sensor.init(); 				Serial.println("BL0"); 
	
	//while (!sensor.init())
	//{
	//	Serial.println("Failed to detect and initialize sensor!");
	//	sensor.init();
	//	delay(1000);
	//}

	sensor.setTimeout(500);				Serial.println("BL1"); 
	sensor.setSignalRateLimit(0.25); 	Serial.println("BL2"); 

	sensor.setMeasurementTimingBudget(timingBudget*1000);  Serial.println("BL3");  //Set measurement time budget to 20ms
	sensor.startContinuous(20); 		Serial.println("BL4"); 

	pinMode(TransOptFromOptocopuler, INPUT); Serial.println("BL5");
	pinMode(TransOptToLED, OUTPUT);		Serial.println("BL6");
	pinMode(motorEnable, OUTPUT);		Serial.println("BL7");

	pinMode(13, OUTPUT); 				Serial.println("BL8");
	digitalWrite(motorEnable,HIGH);		Serial.println("BL9"); 
	
	delay(100); 						Serial.println("BL10");
	 
	
	//Reset position
	digitalWrite(TransOptToLED,HIGH);	Serial.println("BL12");	
	//while (digitalRead(TransOptFromOptocopuler)==0)
	//{
	//	 mot.step(-1); Serial.println(digitalRead(TransOptFromOptocopuler)); delay(100);
	//} 
	digitalWrite(TransOptToLED,LOW); 	Serial.println("BL13");
	loop();
}
 
// Continous loop inside main() function
void loop()
{
	//Check for new speed settings:
	if (Serial.peek() == '@' || Serial.peek() == '%' || Serial.peek() == '&'){
		char cmd = Serial.peek();
		String rx = Serial.readStringUntil('\n');
		rx.remove(0,1);
		switch(cmd)
		{
		case '@': // Set speed  
			setSpeed = rx.toInt(); 
			mot.setSpeed(setSpeed);
			break;
		case '%': // Set step skip size
			stepSkip = rx.toInt(); 
			break;
		case '&': // Set timing budget 
			sensor.stopContinuous();
			timingBudget = rx.toInt(); 
			sensor.setMeasurementTimingBudget(timingBudget*1000);
			sensor.startContinuous();
			break;
		}
		delay(3000); Serial.println("SETTINGS CHANGED");
	}

	//Serial.println(Serial.availableForWrite());
	//If TX buffer is full, halt 
	if (Serial.availableForWrite())
	{
	int now = millis();
	int meas = sensor.readRangeContinuousMillimeters();
	char buffer[100];
	mot.step(stepSkip);

	sprintf(buffer,"$%3d,%4d;%4d,%4d,%3d;%3d",mot.step_number, meas, setSpeed, stepSkip, timingBudget,int(millis()-now)); 
	Serial.println(buffer);  

	//Serial.print(digitalRead(A)); Serial.print(digitalRead(AN)); Serial.print(digitalRead(B)); Serial.print(digitalRead(BN));    
	//Serial.print(digitalRead(TransOptToLED)); Serial.print(digitalRead(TransOptFromOptocopuler)); Serial.println(digitalRead(motorEnable)); 
	}
}