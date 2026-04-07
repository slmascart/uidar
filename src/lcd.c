#include <stdio.h>
#include <string.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "lcd.h"

/*	- CONSTANTS -	*/
#define LCD_ADDR	0x27	/* Defaulkt address, sometimes found at 0x3F */
#define SPI_CHANNEL	0	/* CE0  */
#define SPI_SPEED	1000000	/* 1MHz	*/

int	BLEN = 1;

/*	- FUNCTIONS -	*/
void write_word(
	int data,
	int fd
) {
	int temp = data;

	if(BLEN == 1)	temp |= 0x08;
	else		temp &= 0xF7;
	wiringPiI2CWrite(fd, temp);
}

void send_command(
	int command,
	int fd
) {
	int buffer;

	/* Send bit7-4 firstly */
	buffer = command & 0xF0;
	buffer |= 0x04;
	write_word(buffer, fd);
	delay(2);
	buffer &= 0xFB;
	write_word(buffer, fd);

	/* Send bit3-0 secondly */
	buffer = (command & 0x0F) << 4;
	buffer |= 0x04;
	write_word(buffer, fd);
	delay(2);
	buffer &= 0xFB;
	write_word(buffer, fd);
}

void send_data(
	int data,
	int fd
) {
	int buffer;

	/* Send bit7-4 firstly */
	buffer = data & 0xF0;
	buffer |= 0x05;		/* RS = 1, RW = 0, EN = 1 */
	write_word(buffer, fd);
	delay(2);
	buffer &= 0xFB;		/* Make EN = 0 */
	write_word(buffer, fd);

	/* Send bit3-0 secondly */
	buffer = (data & 0x0F) << 4;	/* RS = 1, RW = 0, EN = 1 */
	buffer |= 0x05;
	write_word(buffer, fd);
	delay(2);
	buffer &= 0xFB;		/* Make EN = 0 */
	write_word(buffer, fd);
}

void clear_lcd(
	int fd
) {
	send_command(0x01, fd);	/* Clear screen */
}

void init_lcd(
	int fd
) {
	send_command(0x33, fd);	/* Must initialize to 8-line mode at first */
	delay(5);
	send_command(0x32, fd);	/* Then initialize to 4-line mode */
	delay(5);
	send_command(0x28, fd);	/* 2 lines and 5x7 dots */
	delay(5);
	send_command(0x0C, fd);	/* Enable display without cursor */
	delay(5);
	clear_lcd(fd);
	wiringPiI2CWrite(fd, 0x08);
	delay(5);
}

void write(
	int x,
	int y,
	char* data,
	int fd
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
	send_command(address, fd);

	tmp = strlen(data);
	for(i = 0; i < tmp; ++i)	send_data(data[i], fd);
}
