//curr version

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <pigpio.h>
#include <signal.h>
#include "motor.h"  
#include <stdbool.h>
//#include "ir_sensor.c"

#include "PCA9685.h"
#include "DEV_Config.h"
#include <pigpio.h>
#include <unistd.h>
#include <time.h>
#include "assignedPins.h"


struct sensorStruct {
	volatile int *sensorValue;
	int sensorPin;
};

volatile sig_atomic_t terminate = 0;

// shared variables for sensor readings
volatile int leftSensorValue, middleSensorValue, rightSensorValue;
volatile int combinedSensorsValue;  // shared variable for combined sensor readings
//Declare and initialize a variable that indicates whether obstacle avoidance is currently happening
// shared variables for ir readings
volatile int IRfront, IRleft, IRside;


// Function prototypes
void *readLeftSensor(void *arg);
void *readMiddleSensor(void *arg);
void *readRightSensor(void *arg);
void *combineSensors(void *arg);
void *masterControl(void *arg);
void intHandler(int);
//void *readSonar(void *arg);



int main(void) {
    if (gpioInitialise() < 0) {
        fprintf(stderr, "pigpio initialisation failed\n");
        return 1;       
    }

    // motor initialization
    motorInit();

  //  gpioSetMode(OUT_BTN, PI_OUTPUT);
    gpioSetMode(IN_BTN, PI_INPUT);
    gpioSetPullUpDown(IN_BTN, PI_PUD_UP);
	
    // initialize line sensors
    gpioSetMode(LINE_SENSOR_LEFT, PI_INPUT);
    gpioSetMode(LINE_SENSOR_MIDDLE, PI_INPUT);
    gpioSetMode(LINE_SENSOR_RIGHT, PI_INPUT);
	
	
    // initialize IR sensors
    gpioSetMode(IR_SENSOR_FRONT, PI_INPUT);
    gpioSetMode(IR_SENSOR_LEFT, PI_INPUT);
    gpioSetMode(IR_SENSOR_SIDE, PI_INPUT);


    signal(SIGINT,intHandler); 
    sleep(0.5);
	
    printf("wait for btn\n");
    while(gpioRead(IN_BTN)){} //stuck here

   // gpioSetMode(OUT_BTN, PI_INPUT);
	
    struct sensorStruct IRsensor[3];

    IRsensor[0].sensorPin = IR_SENSOR_FRONT;
    IRsensor[0].sensorValue = &IRfront;

    IRsensor[1].sensorPin = IR_SENSOR_LEFT;
    IRsensor[1].sensorValue = &IRleft;

    IRsensor[2].sensorPin = IR_SENSOR_SIDE;
    IRsensor[2].sensorValue = &IRside;

    pthread_t threads[8];
    pthread_create(&threads[0], NULL, readLeftSensor, NULL);
    pthread_create(&threads[1], NULL, readMiddleSensor, NULL);
    pthread_create(&threads[2], NULL, readRightSensor, NULL);
    pthread_create(&threads[3], NULL, combineSensors, NULL);
    pthread_create(&threads[4], NULL, masterControl, NULL);
	
    // create threads for ir
    
    pthread_create(&threads[5], NULL, readIRSensor,(void *) &IRsensor[0]);
    pthread_create(&threads[6], NULL, readIRSensor, (void *) &IRsensor[1]);
    pthread_create(&threads[7], NULL, readIRSensor, (void *) &IRsensor[2]);

    //pthread_create(&threads[6], NULL, readSonar, (void *) &sonar);
    for (int i = 0; i < 7; i++) {
        pthread_join(threads[i], NULL);
    }
 


    motorStop(); // stop the motor when the program terminates
    gpioTerminate();
    return 0;
}




void *readLeftSensor(void *arg) {
    while (!terminate) {
 //       pthread_mutex_lock(&sensorMutex);
        leftSensorValue = gpioRead(LINE_SENSOR_LEFT);
   //     pthread_mutex_unlock(&sensorMutex);
        //usleep(10000);  // 10 ms delay
    }
    return NULL;
}

