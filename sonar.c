

void *readSonar(void *arg){
	/* turn on the trigger for 10us*/

	
	int triggerPin = 1;
	int echoPin = 0;
	int start, stop,duration;
    gpioWrite(triggerPin, HIGH);
    usleep(10);
    gpioWrite(triggerPin, LOW);

    while(gpioRead(echoPin) == LOW){
       // printf("checking for echoPin signal to start transmit\n");
    }
    
    // once the echoPin returns high, trasmitting signal back to the device basically catching the signal

    start = clock() / (1.0 * CLOCKS_PER_SEC);

    printf("start time: %f\n", start);
    while (gpioRead(echoPin) == HIGH){
     //   printf("echo signalling\n");
    }

    stop = clock();
    printf("stop time: %f\n", stop);

    
    stop = clock() / (1.0 * CLOCKS_PER_SEC);
    printf("stop time: %f\n", stop);
    printf("clocks_PER_SEC: %ld\n", CLOCKS_PER_SEC);
    duration = stop - start;
    printf("duration %f\n",duration);
    
    distance =  (soundVelocity * duration) / 2;

    printf("distance measured : %f\n", distance);

}
