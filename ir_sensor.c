/*
#include "ir_sensor.h"
#include <stdbool.h>

//Define flags for obstacle detection
volatile bool obstacleFront = false;
volatile bool obstacleLeft = false;
volatile bool obstacleRight = false;

//Function to export a GPIO pin
//We write pin number to the export file
//Create a conditional to handle the potential for error in exporting
int exportGPIO(int pin) {
    int export_fd = open("/sys/class/gpio/export", O_WRONLY);
    if (export_fd == -1) {
        perror("Unable to export GPIO pin");
        return -1;
    }
    char pin_str[4];
    snprintf(pin_str, sizeof(pin_str), "%d", pin);
    write(export_fd, pin_str, strlen(pin_str));
    close(export_fd);
    return 0;
}

//Function to set the direction of a GPIO pin
//We write the pin to the corresponding direction file
//Handle error similar to the exportGPIO function
int setGPIO(int pin, const char* direction) {
    char direction_path[100];
    snprintf(direction_path, sizeof(direction_path), 
    "/sys/class/gpio/gpio%d/direction", pin);
    
    int direction_fd = open(direction_path, O_WRONLY);
    if (direction_fd == -1) {
        perror("Unable to set GPIO direction");
        return -1;
    }
    write(direction_fd, direction, strlen(direction));
    close(direction_fd);
    return 0;
}

//Function to read the value of a GPIO pin
//Read the digital value (1 or 0) from the sensor
int readGPIO(int pin) {
    char value_path[100];
    snprintf(value_path, sizeof(value_path), 
    "/sys/class/gpio/gpio%d/value", pin);
    
    int value_fd = open(value_path, O_RDONLY);
    if (value_fd == -1) {
        perror("Unable to read GPIO value");
        return -1;
    }
    char buffer[2];
    read(value_fd, buffer, sizeof(buffer));
    close(value_fd);
    return atoi(buffer);
}

//Function to handle a sensor IR
//Export, set direction to input, and read value from pin
//Print messages relevant to the output of sensor
void* handleSensor(void* args) {
    int pin = *((int*)args);
    if (exportGPIO(pin) == -1) {
        return NULL;
    }
    if (setGPIO(pin, "in") == -1) {
        return NULL;
    }

    while (1) {
        int sensorValue = readGPIO(pin);
        
       // Update obstacle flags and sensor-specific logic based on sensor readings
    if (pin == IR_SENSOR_FRONT) {
        obstacleFront = (sensorValue == 0);
        // Logic for front IR sensor
        printf("Front IR Sensor: %s\n", 
        sensorValue == 0 ? "Obstacle Detected" : "No Obstacle");
    } else if (pin == IR_SENSOR_LEFT) {
        obstacleLeft = (sensorValue == 0);
        // Logic for left IR sensor
        printf("Left IR Sensor: %s\n", 
        sensorValue == 0 ? "Obstacle Detected" : "No Obstacle");
    } else if (pin == IR_SENSOR_RIGHT) {
        obstacleRight = (sensorValue == 0);
        // Logic for right IR sensor
        printf("Right IR Sensor: %s\n", 
        sensorValue == 0 ? "Obstacle Detected" : "No Obstacle");
    }


        usleep(100000); // Sleep for 100ms
    }

    return NULL;
}
*/
