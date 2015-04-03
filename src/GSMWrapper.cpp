#include "GSMWrapper.h"
	
void GSMWrapper::begin(char* port_address, int baud_rate){
	state = UNINITIALIZED;
	commandSent = false;
	responseReceived = false;
	running = true;
	readyToSend = false;
	lowPower=true;
	connectionTimer.timerCreate();
	if( (fileD = serialConnection.serialOpen(port_address,baud_rate)) < 0)
	{      
		cout << "could not open port!" << endl;
		return;
	}
	else{
		cout << "serial opened successfully, fd: " << fileD << endl;
	}
//sleep(10);
	
	serialConnection.serialFlushRead(fileD);
	serialConnection.serialFlushWrite(fileD);
	
	connectionThread = new thread(&GSMWrapper::autoConnect,this);
}

void GSMWrapper::togglePower(){
	system("echo high > /sys/class/gpio/gpio26/direction");
	sleep(1);
	system("echo low > /sys/class/gpio/gpio26/direction");
	sleep(5);
}

void GSMWrapper::sendData(string data, bool isNewData){
lowPower = !isNewData;
if(isNewData)
	messageStack.push(data);
if(messageStack.size()>40&&connectionTimer.timerExpired()){
	connectionTimer.timerSet(30*ONE_SECOND);
	state = RECONNECT;
}
if(state==CONNECTED){
	if(readyToSend){
		string message;
		for(int i = 0;!messageStack.empty()&&i<10;i++)
		{
			message += messageStack.top();
			messageStack.pop();
		}
		sendFailBuffer = message;
		message += "\r";
		message += (char)26;
		if(serialConnection.serialWrite(fileD, (char*) message.c_str(),message.length())==0){
				cout<<"DATA SENT: "<<message<<endl;
				readyToSend = false;
		}
	}
	else{
		if(!commandSent){
				if(serialConnection.serialWrite(fileD, SEND, sizeof(SEND))==0){
					commandSent = true;
					cout<<"SENT COMMAND: "<<SEND<<endl;
				}
			}
			else{
				if(responseReceived){
					responseReceived=false;
					dataIN.clear();
					sendFailBuffer.clear();
				}
			}
	}
}	
// Serial2.println("AT+CIPSEND");
		// if(!networkRead("ERROR",">",true))
			// autoConnectNetwork();
		// //delay(1000);
		// Serial2.println(data);
		// //delay(1000);
		// Serial2.println((char)26);
		// while((Serial2.read()) != '\n');
		// //delay(1500);
		// // Serial.println(data);
}

