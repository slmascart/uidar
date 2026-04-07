#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "lcd.h"
#include "temperature.h"
#include "ultrasonic.h"
#include "util.h"

/*	- CONSTANTS -	*/
#define TRIG_PIN	4
#define ECHO_PIN	5
#define LCD_ADDR	0x27	/* DEFAULT LCD ADDRESS	*/
#define SPI_CHANNEL     0       /* CE0	*/
#define SPI_SPEED       1000000 /* 1MHz */

int	fd_lcd;

/*	- FUNCTIONS -	*/
void display(
	double temperature,
	double distance
) {
	int	i,
		s_temp,
		s_dist;
	char	temp_display[16] = {'\0'},
		dist_display[16] = {'\0'};

	sprintf(temp_display, "Temp: %0.1fC", temperature - 273.15);
	sprintf(dist_display, "Dist: %0.2fcm", distance);

	s_temp = strlen(temp_display);
	for(i = s_temp; i < 16; ++i)	temp_display[i] = ' ';
	s_dist = strlen(dist_display);
	for(i = s_dist; i < 16; ++i)	dist_display[i] = ' ';

	write(0, 0, temp_display, fd_lcd);
	write(0, 1, dist_display, fd_lcd);
}

static void sigint_callback_handler(int) {
	clear_lcd(fd_lcd);
	exit(0);
}

/*	- MAIN -	*/
int main(void) {
	double	kelvin_temp,
		distance,
		sound_speed;

	signal(SIGINT, sigint_callback_handler);

	/* Check for correct config and sudo call */
	if(check_wiringPi()) {
		printf("Setup wiringPi failed!!\n");
		return -1;
	}

	if(check_wiringPi_SPI(SPI_CHANNEL, SPI_SPEED)) {
		printf("SPI setup failed!\n");
		return -1;
	}

	/* INIT */
	fd_lcd = wiringPiI2CSetup(LCD_ADDR);
	init_lcd(fd_lcd);

	init_ultrasonic_module(TRIG_PIN, ECHO_PIN);

	/* MAIN LOOP */
	while(1) {
		kelvin_temp = compute_temperature(0, SPI_CHANNEL);	/* Read from CH0 */
		sound_speed = temp2speed_diadic_gas(kelvin_temp) * 100;	/* Speed of sound [cm/s] */
		distance = measure_distance(sound_speed, TRIG_PIN, ECHO_PIN);
		delay(150);
		display(kelvin_temp, distance);
		delay(200);
	}

	return 0;
}
