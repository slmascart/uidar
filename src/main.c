#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <wiringPiSPI.h>

/*	- CONSTANTS -	*/
#define	PIN_TRIG	4
#define	PIN_ECHO	5
#define LCD_ADDR	0x27	/* Defaulkt address, sometimes found at 0x3F */
#define SPI_CHANNEL	0	/* CE0  */
#define SPI_SPEED	1000000	/* 1MHz	*/

int	BLEN = 1;
int	fd;
bool	logging = false;

/*	- FUNCTIONS -	*/
void init_ultrasonic_module(void) {
	pinMode(PIN_ECHO, INPUT);
	pinMode(PIN_TRIG, OUTPUT);
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
	double sound_speed
) {
	double		dt_sec;
	struct timeval	start_wait,
			curr_wait,
			sig_sent,
			echo_received;
	float		wait_time;
	int		dr;

	if(logging)	printf("\nMEASURING DISTANCE\n"); fflush(stdout);

	if(logging)	printf("- Setting TRIG PIN on LOW\n"); fflush(stdout);
	digitalWrite(PIN_TRIG, LOW);
	delayMicroseconds(10);

	if(logging)	printf("- Setting TRIG PIN on HIGH then LOW\n"); fflush(stdout);
	digitalWrite(PIN_TRIG, HIGH);
	delayMicroseconds(20);
	digitalWrite(PIN_TRIG, LOW);

	if(logging)	printf("- Wait for no interfering ECHO signal and get time\n"); fflush(stdout);
	gettimeofday(&start_wait, NULL);
	do {
		gettimeofday(&curr_wait, NULL);
		wait_time = (float)(curr_wait.tv_sec - start_wait.tv_sec) + (float)(curr_wait.tv_usec - start_wait.tv_usec) / 1000000.0;
		dr = digitalRead(PIN_ECHO);
		// if(logging)	printf("Waited %f, reading %d\n", wait_time, dr);
	} while(dr != 1 && wait_time < .5);
	if(wait_time >= .5) {
		if(logging)	printf("- Max waiting time reached, returning -1\n");
		return -1;
	}
	gettimeofday(&sig_sent, NULL);

	if(logging)	printf("- Wait for receiving ECHO signal and get time\n"); fflush(stdout);
	gettimeofday(&start_wait, NULL);
	do {
		gettimeofday(&curr_wait, NULL);
		wait_time = (float)(curr_wait.tv_sec - start_wait.tv_sec) + (float)(curr_wait.tv_usec - start_wait.tv_usec) / 1000000.0;
		dr = digitalRead(PIN_ECHO);
		// if(logging)	printf("Waited %f, reading %d\n", wait_time, dr);
	} while(dr != 0 && wait_time < .5);
	if(wait_time >= .5) {
		if(logging)	printf("- Max waiting time reached, returning -1\n");
		return -1;
	}
	gettimeofday(&echo_received, NULL);

	if(logging)	printf("-Compute distance\n");
	dt_sec = (double)(echo_received.tv_sec - sig_sent.tv_sec);
	dt_sec += (double)(echo_received.tv_usec - sig_sent.tv_usec) / 1000000.0;

	return dt_sec * sound_speed / 2;
}

void write_word(
	int data
) {
	int temp = data;

	if(BLEN == 1)	temp |= 0x08;
	else		temp &= 0xF7;
	wiringPiI2CWrite(fd, temp);
}

void send_command(
	int command
) {
	int buffer;

	/* Send bit7-4 firstly */
	buffer = command & 0xF0;
	buffer |= 0x04;
	write_word(buffer);
	delay(2);
	buffer &= 0xFB;
	write_word(buffer);

	/* Send bit3-0 secondly */
	buffer = (command & 0x0F) << 4;
	buffer |= 0x04;
	write_word(buffer);
	delay(2);
	buffer &= 0xFB;
	write_word(buffer);
}

void send_data(
	int data
) {
	int buffer;

	/* Send bit7-4 firstly */
	buffer = data & 0xF0;
	buffer |= 0x05;		/* RS = 1, RW = 0, EN = 1 */
	write_word(buffer);
	delay(2);
	buffer &= 0xFB;		/* Make EN = 0 */
	write_word(buffer);

	/* Send bit3-0 secondly */
	buffer = (data & 0x0F) << 4;	/* RS = 1, RW = 0, EN = 1 */
	buffer |= 0x05;
	write_word(buffer);
	delay(2);
	buffer &= 0xFB;		/* Make EN = 0 */
	write_word(buffer);
}

