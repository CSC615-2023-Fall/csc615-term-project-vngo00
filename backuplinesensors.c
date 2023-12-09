//curr version

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <pigpio.h>
#include <signal.h>
#include "motor.h"  
#include <stdbool.h>
#include "ir_sensor.c"

#include "PCA9685.h"
#include "DEV_Config.h"
#include <pigpio.h>
#include <unistd.h>

#define LINE_SENSOR_LEFT 18
#define LINE_SENSOR_MIDDLE 22
#define LINE_SENSOR_RIGHT 23


#define IR_SENSOR_FRONT  
#define IR_SENSOR_LEFT   
#define IR_SENSOR_RIGHT  


#define OUT_BTN 24
#define IN_BTN 25


volatile sig_atomic_t terminate = 0;

// shared variables for sensor readings
volatile int leftSensorValue, middleSensorValue, rightSensorValue;
volatile int combinedSensorsValue;  // shared variable for combined sensor readings

//Declare and initialize a variable that indicates whether obstacle avoidance is currently happening
volatile bool avoidingObstacle = false;

pthread_mutex_t sensorMutex = PTHREAD_MUTEX_INITIALIZER;  // mutex for sensor value updates

// Function prototypes
void *readLeftSensor(void *arg);
void *readMiddleSensor(void *arg);
void *readRightSensor(void *arg);
void *combineSensors(void *arg);
void *masterControl(void *arg);
void intHandler(int);
int main(void) {
    if (gpioInitialise() < 0) {
        fprintf(stderr, "pigpio initialisation failed\n");
        return 1;       
    }

 //   signal(SIGINT, intHandler);

    // motor initialization
    motorInit();

  //  gpioSetMode(OUT_BTN, PI_OUTPUT);
    gpioSetMode(IN_BTN, PI_INPUT);
    gpioSetPullUpDown(IN_BTN, PI_PUD_UP);
	

    gpioSetMode(LINE_SENSOR_LEFT, PI_INPUT);
    gpioSetMode(LINE_SENSOR_MIDDLE, PI_INPUT);
    gpioSetMode(LINE_SENSOR_RIGHT, PI_INPUT);
	
    signal(SIGINT,intHandler); 
    sleep(0.5);
	
    printf("wait for btn\n");
    while(gpioRead(IN_BTN)){} //stuck here

   // gpioSetMode(OUT_BTN, PI_INPUT);


	

    pthread_t threads[5];
    pthread_create(&threads[0], NULL, readLeftSensor, NULL);
    pthread_create(&threads[1], NULL, readMiddleSensor, NULL);
    pthread_create(&threads[2], NULL, readRightSensor, NULL);
    pthread_create(&threads[3], NULL, combineSensors, NULL);
    pthread_create(&threads[4], NULL, masterControl, NULL);
	
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // GPIO pins for IR sensors
    int irFrontPin = IR_SENSOR_FRONT;
    int irLeftPin = IR_SENSOR_LEFT;
    int irRightPin = IR_SENSOR_RIGHT;

    //Create threads for IR sensors
    pthread_t irFrontThread, irLeftThread, irRightThread;
    pthread_create(&irFrontThread, NULL, handleIRSensor, (void *)&irFrontPin);
    pthread_create(&irLeftThread, NULL, handleIRSensor, (void *)&irLeftPin);
    pthread_create(&irRightThread, NULL, handleIRSensor, (void *)&irRightPin);

    //Join threads for IR sensors
    pthread_join(irFrontThread, NULL);
    pthread_join(irLeftThread, NULL);
    pthread_join(irRightThread, NULL);



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
    while (!terminate) {
        //pthread_mutex_lock(&sensorMutex);
        int sensors = combinedSensorsValue;
        //pthread_mutex_unlock(&sensorMutex);
	printf("%d\n", sensors);
	/*
        if (speed > constantSpeed) {
            forward(speed, FORWARD);  // Start with higher speed to overcome static friction
            //usleep(500000);           // Run at higher speed for a short duration
            speed = constantSpeed;    // Then set to constant speed
        }
	*/
    //Obstacle avoidance logic
    if(obstacleFront){
        avoidingObstacle = true;

            // Check for obstacle on the left and right
            if (!obstacleLeft) {
                turnLeft();  // Turn left if no obstacle on the left
            } else if (!obstacleRight) {
                turnRight();  // Turn right if no obstacle on the right
            }
            
            avoidingObstacle = false;
    } else {
        switch (sensors) {
            case 0:   // 000 - lost the line
                //motorStop();
		if (left)
			turnLeft(speed);
		else
			turnRight(speed);
                break;
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
            case 7:   // 111 - all sensors active, wide line or intersection
                forward(speed, FORWARD);
                break;
            default:
                motorStop();  // handle unexpected cases
                break;
        }
	

    }
        

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
