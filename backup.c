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

#define HIGH 1
#define LOW 0

#define LINE_SENSOR_LEFT 18
#define LINE_SENSOR_MIDDLE 22
#define LINE_SENSOR_RIGHT 23


#define IR_SENSOR_FRONT 17 
#define IR_SENSOR_LEFT  6 
#define IR_SENSOR_SIDE  5 

//#define SONAR_TRIGGER 0
//#define SONAR_ECHO    5

#define IN_BTN 25

struct sensorStruct {
	volatile int *sensorValue;
	int sensorPin;
};
/*
struct sonarStruct {
	volatile double *distance;
	int triggerPin;
	int echoPin;
};
*/
volatile sig_atomic_t terminate = 0;
//volatile int terminate = 0;

// shared variables for sensor readings
volatile int leftSensorValue, middleSensorValue, rightSensorValue;
volatile int combinedSensorsValue;  // shared variable for combined sensor readings
volatile double sonarDistance;
//Declare and initialize a variable that indicates whether obstacle avoidance is currently happening
volatile bool avoidingObstacle = false;
// shared variables for ir readings
volatile int IRfront, IRleft, IRside;
pthread_mutex_t sensorMutex = PTHREAD_MUTEX_INITIALIZER;  // mutex for sensor value updates

double soundVelocity = 331.0;

// Function prototypes
void *readLeftSensor(void *arg);
void *readMiddleSensor(void *arg);
void *readRightSensor(void *arg);
void *combineSensors(void *arg);
void *masterControl(void *arg);
void intHandler(int);
void *readIRSensor(void *arg);
//void *readSonar(void *arg);



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
	
    // initialize line sensors
    gpioSetMode(LINE_SENSOR_LEFT, PI_INPUT);
    gpioSetMode(LINE_SENSOR_MIDDLE, PI_INPUT);
    gpioSetMode(LINE_SENSOR_RIGHT, PI_INPUT);
	
	
    // initialize IR sensors
    gpioSetMode(IR_SENSOR_FRONT, PI_INPUT);
    //gpioSetMode(IR_SENSOR_LEFT, PI_INPUT);
    //gpioSetMode(IR_SENSOR_RIGHT, PI_INPUT);


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

	
    // initialize sonar struct
   // struct sonarStruct sonar;
    //sonar.triggerPin = SONAR_TRIGGER;
    //sonar.echoPin = SONAR_ECHO;
    //sonar.distance = &sonarDistance;

	

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
 /*  
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
*/


    motorStop(); // stop the motor when the program terminates
    gpioTerminate();
    return 0;
}


void *readIRSensor(void *arg) {
	struct sensorStruct *sensor = (struct sensorStruct *) arg;
       		
	
	while(!terminate) {
		*(sensor->sensorValue) = gpioRead(sensor->sensorPin);

	}
	return NULL;
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
   // if(obstacleFront){
   if(!IRfront){
	   speed = 85;
	// rorate to the right until left hit
	while(!( IRleft == 1 && IRside == 0 )){

		printf("step 1\n");
		printf("value of left: %d\n value of side: %d\n", IRleft, IRside);
		rotate(speed, RIGHT);
	}
	
	// straight until side not hit (or 1)
	while(IRside == 0) {
		printf("step 2\n");
		printf("value of left: %d\n value of side: %d\n", IRleft, IRside);
		forward(speed, FORWARD);
	}

	// turn left until left hit (or 0)
	while(IRleft == 1) {
		printf("step 3\n");
		printf("value of left: %d\n value of side: %d\n", IRleft, IRside);
		turnLeft(speed);	
	}

	// straight till side hit == 0
	while(IRside == 1){
		
		printf("step 4\n");
		printf("value of left: %d\n value of side: %d\n", IRleft, IRside);
		forward(speed, FORWARD);
	}

	// straight til side not hit
	while(IRside == 0 ){
		forward(speed, FORWARD);
		printf("step 5\n");
		printf("value of left: %d\n value of side: %d\n", IRleft, IRside);
	}

	// turn left until left sensor hit == 0
	while(IRleft == 1) {
		turnLeft(speed);
		printf("step 6\n");
		printf("value of left: %d\n value of side: %d\n", IRleft, IRside);
	}

	// straight until side hit == 0
	while(IRside == 1){
		forward(speed, FORWARD);
		printf("step 7\n");
		printf("value of left: %d\n value of side: %d\n", IRleft, IRside);
	}

	// turn right a bit maybe
	

	// staright until hit the line
	while(combinedSensorsValue == 0) {
		forward(speed, FORWARD);
		printf("step 8\n");
		printf("value of left: %d\n value of side: %d\n", IRleft, IRside);
	}

	// turn right until left not hit == 1
	while(IRleft == 0 || IRside == 0){
		turnRight(speed);
	}



	/*
	//printf("left ir sensor: %d\n", IRleft);
	printf("front ir sensor: %d\n", IRfront);
	//printf("right ir sensor: %d\n\n\n", IRright);

	forward(speed, REVERSE);
	printf("reversing");
	usleep(500000);
	
	forward(speed, FORWARD);
	turnRight(speed);
	sleep(1);

	forward(speed, FORWARD);
	usleep(500000);

	turnLeft(speed);
	usleep(1000000);

	forward(speed, FORWARD);
	usleep(1500000);
   	
	turnLeft(speed);
	*/
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

/*
void *readSonar(void *arg){
	struct sonarStruct *sonar = (struct sonarStruct *) arg;
	
	double start, stop, duration;
	while(!terminate){
	gpioWrite(sonar->triggerPin, HIGH);
	usleep(10);
	gpioWrite(sonar->triggerPin, LOW);

	while(gpioRead(sonar->echoPin) == LOW){}

	start = clock() / (1.0 * CLOCKS_PER_SEC);

	while(gpioRead(sonar->echoPin) == HIGH){}

	stop = clock() / (1.0 * CLOCKS_PER_SEC);

	duration = stop - start;
	
	*(sonar->distance) = (soundVelocity * duration) / 2;
	usleep(100);
	}
	return NULL;
}
*/
	

