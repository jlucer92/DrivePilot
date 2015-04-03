#include "DrivePilot.h"

void DrivePilot::init(){
	debug = false;
	did=throt=spd=rpm=alt=vid=0;
	// timer = millis();
	// powerGSM();
	//Serial2.println("AT+IPR=38400");
	GPS.begin("/dev/ttyO5",9600);
	GSM.begin("/dev/ttyO1",38400);
	OBD.begin("/dev/ttyO2");
	
	GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA,sizeof(PMTK_SET_NMEA_OUTPUT_RMCGGA));
	GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ,sizeof(PMTK_SET_NMEA_UPDATE_1HZ));
	GPS.sendCommand(PGCMD_ANTENNA,sizeof(PGCMD_ANTENNA));
	// Serial3.println(PMTK_Q_RELEASE);
	GPS.sendCommand(PMTK_Q_RELEASE,sizeof(PMTK_Q_RELEASE));
	cout<<"DP: Constructor Initiailized"<<endl;
}


bool DrivePilot::parseData(){
	GPS.read();
	
	if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return false; // we can fail to parse a sentence in which case we should just wait for another
	}
	else{
		return false;
	}
	// int index = 0;
	// while(1){
	// GSM.sendData("LONG STRING OF DATA LA LA LA LA LA LA "+to_string(index)+'\n');
	// sleep(1);
	// index++;
	// }
	if(GPS.fix){
	cout<<"GPS Fixed"<<endl;
	data += "data ";
	if(did==0){
		srand(atoi(getTime().c_str()));
		did = rand() % 200000 + 1;
	}
	data += to_string(did)+" ";
		data += getTime() + " ";
		data += getAltitude() + " ";
		data += getLatitude() + " ";//lat
		data += getLongitude() + " ";//lon

		
		data+= to_string(OBD.alphaData.Speed) + " ";
		data+= to_string(OBD.alphaData.RPM) + " ";
		data+= to_string(OBD.alphaData.MAF) + " ";

		cout<<"ATTEMPTING TO SEND DATA..."<<endl;
		if(OBD.lowPowerState)
			GSM.sendData(data+"\n", false);
		else
			GSM.sendData(data+"\n", true);
		data.clear();
		sleep(1);
		// cout<<data<<endl;
		return true;
	}
	return false;
}

string DrivePilot::getLatitude(){
	float latitude = DMStoDD(GPS.latitude);
	// Serial.println(GPS.latitude,4);
	if(GPS.lat=='S')
		latitude *= -1;
	return to_string(latitude);
}
string DrivePilot::getLongitude(){
	float longitude = DMStoDD(GPS.longitude);
	// Serial.println(GPS.longitude,4);
	if(GPS.lon=='W')
		longitude *= -1;
	return to_string(longitude);
}
string DrivePilot::getAltitude(){
	int altitude = GPS.altitude;
	return to_string(altitude);
}
string DrivePilot::getTime(){
  struct tm timeObject;
  time_t t_of_day;
  timeObject.tm_year = GPS.year+100;
  timeObject.tm_mon = GPS.month-1;
  timeObject.tm_mday = GPS.day;
  timeObject.tm_hour = GPS.hour-6;
  timeObject.tm_min = GPS.minute;
  timeObject.tm_sec = GPS.seconds;
  timeObject.tm_isdst = -1;
  t_of_day = mktime(&timeObject);
  long epoch = ((long)t_of_day);
  return to_string(epoch);
}

float DrivePilot::DMStoDD(float DMS){
	int degrees = DMS/100;
	float minutes = DMS - ((float)degrees*100.f);
	return (float)degrees + (minutes/60.f);
}