#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "util.h"

int check_wiringPi(void) {
	if(wiringPiSetup() == -1)	return -1;
	else				return 0;
}

int check_wiringPi_SPI(
	int spi_channel,
	int spi_speed
) {
        if(wiringPiSPISetup(spi_channel, spi_speed) == -1)      return 1;
        else                                                    return 0;
}

