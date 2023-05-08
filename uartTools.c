#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "wiringSerial.h"

int myserialOpen (const char *device, const int baud)
{
	struct termios options ;
	speed_t myBaud ;
	int     status, fd ;
	switch(baud)
	{
		case    9600:	myBaud =    B9600 ; break ;
		case  115200:	myBaud =  B115200 ; break ;
		default:
						return -2 ;
	}

	if ((fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK)) == -1)
		return -1 ;
	fcntl (fd, F_SETFL, O_RDWR) ;

	// Get and modify current options:

	tcgetattr (fd, &options) ;

	cfmakeraw   (&options) ;
	cfsetispeed (&options, myBaud) ;
	cfsetospeed (&options, myBaud) ;

	options.c_cflag |= (CLOCAL | CREAD) ;
	options.c_cflag &= ~PARENB ;
	options.c_cflag &= ~CSTOPB ;
	options.c_cflag &= ~CSIZE ;
	options.c_cflag |= CS8 ;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG) ;
	options.c_oflag &= ~OPOST ;

	options.c_cc [VMIN]  =   0 ;
	options.c_cc [VTIME] = 100 ;	// Ten seconds (100 deciseconds)

	tcsetattr (fd, TCSANOW, &options) ;

	ioctl (fd, TIOCMGET, &status);

	status |= TIOCM_DTR ;
	status |= TIOCM_RTS ;

	ioctl (fd, TIOCMSET, &status);

	usleep (10000) ;	// 10mS

	return fd ;

}

int serial_Get_char_one(const int fd)
{
    char ch;
    read(fd,&ch,1);

    return ch;
}


int serialGetString (const int fd,char* s)
{
    return read (fd, s, 32);
}

int serialDataAvail (const int fd)
{
  int result ;
	
  if (ioctl (fd, FIONREAD, &result) == -1)return -1 ;

  printf("result=%d\n",result);
	

	

  return result ;
}











