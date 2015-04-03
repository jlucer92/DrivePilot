
#ifndef _DRIVEPILOT_H
#define _DRIVEPILOT_H

#include "OBDWrapper.h"
#include "Adafruit_GPS.h"
#include "GSMWrapper.h"
#include <iostream>
#include <string>


class DrivePilot {
 public:
	DrivePilot(){}; // Constructor 
	bool parseData();
	void init();

 private:
	Adafruit_GPS GPS;
	GSMWrapper GSM;
	int fileD;
	OBDWrapper OBD; /* for Model A (UART version) */
	string data;
	char s[20];
	int did, throt, spd, rpm, alt, vid;
	uint32_t timer;
	bool debug;

	string getLatitude();
	string getLongitude();
	string getAltitude();
	string getTime();
	float DMStoDD(float DMS);
  
};


#endif