void clear(void) {
	send_command(0x01);	/* Clear screen */
}

void init_lcd(void) {
	send_command(0x33);	/* Must initialize to 8-line mode at first */
	delay(5);
	send_command(0x32);	/* Then initialize to 4-line mode */
	delay(5);
	send_command(0x28);	/* 2 lines and 5x7 dots */
	delay(5);
	send_command(0x0C);	/* Enable display without cursor */
	delay(5);
	clear();
	wiringPiI2CWrite(fd, 0x08);
	delay(5);
}

void write(
	int x,
	int y,
	char* data
) {
	int	address,
		i,
		tmp;

	if(x < 0)	x = 0;
	if(x > 15)	x = 15;
	if(y < 0)	y = 0;
	if(y > 1)	y = 1;

	/* Move cursor */
	address = 0x80 + 0x40 * y + x;
	send_command(address);

	tmp = strlen(data);
	for(i = 0; i < tmp; ++i)	send_data(data[i]);
}

int read_adc(
	int channel
) {
	unsigned char buffer[3];

	if(channel < 0 || channel > 27)	return -1;

	buffer[0] = 1; /* Start bit */
	buffer[1] = (8 + channel) << 4; /* Single-ended mode + channel */
	buffer[2] = 0;

	wiringPiSPIDataRW(SPI_CHANNEL, buffer, 3);
	return ((buffer[1] & 3) << 8) | buffer[2];
}

double compute_temperature(void) {
	int	analog_temperature;
	double	Vref,
		Vratio;

	analog_temperature = read_adc(0); /* Read from CH0 */

	/* MCP3008 is 10-bit ADC (0-1023) */
	Vref = 3.3 * analog_temperature / 1023.0; /* Assuming Vref == 3.3V */
	Vratio = Vref / (3.3 - Vref); /* Voltage divider, 10k resistor */
	return 1.0 / ((log(Vratio) / 3950.0) + (1.0 / (273.15 + 25.0)));
}

/*	- MAIN -	*/
int main(void) {
	double	kelvin_temp,
		distance,
		sound_speed;
	int	i;
	char	dist_value[9]		= {'\0'},
		temp_value[6]		= {'\0'},
		n_spaces[9]		= {'\0'},
		dist_temp_values[16]	= {'\0'};

	/* Check for correct config and sudo call */
	if(wiringPiSetup() == -1) {
		printf("Setup wiringPi failed!!\n");
		return 1;
	}

	if(wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1) {
		printf("SPI setup failed!\n");
		return 1;
	}

	/* INIT */
	if(logging)	printf("- Loading LCD on memory\n"); fflush(stdout);
	fd = wiringPiI2CSetup(LCD_ADDR);
	if(logging)	printf("- Init LCD\n"); fflush(stdout);
	init_lcd();
	if(logging)	printf("- Init ultrasonic module\n"); fflush(stdout);
	init_ultrasonic_module();

	/* MAIN LOOP */
	while(1) {
		kelvin_temp = compute_temperature();
		sound_speed = temp2speed_diadic_gas(kelvin_temp) * 100;	/* Speed of sound [cm/s] */
		distance = measure_distance(sound_speed);
		delay(100);
		sprintf(dist_value, "%0.2fcm", distance);
		sprintf(temp_value, "%0.1fC", kelvin_temp - 273.15);
		for(i = 0; i < 9; ++i)	printf("%c", dist_value[i]);
		printf("\n");
		for(i = 0; i < 6; ++i)	printf("%c", temp_value[i]);
		printf("\n");
		if(logging)	printf("Dist+Temp %s / %s, #spaces %d\n", dist_value, temp_value, 9-strlen(dist_value));
		for(i = 0; i < 9 - strlen(dist_value) - 1; ++i)	n_spaces[i] = ' ';
		for(; i < 9; ++i)				n_spaces[i] = '\0';
		printf("%s%s / %s\n", dist_value, n_spaces, temp_value);
		sprintf(dist_temp_values, "%s%s / %s", dist_value, n_spaces, temp_value);
		if(logging)	printf("\nDistance / Temperature: %s\n", dist_temp_values);
		write(0, 0, "Distance / Temp");
		write(0, 1, dist_temp_values);
		delay(200);
	}

	return 0;
}
