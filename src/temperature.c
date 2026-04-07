#include <math.h>
#include <wiringPiSPI.h>

#include "temperature.h"

/*	- FUNCTIONS -	*/
int read_adc(
	int channel,
	int spi_channel
) {
	unsigned char buffer[3];

	if(channel < 0 || channel > 27)	return -1;

	buffer[0] = 1; /* Start bit */
	buffer[1] = (8 + channel) << 4; /* Single-ended mode + channel */
	buffer[2] = 0;

	wiringPiSPIDataRW(spi_channel, buffer, 3);
	return ((buffer[1] & 3) << 8) | buffer[2];
}

double compute_temperature(
	int channel,
	int spi_channel
) {
	int	analog_temperature;
	double	Vref,
		Vratio;

	analog_temperature = read_adc(channel, spi_channel);

	/* MCP3008 is 10-bit ADC (0-1023) */
	Vref = 3.3 * analog_temperature / 1023.0; /* Assuming Vref == 3.3V */
	Vratio = Vref / (3.3 - Vref); /* Voltage divider, 10k resistor */
	return 1.0 / ((log(Vratio) / 3950.0) + (1.0 / (273.15 + 25.0)));
}
