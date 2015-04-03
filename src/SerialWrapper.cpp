#include "SerialWrapper.h"

int SerialWrapper::serialOpen(char* port_address, int baud_rate) 
{
	// Open device:
	int fd;
	if( (fd = open(port_address, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
	{
		return -1;
	}	

	// If port opened, set up our parameters:
	struct termios options;

	//fcntl(fd, F_SETFL, FNDELAY); // non-blocking read
	fcntl(fd, F_SETFL,0);          // blocking read

	// Save current port settings so nothing is corrupt on exit:
	tcgetattr(fd, &options);
	cfmakeraw(&options);

	// Convert integer baud to baud type:
	// Default to 9600 baud if none specified:
	switch (baud_rate)
	{
		case 4800:
			cfsetispeed(&options, B4800); 
			cfsetospeed(&options, B4800);
			break;
		case 9600:
			cfsetispeed(&options, B9600); 
			cfsetospeed(&options, B9600);
			break;
		case 19200:
			cfsetispeed(&options, B19200); 
			cfsetospeed(&options, B19200);
			break;
		case 38400:	
			cfsetispeed(&options, B38400); 
			cfsetospeed(&options, B38400);
			break;
		case 57600:	
			cfsetispeed(&options, B57600); 
			cfsetospeed(&options, B57600);
			break;
		case 115200:	
			cfsetispeed(&options, B115200); 
			cfsetospeed(&options, B115200);
			break;
		case 230400:	
			cfsetispeed(&options, B230400); 
			cfsetospeed(&options, B230400);
			break;
		case 500000:	
			cfsetispeed(&options, B500000); 
			cfsetospeed(&options, B500000);
			break;
		default:
			cfsetispeed(&options, B9600); 
			cfsetospeed(&options, B9600);
			break;
	}

	// Set options for proper port settings:
	options.c_cflag |= (CLOCAL | CREAD);		

	// Turn off hardware flow control:
	options.c_cflag &= ~CRTSCTS;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	// Write our changes to the port configuration:
	tcsetattr(fd, TCSANOW, &options);

	return fd;
}

int SerialWrapper::serialFlush(int fd) 
{
	if(tcflush(fd,TCIOFLUSH) == -1)
	{
		return -1;
	}

	return 0;
}

int SerialWrapper::serialFlushWrite(int fd) 
{
	if(tcflush(fd,TCOFLUSH) == -1)
	{
		return -1;
	}

	return 0;
}

int SerialWrapper::serialFlushRead(int fd) 
{	
	if(tcflush(fd,TCIFLUSH) == -1)
	{
		return -1;
	}

	return 0;
}

int SerialWrapper::serialWrite(int fd, char* data, int length)
{		
	if(write(fd,data,length)==-1)
	{
	    return -1;
	}

	return 0;
}

int SerialWrapper::serialRead(int fd, char* temp, int length)
{
	int cnt;

	// Read:
	if((cnt = read(fd,temp,length))==-1)
	{
		return -1;
	}

	return cnt;
}

int SerialWrapper::serialCommandByDelim(int fd, char* cmd, char* reply, int replyLength, int usecs, char delim)
{
	// Timeout stuff:
	int bufferCount = 0;
	int index = 0;
	char temp;

	// Sleep for half a second before checking timeout, unless it should be smaller.
	int sleeptime = HALF_SECOND;
	int st = usecs / 10;
	if ( sleeptime > st )
	{
		sleeptime = st; 
	}

	// Create timeout timer:
	Timer timer;
	timer.timerCreate();
	timer.timerSet(usecs);

	// Flush the read port:
	if ( serialFlushRead(fd) < 0)
	{
		return -1;
	}

	// Write the command:
	if ( serialWrite(fd, cmd, strlen(cmd)) < 0)
	{
		return -1;
	}

	// Flush the write port:
	if ( serialFlushWrite(fd) < 0)
	{
		return -1;
	}

	// Wait for response:
	while( !timer.timerExpired() )
	{
		if( (bufferCount = serialBufferCount( fd )) > 0 )
		{
			if( serialRead( fd, &temp, 1 ) < 0 )
			{
				return -1;
			}

			// cout << hex << "0x" << (int) temp << endl;

			// Have we found the delimter yet?
			if( temp == delim )
			{
				return index;
			}

			reply[index] = temp;
			index++;

			if( index >= replyLength )
			{
				return index;
			}
		}
		else
		{
			// cout << "sweeping" << endl;
			rt_sleep( sleeptime );
		}
	}

	return index;
}

int SerialWrapper::serialCommandByLength(int fd, char* cmd, char* reply, int replyLength, int usecs)
{
	// Timeout stuff:
	int bufferCount = 0;

	// Sleep for half a second before checking timeout, unless it should be smaller.
	int sleeptime = HALF_SECOND;
	int st = usecs / 10;
	if ( sleeptime > st )
	{
		sleeptime = st; 
	}

	// Create timeout timer:
	Timer timer;
	timer.timerCreate();
	timer.timerSet(usecs);

	// Flush the read port:
	if ( serialFlushRead(fd) < 0)
	{
		return -1;
	}

	// Write the command:
	if ( serialWrite(fd, cmd, strlen(cmd)) < 0)
	{
		return -1;
	}

	// Flush the write port:
	if ( serialFlushWrite(fd) < 0)
	{
		return -1;
	}

	// Wait for response:
	while ( (bufferCount = serialBufferCount( fd )) < replyLength )
	{
		if( timer.timerExpired() )
		{
			bufferCount = serialBufferCount( fd );
			break;
		}

		rt_sleep( sleeptime );
	}

	// Make sure we don't read more data than was asked for...
	if( bufferCount > replyLength )
	{
		bufferCount = replyLength;
	}

	bufferCount = serialRead( fd, reply, bufferCount );

	return bufferCount;
}
int SerialWrapper::serialBufferCount(int fd)
{
	int count;

	// Get buffer count:
	ioctl(fd, FIONREAD, &count);

	return count;
}

int SerialWrapper::serialClose(int fd) 
{
	return close(fd);
}

bool SerialWrapper::serialAvailable(int fd)
{
	if(serialBufferCount>0)
		return true;
	else
		return false;
}
