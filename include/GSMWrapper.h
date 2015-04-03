#ifndef GSMWRAPPER_H     // Prevent duplicate definition
#define GSMWRAPPER_H

//Include:
#include "SerialWrapper.h"
#include <string>
#include <thread>
#include <stack>

#define PING "AT\r"
#define SETECHO "ATE0\r"
#define SETSHUT "AT+CIPSHUT\r"
#define SETCARRIER "AT+CSTT=\"wap.cingular\"\r"
#define SETWIRELESS "AT+CIICR\r"
#define GETIP "AT+CIFSR\r"
#define TCPCONNECT "AT+CIPSTART=\"TCP\",\"54.187.143.98\",\"4200\"\r"
#define SEND "AT+CIPSEND\r"

using namespace std;

class GSMWrapper
{
public:
	// Public Functions:
	void begin(char* port_address, int baud_rate);
	~GSMWrapper(){
		running = false;
		if(connectionThread!=NULL){
			connectionThread->join();
			free(connectionThread);
		}
	}
	void sendData(string data, bool isNewData);
private:
	SerialWrapper serialConnection;
	enum ConnectionState { UNINITIALIZED, ECHOOFF, SHUT, CARRIER, WIRELESS, IP, CONNECTED, RECONNECT };
	ConnectionState state;
	thread* connectionThread;
	bool lowPower;
	bool running;
	bool commandSent;
	bool responseReceived;
	bool readyToSend;
	stack<string> messageStack;
	string sendFailBuffer;
	Timer connectionTimer;
	string dataIN;
	int fileD;
	char read();
	int checkResponse(char* message);
	void togglePower();
	void autoConnect();
};
#endif