#include "motor.h"
#include <stdio.h>

// initialize the motor
void motorInit()
{
	PCA9685_Init(0x40);
	PCA9685_SetPWMFreq(100);
}	

// run the motor with given speed and direction
void forward(int speed, int direction) {
	if (speed > 100) {
		speed = 100;
	}

    	PCA9685_SetPwmDutyCycle(PWMB, speed);
	PCA9685_SetPwmDutyCycle(PWMA, speed);
if (direction == FORWARD) {
			
	PCA9685_SetLevel(AIN1, 1);
    	PCA9685_SetLevel(AIN2, 0);

	PCA9685_SetLevel(BIN1, 0);
    	PCA9685_SetLevel(BIN2, 1);
} else {
	PCA9685_SetLevel(AIN1, 0);
    	PCA9685_SetLevel(AIN2, 1);

	PCA9685_SetLevel(BIN1, 1);
    	PCA9685_SetLevel(BIN2, 0);
	}



}

void setDirection(int direction){

if (direction == FORWARD) {
			
	PCA9685_SetLevel(AIN1, 1);
    	PCA9685_SetLevel(AIN2, 0);

	PCA9685_SetLevel(BIN1, 0);
    	PCA9685_SetLevel(BIN2, 1);



} else {
	PCA9685_SetLevel(AIN1, 0);
    	PCA9685_SetLevel(AIN2, 1);

	PCA9685_SetLevel(BIN1, 1);
    	PCA9685_SetLevel(BIN2, 0);
	}



}
// stop the motor
void motorStop() {

	PCA9685_SetPwmDutyCycle(PWMA, 0);
    PCA9685_SetPwmDutyCycle(PWMB, 0);
    
}
void turnLeft(int speed){
    printf("turning left\n");
    PCA9685_SetPwmDutyCycle(PWMA, 0);
    PCA9685_SetPwmDutyCycle(PWMB, speed );
}
/*
void turnLeft(int speed){
	if(speed > 100) speed = 100;
		PCA9685_SetPwmDutyCycle(PWMA, speed-10);
	PCA9685_SetPwmDutyCycle(PWMB, speed-20);
	PCA9685_SetLevel(AIN1, 0);
    	PCA9685_SetLevel(AIN2, 1);
	PCA9685_SetLevel(BIN1, 0);
    	PCA9685_SetLevel(BIN2, 1);


	}
*/
void turnRight(int speed){
	printf("turning right\n");
	PCA9685_SetPwmDutyCycle(PWMA, speed - 10);
    PCA9685_SetPwmDutyCycle(PWMB, 0);
}


void rotate(int speed, int direction){
	PCA9685_SetPwmDutyCycle(PWMA, speed-10);
	PCA9685_SetPwmDutyCycle(PWMB, speed-20);



	if (direction == LEFT) {
		PCA9685_SetLevel(AIN1, 0);
    		PCA9685_SetLevel(AIN2, 1);
		PCA9685_SetLevel(BIN1, 0);
    		PCA9685_SetLevel(BIN2, 1);


	}
	else {
		PCA9685_SetLevel(AIN1, 1);
    		PCA9685_SetLevel(AIN2, 0);
		PCA9685_SetLevel(BIN1, 1);
    		PCA9685_SetLevel(BIN2, 0);


	}

}

void curve(double leftSpeed, double rightSpeed) {
	PCA9685_SetPwmDutyCycle(PWMA, leftSpeed);
	PCA9685_SetPwmDutyCycle(PWMB, rightSpeed);

	PCA9685_SetLevel(AIN1, 1);
    	PCA9685_SetLevel(AIN2, 0);

	PCA9685_SetLevel(BIN1, 0);
    	PCA9685_SetLevel(BIN2, 1);


}

