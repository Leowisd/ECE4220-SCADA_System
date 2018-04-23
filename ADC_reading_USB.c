#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>		// for the integer types
#include <wiringPi.h>
#include <wiringSerial.h>

int main()
{
	int fd;
	if ((fd = serialOpen("/dev/ttyACM0",9600))<0)
	{
		fprintf(stderr, "Unable to open serial device: %s\n", strerror(errno));
		return 1;
	}
	
	while(1)
	{
		char ch=serialGetchar(fd);
	        putchar(ch);				
		fflush(stdout);
		if (ch == '\n' ) sleep(1);
	}

	return 0;
}
