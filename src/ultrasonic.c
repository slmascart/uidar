#include <math.h>
#include <stdio.h>
#include <sys/time.h>

#include <wiringPi.h>

#include "ultrasonic.h"

/*	- FUNCTIONS -	*/
void init_ultrasonic_module(
	int trig_pin,
	int echo_pin
) {
	pinMode(trig_pin, OUTPUT);
	pinMode(echo_pin, INPUT);
}

double temp2speed(
	double temperature,
	double gamma
) {
	double	R	= 8.31446261815324,
		Mair	= 0.0289647;

	return sqrt(gamma * R / Mair * temperature);
}

double temp2speed_diadic_gas(
	double temperature
) {
	return temp2speed(temperature, 1.4);
}

double measure_distance(
	double sound_speed,
	int trig_pin,
	int echo_pin
) {
	double		dt_sec;
	struct timeval	start_wait,
			curr_wait,
			sig_sent,
			echo_received;
	float		wait_time;
	int		dr;

	digitalWrite(trig_pin, LOW);
	delayMicroseconds(10);

	digitalWrite(trig_pin, HIGH);
	delayMicroseconds(20);
	digitalWrite(trig_pin, LOW);

	gettimeofday(&start_wait, NULL);
	do {
		gettimeofday(&curr_wait, NULL);
		wait_time = (float)(curr_wait.tv_sec - start_wait.tv_sec) + (float)(curr_wait.tv_usec - start_wait.tv_usec) / 1000000.0;
		dr = digitalRead(echo_pin);
	} while(dr != 1 && wait_time < .5);
	if(wait_time >= .5) {
		printf("- Max waiting time reached, returning -1\n");
		return -1;
	}
	gettimeofday(&sig_sent, NULL);

	gettimeofday(&start_wait, NULL);
	do {
		gettimeofday(&curr_wait, NULL);
		wait_time = (float)(curr_wait.tv_sec - start_wait.tv_sec) + (float)(curr_wait.tv_usec - start_wait.tv_usec) / 1000000.0;
		dr = digitalRead(echo_pin);
	} while(dr != 0 && wait_time < .5);
	if(wait_time >= .5) {
		printf("- Max waiting time reached, returning -1\n");
		return -1;
	}
	gettimeofday(&echo_received, NULL);

	dt_sec = (double)(echo_received.tv_sec - sig_sent.tv_sec);
	dt_sec += (double)(echo_received.tv_usec - sig_sent.tv_usec) / 1000000.0;

	return dt_sec * sound_speed / 2;
}