void GSMWrapper::autoConnect(){
	togglePower();
	// connectionTimer.timerSet(HALF_SECOND);
	while(running){
		read();
		if(state == RECONNECT){
			if(!lowPower&&!messageStack.empty())
				state = ECHOOFF;
			commandSent = false;
			readyToSend = false;
			dataIN.clear();
			serialConnection.serialFlushRead(fileD);
		}
		else if(state == UNINITIALIZED){
			if(!commandSent){
				if(serialConnection.serialWrite(fileD, SETECHO, sizeof(SETECHO))==0){
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
				if(serialConnection.serialWrite(fileD, SETSHUT, sizeof(SETSHUT))==0){
					commandSent = true;
					cout<<"SENT COMMAND: "<<SETSHUT<<endl;
				}
			}
			else{
				if(responseReceived){
					state = SHUT;
					commandSent = false;
					responseReceived=false;
					dataIN.clear();
				}
			}
		}
		else if(state == SHUT){
			if(!commandSent){
				if(serialConnection.serialWrite(fileD, SETCARRIER, sizeof(SETCARRIER))==0){
					commandSent = true;
					cout<<"SENT COMMAND: "<<SETCARRIER<<endl;
				}
			}
			else{
				if(responseReceived){
					state = CARRIER;
					commandSent = false;
					responseReceived=false;
					dataIN.clear();
				}
			}
		}
		else if(state == CARRIER){
			if(!commandSent){
				if(serialConnection.serialWrite(fileD, SETWIRELESS, sizeof(SETWIRELESS))==0){
					commandSent = true;
					cout<<"SENT COMMAND: "<<SETWIRELESS<<endl;
				}
			}
			else{
				if(responseReceived){
					state = WIRELESS;
					commandSent = false;
					responseReceived=false;
					dataIN.clear();
				}
			}
		}
		else if(state == WIRELESS){
			if(!commandSent){
				if(serialConnection.serialWrite(fileD, GETIP, sizeof(GETIP))==0){
					commandSent = true;
					cout<<"SENT COMMAND: "<<GETIP<<endl;
				}
			}
			else{
				if(responseReceived){
					state = IP;
					commandSent = false;
					responseReceived=false;
					dataIN.clear();
				}
			}
		}
		else if(state == IP){
			if(!commandSent){
				if(serialConnection.serialWrite(fileD, TCPCONNECT, sizeof(TCPCONNECT))==0){
					commandSent = true;
					cout<<"SENT COMMAND: "<<TCPCONNECT<<endl;
				}
			}
			else{
				if(responseReceived){
					state = CONNECTED;
					commandSent = false;
					responseReceived=false;
					dataIN.clear();
				}
			}
		}
		else if(state == CONNECTED){
			
		}
	}
}

char GSMWrapper::read(){
	char c = 0;

	if(!serialConnection.serialAvailable(fileD)){return c;}
	serialConnection.serialRead(fileD,&c,1);

	//Serial.print(c);

	dataIN += c;
	//cout<<hex<<int(c)<<endl;
	if ((c==0xA&&dataIN.size()>2)||c=='>') {
	
		if(state == UNINITIALIZED){
			if(dataIN.find("OK")!=string::npos){
				responseReceived = true;
				cout<<dataIN<<endl;
				serialConnection.serialFlushRead(fileD);
			}
			else if(dataIN.find("ATE0")!=string::npos){
				cout<<dataIN<<endl;
				dataIN.clear();
			}
			else if(dataIN.find("NORMAL POWER DOWN")!=string::npos){
				commandSent = false;
				togglePower();
				dataIN.clear();
				serialConnection.serialFlushRead(fileD);
			}
			else{
				commandSent = false;
				cout<<dataIN<<endl;
				dataIN.clear();
			}
		}
		// else if(dataIN.find("DEACT")!=string::npos){
				// commandSent = false;
				// readyToSend = false;
				// state = ECHOOFF;
				// cout<<dataIN<<endl;
				// dataIN.clear();
				// serialConnection.serialFlushRead(fileD);
			// }
		else if(state == ECHOOFF){
			if(dataIN.find("SHUT OK")!=string::npos){
				responseReceived = true;
				cout<<dataIN<<endl;
				serialConnection.serialFlushRead(fileD);
			}
			else{
				commandSent = false;
				cout<<dataIN<<endl;
				dataIN.clear();
				serialConnection.serialFlushRead(fileD);
			}
		}
		else if(state == SHUT){
			if(dataIN.find("OK")!=string::npos){
				responseReceived = true;
				cout<<dataIN<<endl;
				serialConnection.serialFlushRead(fileD);
			}
			else{
				commandSent = false;
				state = ECHOOFF;
				cout<<dataIN<<endl;
				dataIN.clear();
				serialConnection.serialFlushRead(fileD);
			}
		}
		else if(state == CARRIER){
			if(dataIN.find("OK")!=string::npos){
				responseReceived = true;
				cout<<dataIN<<endl;
				serialConnection.serialFlushRead(fileD);
				connectionTimer.timerSet(30*ONE_SECOND);
			}
			else if(dataIN.find("DEACT")!=string::npos){
				commandSent = false;
				readyToSend = false;
				state = ECHOOFF;
				cout<<dataIN<<endl;
				dataIN.clear();
				serialConnection.serialFlushRead(fileD);
			}
			else{
				commandSent = false;
				state = ECHOOFF;
				cout<<dataIN<<endl;
				dataIN.clear();
				serialConnection.serialFlushRead(fileD);
			}
		}
		else if(state == WIRELESS){
			if(dataIN.find("ERROR")==string::npos){
				responseReceived = true;
				cout<<dataIN<<endl;
				serialConnection.serialFlushRead(fileD);
				connectionTimer.timerSet(30*ONE_SECOND);
			}
			else{
				commandSent = false;
				state = ECHOOFF;
				cout<<dataIN<<endl;
				dataIN.clear();
				serialConnection.serialFlushRead(fileD);
			}
		}
		else if(state == IP){
			if(dataIN.find("CONNECT OK")!=string::npos){
				responseReceived = true;
				cout<<dataIN<<endl;
				serialConnection.serialFlushRead(fileD);
				connectionTimer.timerSet(30*ONE_SECOND);
			}
			else if(dataIN.find("CONNECT FAIL")!=string::npos){
				commandSent = false;
				cout<<dataIN<<endl;
				dataIN.clear();
				serialConnection.serialFlushRead(fileD);
			}
			else{
				cout<<dataIN<<endl;
				dataIN.clear();
			}
		}
		else if(state == CONNECTED){
			if(dataIN.find("CLOSED")!=string::npos){
				commandSent = false;
				readyToSend = false;
				state = ECHOOFF;
				cout<<dataIN<<endl;
				if(!sendFailBuffer.empty())
					messageStack.push(sendFailBuffer);
				dataIN.clear();
				serialConnection.serialFlushRead(fileD);
			}
			else if(dataIN.find(">")!=string::npos){
				readyToSend = true;
				responseReceived = true;
				cout<<dataIN<<endl;
				dataIN.clear();
				serialConnection.serialFlushRead(fileD);
			}
			else if(dataIN.find("SEND OK")!=string::npos){
				responseReceived = true;
				commandSent = false;
				readyToSend = false;
				cout<<dataIN<<endl;
				dataIN.clear();
				connectionTimer.timerSet(10*ONE_SECOND);
			}
			else if(dataIN.find("DEACT")!=string::npos){
				commandSent = false;
				readyToSend = false;
				state = ECHOOFF;
				cout<<dataIN<<endl;
				dataIN.clear();
				serialConnection.serialFlushRead(fileD);
			}
			else if(dataIN.find("ERROR")!=string::npos){
				commandSent = false;
				readyToSend = false;
				state = ECHOOFF;
				cout<<dataIN<<endl;
				dataIN.clear();
				if(!sendFailBuffer.empty())
					messageStack.push(sendFailBuffer);
				serialConnection.serialFlushRead(fileD);
			}
			else if(dataIN.find("SEND FAIL")!=string::npos){
				commandSent = false;
				readyToSend = false;
				state = ECHOOFF;
				cout<<dataIN<<endl;
				dataIN.clear();
				if(!sendFailBuffer.empty())
					messageStack.push(sendFailBuffer);
				serialConnection.serialFlushRead(fileD);
			}
			else{
				cout<<dataIN<<endl;
				dataIN.clear();
			}
		}
	}
	return c;
}
int GSMWrapper::checkResponse(char* message){return 0;}