// SerialWrapper.h
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef SERIALWRAPPER_H     // Prevent duplicate definition
#define SERIALWRAPPER_H

//Include:
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <stropts.h>
#include <istream>
#include <fstream>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <termios.h>
#include <sstream>
#include "Timer.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

// Debug:
#define DEBUG_PORT 0


using namespace std;

class SerialWrapper
{
public:
	// Public Functions:
	int serialOpen(char* port_address, int baud_rate);  
	int serialFlush(int fd);
	int serialFlushRead(int fd);
	int serialFlushWrite(int fd);
	int serialWrite(int fd, char* data, int length);
	int serialRead(int fd, char* temp, int length);
	int serialCommandByDelim(int fd, char* cmd, char* reply, int replyLength, int usecs=HALF_SECOND, char delim='\n');
	int serialCommandByLength(int fd, char* cmd, char* reply, int replyLength, int usecs=HALF_SECOND);
    int serialBufferCount(int fd);
	int serialClose(int fd);
	bool serialAvailable(int fd);
};
#endif