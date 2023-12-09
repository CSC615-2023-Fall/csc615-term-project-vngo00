#ifndef MOTOR_H
#define MOTOR_H

#include "PCA9685.h"
#include "DEV_Config.h"

#define PWMA PCA_CHANNEL_0
#define AIN1 PCA_CHANNEL_1
#define AIN2 PCA_CHANNEL_2
#define BIN1 PCA_CHANNEL_3
#define BIN2 PCA_CHANNEL_4
#define PWMB PCA_CHANNEL_5
#define FORWARD	0
#define REVERSE 1
#define LEFT 2
#define RIGHT 3

void motorInit(void);
void forward(int speed, int direction);
void motorStop(void);
void turnLeft(int speed);
void turnRight(int speed);
void rotate(int speed, int direction);
void curve(double leftSpeed, double rightSpeed);
void setDirection(int direction);

#endif
