#include <Arduino.h>
#include "motor.h"
#include <Wire.h>
#include <string.h> 
#include <VL53L0X.h> 
#include <Stepper.h>

// Pin definition
#define A	6
#define AN	7
#define B	8
#define BN	9
#define TransOptToLED	3
#define TransOptFromOptocopuler	2
#define motorEnable 4



VL53L0X sensor; 
Stepper mot(200, A,AN,B,BN);

int setSpeed;
int stepSkip;
int timingBudget;

void setup() {
	//Start serial communication  
	Serial.begin(57600); 
	Serial.setTimeout(50);

	delay(5000);
	
	//Set global variables
	setSpeed = 6;  
	stepSkip = 1; 
	timingBudget = 20; 
	mot.setSpeed(setSpeed); 				Serial.println("BT: Parameters set"); 

	//Initialize sensor
	Wire.begin();  							Serial.println("BT: I2C Started"); 
	delay(1000);
	sensor.init(); 							Serial.println("BT: Init sensor"); 
	while (!sensor.init())
	{
		Serial.println("BT: Failed to detect and initialize sensor");
		sensor.init();
		delay(1000);
	}

	//Set sensor parameters
	sensor.setTimeout(500);				
	sensor.setSignalRateLimit(0.1); 	
	sensor.setMeasurementTimingBudget(timingBudget*1000);  //Set measurement time budget to 20ms
	sensor.startContinuous(20); 		Serial.println("BT: Sensor parameters set"); 

	//Set DDR registers
	pinMode(TransOptFromOptocopuler, INPUT); 	
	pinMode(TransOptToLED, OUTPUT);				
	pinMode(motorEnable, OUTPUT);		Serial.println("BT: DDR set"); 

	//Reset position
	digitalWrite(motorEnable,HIGH);		Serial.println("BL: Position reset");	
	digitalWrite(TransOptToLED,HIGH);	
	delay(1000);
	while (digitalRead(TransOptFromOptocopuler)==0)
	{
		 mot.step(-1); Serial.println(digitalRead(TransOptFromOptocopuler)); delayMicroseconds(1250); Serial.println("BL18");
	} 
	mot.step_number = 0;
	digitalWrite(TransOptToLED,LOW); 	
	digitalWrite(motorEnable,LOW);		Serial.println("BL: Done");
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
		delay(3000); 
	}
	// Get the measurement 
	
	delay(timingBudget);
	uint16_t meas = sensor.readRangeContinuousMillimeters(); 
	
	//Check for timeout error (0xFFFF) and reset device
	while (meas == uint16_t(0xFFFF))
	{
		sensor.stopContinuous(); Serial.println("13");
		sensor.init();
		sensor.setTimeout(500);				
		sensor.setSignalRateLimit(0.25); 	
		sensor.setMeasurementTimingBudget(timingBudget*1000);  
		sensor.startContinuous();
		delay(100);
		meas = sensor.readRangeContinuousMillimeters();
	}
	//If TX buffer is full, halt 
	if (Serial.availableForWrite()){
	digitalWrite(motorEnable,HIGH);
	int now = millis(); 
	char buffer[50]; 
	mot.step(stepSkip); 

	//TX measurement data and parameters
	sprintf(buffer,"$%4d,%4d;%4d,%4d,%3d;%3d ",
					mot.step_number, 	// Motor shaft position
					meas, 				// Sensor measurement results
					setSpeed, 			// Motor speed
					stepSkip, 			// Motor step skip
					timingBudget,		// Sensor timing budget
					int(millis()-now)); // Loop time
	Serial.print(buffer);Serial.println("19");
	Serial.print(digitalRead(TransOptToLED));  
	Serial.println(digitalRead(TransOptFromOptocopuler));   Serial.println("20");
	}
	//If not transmitting, stop the motor
	else digitalWrite(motorEnable,LOW);
	
	
}
