/***********************************
This is our GPS library

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.
BSD license, check license.txt for more information
All text above must be included in any redistribution
****************************************/

#include <Adafruit_GPS.h>

// how long are max NMEA lines to parse?
#define MAXLINELENGTH 120

// we double buffer: read one line in and leave one for the main program
volatile char line1[MAXLINELENGTH];
volatile char line2[MAXLINELENGTH];
// our index into filling the current line
volatile uint8_t lineidx=0;
// pointers to the double buffers
volatile char *currentline;
volatile char *lastline;
volatile bool recvdflag;
volatile bool inStandbyMode;


bool Adafruit_GPS::parse(char *nmea) {
  // do checksum check
  // first look if we even have one
  if (nmea[strlen(nmea)-4] == '*') {
    uint16_t sum = parseHex(nmea[strlen(nmea)-3]) * 16;
    sum += parseHex(nmea[strlen(nmea)-2]);
    
    // check checksum 
    for (uint8_t i=1; i < (strlen(nmea)-4); i++) {
      sum ^= nmea[i];
    }
    if (sum != 0) {
      // bad checksum :(
      return false;
    }
	int commas = 0;
  // look for a few common sentences
  if (strstr(nmea, "$GPGGA")) {
	for (uint8_t i=0; i < strlen(nmea); i++) {	
      if(nmea[i]==',')
		commas++;
    }
	if(commas != 14)
		return false;
    // found GGA
    char *p = nmea;
    // get time
    if((p = strchr(p, ','))!=NULL){
		p++;
		float timef = atof(p);
		uint32_t time = timef;
		hour = time / 10000;
		minute = (time % 10000) / 100;
		seconds = (time % 100);

		milliseconds = fmod(timef, 1.0) * 1000;
	}
    // parse out latitude
    if((p = strchr(p, ','))!=NULL){
		p++;
		latitude = atof(p);
	}
	if((p = strchr(p, ','))!=NULL){
		p++;
		if (p[0] == 'N') lat = 'N';
		else if (p[0] == 'S') lat = 'S';
		else if (p[0] == ',') lat = 0;
		else return false;
	}
    // parse out longitude
    if((p = strchr(p, ','))!=NULL){
		p++;
		longitude = atof(p);
	}
	
    if((p = strchr(p, ','))!=NULL){
		p++;
		if (p[0] == 'W') lon = 'W';
		else if (p[0] == 'E') lon = 'E';
		else if (p[0] == ',') lon = 0;
		else return false;
	}

    if((p = strchr(p, ','))!=NULL){
		p++;
		fixquality = atoi(p);
	}
    if((p = strchr(p, ','))!=NULL){
		p++;
		satellites = atoi(p);
	}
    if((p = strchr(p, ','))!=NULL){
		p++;
		HDOP = atof(p);
	}
	
    if((p = strchr(p, ','))!=NULL){
		p++;
		altitude = atof(p);
	}
    if((p = strchr(p, ','))!=NULL){
		p++;
	}
    if((p = strchr(p, ','))!=NULL){
		p++;
		geoidheight = atof(p);
	}
    return true;
  }
  if (strstr(nmea, "$GPRMC")) {
	for (uint8_t i=0; i < strlen(nmea); i++) {	
      if(nmea[i]==',')
		commas++;
    }
	if(commas != 12)
		return false;
   // found RMC
    char *p = nmea;
    // get time
    if((p = strchr(p, ','))!=NULL){
		p++;
		float timef = atof(p);
		uint32_t time = timef;
		hour = time / 10000;
		minute = (time % 10000) / 100;
		seconds = (time % 100);

		milliseconds = fmod(timef, 1.0) * 1000;
	}
    if((p = strchr(p, ','))!=NULL){
		p++;
		// Serial.println(p);
		if (p[0] == 'A') 
		  fix = true;
		else if (p[0] == 'V')
		  fix = false;
		else
		  return false;
	}
    // parse out latitude
    if((p = strchr(p, ','))!=NULL){
		p++;
		latitude = atof(p);
	}
    if((p = strchr(p, ','))!=NULL){
		p++;
		if (p[0] == 'N') lat = 'N';
		else if (p[0] == 'S') lat = 'S';
		else if (p[0] == ',') lat = 0;
		else return false;
	}
    // parse out longitude
    if((p = strchr(p, ','))!=NULL){
		p++;
		longitude = atof(p);
	}
    if((p = strchr(p, ','))!=NULL){
		p++;
		if (p[0] == 'W') lon = 'W';
		else if (p[0] == 'E') lon = 'E';
		else if (p[0] == ',') lon = 0;
		else return false;
	}
    // speed
    if((p = strchr(p, ','))!=NULL){
		p++;
		speed = atof(p);
	}
    // angle
    if((p = strchr(p, ','))!=NULL){
		p++;
		angle = atof(p);
	}
    if((p = strchr(p, ','))!=NULL){
		p++;
		uint32_t fulldate = atof(p);
		day = fulldate / 10000;
		month = (fulldate % 10000) / 100;
		year = (fulldate % 100);
	}
    // we dont parse the remaining, yet!
    return true;
  }
  }

  return false;
}

