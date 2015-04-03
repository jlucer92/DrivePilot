#include "OBDWrapper.h"

unsigned int hex2uint16(const char *p)
{
	char c = *p;
	unsigned int i = 0;
	for (char n = 0; c && n < 4; c = *(++p)) {
		if (c >= 'A' && c <= 'F') {
			c -= 7;
		} else if (c>='a' && c<='f') {
			c -= 39;
        } else if (c == ' ') {
		continue;
        } else if (c < '0' || c > '9') {
		break;
        }
		i = (i << 4) | (c & 0xF);
		n++;
	}
	return i;
}

unsigned char hex2uint8(const char *p)
{
	unsigned char c1 = *p;
	unsigned char c2 = *(p + 1);
	if (c1 >= 'A' && c1 <= 'F')
		c1 -= 7;
	else if (c1 >='a' && c1 <= 'f')
		c1 -= 39;
	else if (c1 < '0' || c1 > '9')
		return 0;

	if (c2 >= 'A' && c2 <= 'F')
		c2 -= 7;
	else if (c2 >= 'a' && c2 <= 'f')
		c2 -= 39;
	else if (c2 < '0' || c2 > '9')
		return 0;

	return c1 << 4 | (c2 & 0xf);
}




int OBDWrapper::normalizeData(char pid, char* data)
{
	int result;
	switch (pid) {
	case PID_SPEED:
		result = (int)((float)(getLargeValue(data))*0.621371);
		break;
	case PID_RPM:
		result = getLargeValue(data) >> 2;
		break;
	case PID_FUEL_PRESSURE:
		result = getSmallValue(data) * 3;
		break;
	case PID_COOLANT_TEMP:
	case PID_INTAKE_TEMP:
	case PID_AMBIENT_TEMP:
	case PID_ENGINE_OIL_TEMP:
		result = getTemperatureValue(data);
		break;
	case PID_THROTTLE:
	case PID_ENGINE_LOAD:
	case PID_FUEL_LEVEL:
	case PID_ABSOLUTE_ENGINE_LOAD:
	case PID_ETHANOL_PERCENTAGE:
	case PID_HYBRID_BATTERY_PERCENTAGE:
		result = getPercentageValue(data);
		break;
	case PID_MAF_FLOW:
		result = getLargeValue(data) / 100;
		break;
	case PID_TIMING_ADVANCE:
		result = (int)(getSmallValue(data) / 2) - 64;
		break;
	case PID_DISTANCE: // km
	case PID_RUNTIME: // second
	case PID_FUEL_RAIL_PRESSURE: // kPa
	case PID_ENGINE_REF_TORQUE: // Nm
		result = getLargeValue(data);
		break;
	case PID_CONTROL_MODULE_VOLTAGE: // V
		result = getLargeValue(data) / 1000;
		break;
	case PID_ENGINE_FUEL_RATE: // L/h
		result = getLargeValue(data) / 20;
		break;
	case PID_ENGINE_TORQUE_PERCENTAGE: // %
		result = (int)getSmallValue(data) - 125;
		break;
	case PID_VIN:
		result = getLargeValue(data);
	default:
		result = getSmallValue(data);
	}
	return result;
}




void OBDWrapper::begin(char* port_address)
{
	state = UNINITIALIZED;
	commandSent = false;
	responseReceived = false;
	lowPowerState = false;
	running = true;
	alphaData.Speed = -1;
	alphaData.RPM = -1;
	alphaData.MAF = -1;
	alphaData.VIN = -1;
	if( (fileD = obdSerial.serialOpen(port_address,OBD_SERIAL_BAUDRATE)) < 0)
	{      
		cout << "could not open port!" << endl;
		return;
	}
	else{
		cout << "serial opened successfully, fd: " << fileD << endl;
	}
	
	obdSerial.serialFlushRead(fileD);
	obdSerial.serialFlushWrite(fileD);
	
	connectionThread = new thread(&OBDWrapper::init,this);
}

char OBDWrapper::read()
{
	char c = 0;

	if(!obdSerial.serialAvailable(fileD)){return c;}
	obdSerial.serialRead(fileD,&c,1);

	//Serial.print(c);

	dataIN += c;
	//cout<<hex<<int(c)<<endl;
	if (c==0xD&&dataIN.size()>2) {
	
		if(state == UNINITIALIZED){
			if(dataIN.find("LM327 v1.3a")!=string::npos){
			// if(dataIN.find("OBDUART v1.0")!=string::npos){
				responseReceived = true;
				cout<<dataIN<<endl;
				obdSerial.serialFlushRead(fileD);
			}
			else{
				commandSent = false;
				cout<<dataIN<<endl;
				dataIN.clear();
				obdSerial.serialFlushRead(fileD);
			}
		}
		else if(state == RESET){
			if(dataIN.find("OK")!=string::npos){
				responseReceived = true;
				cout<<dataIN<<endl;
				obdSerial.serialFlushRead(fileD);
			}
			else if(dataIN.find("ATE0")!=string::npos){
				cout<<dataIN<<endl;
				dataIN.clear();
			}
			else if(dataIN.find("LM327 v1.3a")!=string::npos){
				commandSent = false;
				state = UNINITIALIZED;
				cout<<dataIN<<endl;
				dataIN.clear();
				obdSerial.serialFlushRead(fileD);
			}
			else{
				cout<<dataIN<<endl;
				dataIN.clear();
			}
		}
		else if(state == ECHOOFF){
			if(dataIN.find("OK")!=string::npos){
				responseReceived = true;
				cout<<dataIN<<endl;
				obdSerial.serialFlushRead(fileD);
			}
			else{
				cout<<dataIN<<endl;
				dataIN.clear();
			}
		}
		else if(state == CONNECTED){
				cout<<dataIN<<endl;
				dataIN.clear();
		}
	}
	return c;
}