void *readMiddleSensor(void *arg) {
    while (!terminate) {
     //   pthread_mutex_lock(&sensorMutex);
        middleSensorValue = gpioRead(LINE_SENSOR_MIDDLE);
       // pthread_mutex_unlock(&sensorMutex);
        //usleep(10000);  // 10 ms delay
    }
    return NULL;
}

void *readRightSensor(void *arg) {
    while (!terminate) {
        //pthread_mutex_lock(&sensorMutex);
        rightSensorValue = gpioRead(LINE_SENSOR_RIGHT);
        //pthread_mutex_unlock(&sensorMutex);
        //usleep(10000);  // 10 ms delay
    }
    return NULL;
}

void *combineSensors(void *arg) {
    while (!terminate) {
        //pthread_mutex_lock(&sensorMutex);
        combinedSensorsValue = leftSensorValue * 1 + middleSensorValue * 2 + rightSensorValue * 4;
        //pthread_mutex_unlock(&sensorMutex);
        //usleep(10000);  // 10 ms delay
    }
    return NULL;
}



void *masterControl(void *arg) {
    int speed = 85;  // Initial speed to overcome static friction
    int constantSpeed = 80;  // Constant speed for normal operation
    int left = 0;
	
    setDirection(FORWARD);
    // testing purposes
    volatile double leftSpeed = 90;
    volatile double rightSpeed = 0;

    while (!terminate) {
        //pthread_mutex_lock(&sensorMutex);
        int sensors = combinedSensorsValue;
        //pthread_mutex_unlock(&sensorMutex);
	printf("%d\n", sensors);
    //Obstacle avoidance logic
   if(!IRfront){

	
	//forward(speed, REVERSE);
	//usleep(300000);
	//curve(leftSpeed, rightSpeed);
	//usleep(500000);
	while ( IRfront == 0 || combinedSensorsValue == 0 ) {
	    if ((int)rightSpeed > 90){
		    rightSpeed =90.0;
	    }
	    if ( (int) rightSpeed < 40) {
		    rightSpeed = 40.0;
	    }
	    if ( (int) leftSpeed > 90) {
		    leftSpeed =90.0;
	    }
	    if ( (int) leftSpeed < 40) {
		    leftSpeed = 40.0;
	    }
	    curve(leftSpeed, rightSpeed);	
	    printf("left: %.2f\n right %.2f\n", leftSpeed, rightSpeed);
	    if (IRfront == 0 || (IRleft == 0 && IRside == 0) || IRleft == 0){
			rightSpeed -= (0.54);
			leftSpeed += (0.4 );
		//	mult += 0.1;
	    } else { // turnign left
		    rightSpeed += (1.2);
		    leftSpeed -= ( 1.0);
		    //mult -= 0.1;
	    }
    }	


	while(IRside == 0) {
		turnRight(speed);
	}


    } else {
        switch (sensors) {
            case 0:   // 000 - lost the line
                if (left)
			turnLeft(speed);
		else
			turnRight(speed);
                break;
//motorStop();
		            case 1:   // 100 - left sensor active, turn left
                //turnLeft();
		turnLeft(speed);
		left = 1;
                break;
            case 2:   // 010 - optimal, middle sensor active, on track, move forward
                forward(speed, FORWARD);
		//motorStop();
                break;
            case 3:   // 110 - left and middle sensors active, turn left
                turnLeft(speed);
		left = 1;
                break;
            case 4:   // 001 - right sensor active, turn right
                turnRight(speed);
		left = 0;
                break;
            case 5:   // 101 - left and right sensors active, unusual situation
                //motorStop();
		forward(speed, FORWARD);
                break;
            case 6:   // 011 - middle and right sensors active, turn right
                turnRight(speed);
		left = 0;
                break;
            case 7: 
		if (left)
			turnLeft(speed);
		else
			turnRight(speed);
                break;
  // 111 - all sensors active, wide line or intersection
                //forward(speed, FORWARD);
                break;
            default:
                motorStop();  // handle unexpected cases
                break;
        }
	

    }
    leftSpeed = 90;
    rightSpeed = 0;
        

        usleep(10000);  // 100 ms delay
    }
    return NULL;
}

void intHandler(int signo) {
    terminate = 1;
    motorStop();
    DEV_ModuleExit();
    gpioTerminate();
    exit(0);
}