char Adafruit_GPS::read(void) {
  char c = 0;
  
  if (paused) return c;
	
  if(!gpsSerial->serialAvailable(fileDescriptor)) return c;
	gpsSerial->serialRead(fileDescriptor,&c,1);

  //Serial.print(c);

  if (c == '$') {
    currentline[lineidx] = 0;
    lineidx = 0;
  }
  if (c == '\n') {
    currentline[lineidx] = 0;

    if (currentline == line1) {
      currentline = line2;
      lastline = line1;
    } else {
      currentline = line1;
      lastline = line2;
    }

    //Serial.println("----");
    //Serial.println((char *)lastline);
    //Serial.println("----");
    lineidx = 0;
    recvdflag = true;
  }

  currentline[lineidx++] = c;
  if (lineidx >= MAXLINELENGTH)
    lineidx = MAXLINELENGTH-1;

  return c;
}


// Constructor when using HardwareSerial
Adafruit_GPS::Adafruit_GPS() {
  common_init();  // Set everything to common state, then...
}

// Initialization code used by all constructor types
void Adafruit_GPS::common_init(void) {
  gpsSerial = new SerialWrapper(); // Set both to NULL, then override correct
  recvdflag   = false;
  paused      = false;
  lineidx     = 0;
  currentline = line1;
  lastline    = line2;

  hour = minute = seconds = year = month = day =
    fixquality = satellites = 0; // uint8_t
  lat = lon = mag = 0; // char
  fix = false; // bool
  milliseconds = 0; // uint16_t
  latitude = longitude = geoidheight = altitude =
    speed = angle = magvariation = HDOP = 0.0; // float
}

void Adafruit_GPS::begin(char* port_address,uint16_t baud)
{

	if( (fileDescriptor = gpsSerial->serialOpen(port_address, baud)) < 0)
	{      
		cout << "could not open port!" << endl;
		return;
	}
	else{
		cout << "serial opened successfully, fd: " << fileDescriptor << endl;
	}

  //sleep(10);
}

void Adafruit_GPS::sendCommand(char *str, int length) {
  gpsSerial->serialWrite(fileDescriptor,str,length);
}

bool Adafruit_GPS::newNMEAreceived(void) {
  return recvdflag;
}

void Adafruit_GPS::pause(bool p) {
  paused = p;
}

char *Adafruit_GPS::lastNMEA(void) {
  recvdflag = false;
  return (char *)lastline;
}

// read a Hex value and return the decimal equivalent
uint8_t Adafruit_GPS::parseHex(char c) {
    if (c < '0')
      return 0;
    if (c <= '9')
      return c - '0';
    if (c < 'A')
       return 0;
    if (c <= 'F')
       return (c - 'A')+10;
	   
	return 0;
}

bool Adafruit_GPS::waitForSentence(char *wait4me, uint8_t max) {
  char str[20];

  uint8_t i=0;
  while (i < max) {
    if (newNMEAreceived()) { 
      char *nmea = lastNMEA();
      strncpy(str, nmea, 20);
      str[19] = 0;
      i++;

      if (strstr(str, wait4me))
	return true;
    }
  }

  return false;
}

bool Adafruit_GPS::LOCUS_StartLogger(void) {
  sendCommand(PMTK_LOCUS_STARTLOG,sizeof(PMTK_LOCUS_STARTLOG));
  recvdflag = false;
  return waitForSentence(PMTK_LOCUS_LOGSTARTED);
}

bool Adafruit_GPS::LOCUS_ReadStatus(void) {
  sendCommand(PMTK_LOCUS_QUERY_STATUS, sizeof(PMTK_LOCUS_QUERY_STATUS));
  
  if (! waitForSentence("$PMTKLOG"))
    return false;

  char *response = lastNMEA();
  uint16_t parsed[10];
  uint8_t i;
  
  for (i=0; i<10; i++) parsed[i] = -1;
  
  response = strchr(response, ',');
  for (i=0; i<10; i++) {
    if (!response || (response[0] == 0) || (response[0] == '*')) 
      break;
    response++;
    parsed[i]=0;
    while ((response[0] != ',') && 
	   (response[0] != '*') && (response[0] != 0)) {
      parsed[i] *= 10;
      char c = response[0];
      if (isdigit(c))
        parsed[i] += c - '0';
      else
        parsed[i] = c;
      response++;
    }
  }
  LOCUS_serial = parsed[0];
  LOCUS_type = parsed[1];
  if (isalpha(parsed[2])) {
    parsed[2] = parsed[2] - 'a' + 10; 
  }
  LOCUS_mode = parsed[2];
  LOCUS_config = parsed[3];
  LOCUS_interval = parsed[4];
  LOCUS_distance = parsed[5];
  LOCUS_speed = parsed[6];
  LOCUS_status = !parsed[7];
  LOCUS_records = parsed[8];
  LOCUS_percent = parsed[9];

  return true;
}

// Standby Mode Switches
bool Adafruit_GPS::standby(void) {
  if (inStandbyMode) {
    return false;  // Returns false if already in standby mode, so that you do not wake it up by sending commands to GPS
  }
  else {
    inStandbyMode = true;
    sendCommand(PMTK_STANDBY, sizeof(PMTK_STANDBY));
    //return waitForSentence(PMTK_STANDBY_SUCCESS);  // don't seem to be fast enough to catch the message, or something else just is not working
    return true;
  }
}

bool Adafruit_GPS::wakeup(void) {
  if (inStandbyMode) {
   inStandbyMode = false;
    sendCommand("",1);  // send byte to wake it up
    return waitForSentence(PMTK_AWAKE);
  }
  else {
      return false;  // Returns false if not in standby mode, nothing to wakeup
  }
}