void OBDWrapper::init()
{
	while(running){
	
	if(state != CONNECTED)
		read();
		
		if(state == UNINITIALIZED){
			if(!commandSent){
				if(obdSerial.serialWrite(fileD, SETRESET, sizeof(SETRESET))==0){
					commandSent = true;
					cout<<"SENT COMMAND: "<<SETRESET<<endl;
				}
			}
			else{
				if(responseReceived){
					state = RESET;
					commandSent = false;
					responseReceived=false;
					dataIN.clear();
				}
			}
		}
		else if(state == RESET){
			if(!commandSent){
				if(obdSerial.serialWrite(fileD, SETECHO, sizeof(SETECHO))==0){
					commandSent = true;
					cout<<"SENT COMMAND: "<<SETECHO<<endl;
				}
			}
			else{
				if(responseReceived){
					state = ECHOOFF;
					commandSent = false;
					responseReceived=false;
					dataIN.clear();
				}
			}
		}
		else if(state == ECHOOFF){
			if(!commandSent){
				if(obdSerial.serialWrite(fileD, SETNEWLINE, sizeof(SETNEWLINE))==0){
					commandSent = true;
					cout<<"SENT COMMAND: "<<SETNEWLINE<<endl;
				}
			}
			else{
				if(responseReceived){
					state = GETVIN;
					commandSent = false;
					responseReceived=false;
					dataIN.clear();
				}
			}
		}
		else if(state == GETVIN){
			// while((alphaData.VIN = getData(PID_VIN,0x09))<0);
			// alphaData.VIN = getData(PID_VIN,0x09);
			// cout<<"VIN "<<alphaData.VIN<<endl;
			state = CONNECTED;
			
		}
		else if(state == CONNECTED){
			for(int i = 0;((alphaData.Speed = getData(PID_SPEED,0x01))<0)&&i<3;i++);
			// cout<<"Speed "<<alphaData.Speed<<endl;
			for(int i = 0;((alphaData.RPM = getData(PID_RPM,0x01))<0)&&i<3;i++);
			// cout<<"RPM "<<alphaData.RPM<<endl;
			// alphaData.MAF = -1;
			for(int i = 0;((alphaData.MAF = getData(PID_MAF_FLOW,0x01))<0)&&i<3;i++);
			// cout<<"MAF "<<alphaData.MAF<<endl;
			if(alphaData.MAF <200 && alphaData.MAF != -1)
				lowPowerState = true;
			else
				lowPowerState = false;
				
			if(lowPowerState)
				usleep(5000000);
			else
				usleep(350000);
		}
		
	}
}

int OBDWrapper::getData(char pid, char mode){
	dataIN.clear();
	sprintf(cmdBuffer, "%02X%02X\r", mode, pid);
	// currentCommand = pid;
	usleep(50000);
	obdSerial.serialFlushRead(fileD);
	// cout<<"SENT: "<<cmdBuffer<<endl;
	if(obdSerial.serialWrite(fileD, cmdBuffer, 5)!=0)
		return -1;
	Timer timer;
	timer.timerCreate();
	timer.timerSet(HALF_SECOND);
	char c  = 0;
	while(1){
		if(obdSerial.serialAvailable(fileD)){
			obdSerial.serialRead(fileD,&c,1);
			dataIN += c;
			if (c==0xD&&dataIN.size()>2){
				// cout<<dataIN<<endl;
				if(dataIN.find("SEARCHING...")!=string::npos){
					dataIN.clear();
				}
				else if(dataIN.find("STOPPED")!=string::npos){
					dataIN.clear();
					timer.timerDelete();
					return -1;
				}
				else if(dataIN.find("NO DATA")!=string::npos){
					dataIN.clear();
					timer.timerDelete();
					return -1;
				}
				else if(dataIN.find("41 ")!=string::npos){
					break;
				}
			}
		}
		else if(timer.timerExpired()){
			dataIN.clear();
			timer.timerDelete();
			return -1;
		}
	}
	timer.timerDelete();
	char* data = getResponse(pid,(char*) dataIN.c_str());
	if(!data){
		return -1;
	}
	return normalizeData(pid, data);
}

char* OBDWrapper::getResponse(char& pid, char* buffer)
{
		char *p = buffer;
		while ((p = strstr(p, "41 "))) {
		    p += 3;
		    char curpid = hex2uint8(p);
		    if (pid == 0) pid = curpid;
		    if (curpid == pid) {
		        p += 2;
		        if (*p == ' ')
		            return p + 1;
		    }
		}
	return 0;
}


