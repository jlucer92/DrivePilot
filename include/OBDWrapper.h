#ifndef OBDWRAPPER_H     // Prevent duplicate definition
#define OBDWRAPPER_H

#include "SerialWrapper.h"
#include <thread>

#define OBD_TIMEOUT_SHORT 2000000 /* us */
#define OBD_TIMEOUT_LONG 7000000 /* us */
#define OBD_SERIAL_BAUDRATE 9600
#define OBD_RECV_BUF_SIZE 128

// Mode 1 PIDs
#define PID_ENGINE_LOAD 0x04
#define PID_COOLANT_TEMP 0x05
#define PID_FUEL_PRESSURE 0x0A
#define PID_INTAKE_MAP 0x0B
#define PID_RPM 0x0C
#define PID_SPEED 0x0D
#define PID_TIMING_ADVANCE 0x0E
#define PID_INTAKE_TEMP 0x0F
#define PID_MAF_FLOW 0x10
#define PID_THROTTLE 0x11
#define PID_RUNTIME 0x1F
#define PID_FUEL_LEVEL 0x2F
#define PID_DISTANCE 0x31
#define PID_BAROMETRIC 0x33
#define PID_CONTROL_MODULE_VOLTAGE 0x42
#define PID_ABSOLUTE_ENGINE_LOAD 0x43
#define PID_AMBIENT_TEMP 0x46
#define PID_ETHANOL_PERCENTAGE 0x52
#define PID_FUEL_RAIL_PRESSURE 0x59
#define PID_HYBRID_BATTERY_PERCENTAGE 0x5B
#define PID_ENGINE_OIL_TEMP 0x5C
#define PID_ENGINE_FUEL_RATE 0x5E
#define PID_ENGINE_TORQUE_PERCENTAGE 0x62
#define PID_ENGINE_REF_TORQUE 0x63

// Mode 9 PIDS
#define PID_VIN 0x02

// states
#define OBD_DISCONNECTED 0
#define OBD_CONNECTING 1
#define OBD_CONNECTED 2

#define SETRESET "ATZ\r"
#define SETECHO "ATE0\r"
#define SETNEWLINE "ATL1\r"

#define LowPowerSleepTime = 5000000
#define FullSleepTime = 350000

unsigned int hex2uint16(const char *p);
unsigned char hex2uint8(const char *p);
	
struct AlphaData {
  int Speed;
  int RPM;
  int MAF;
  int VIN;
};


class OBDWrapper
{
public:
	OBDWrapper():dataMode(1),m_state(OBD_DISCONNECTED) {}
	virtual void begin(char* port_address);
	virtual void init();
	char read();
	
	// char getState() { return m_state; }
	char dataMode;
	char pidmap[4 * 4];
	AlphaData alphaData;
	char vin[17];
	bool lowPowerState;
	
	~OBDWrapper(){
		running = false;
		if(connectionThread!=NULL){
			connectionThread->join();
			free(connectionThread);
		}
	}
	
protected:
	int normalizeData(char pid, char* data);
	char m_state;
private:
	enum ConnectionState { UNINITIALIZED, RESET, ECHOOFF, GETVIN, CONNECTED };
	ConnectionState state;
	char currentCommand;
	char cmdBuffer[8];
	char responseBuffer[20];
	thread* connectionThread;
	// thread* connectionThread;
	bool running;
	bool commandSent;
	bool responseReceived;
	string dataIN;
	SerialWrapper obdSerial;
	int fileD;
	int requestSleepTime;
	
	int getData(char pid, char mode);
	char* getResponse(char& pid, char* buffer);

	virtual uint8_t getPercentageValue(char* data)
	{
		return (uint16_t)hex2uint8(data) * 100 / 255;
	}
	virtual uint16_t getLargeValue(char* data)
	{
		return hex2uint16(data);
	}
	virtual uint8_t getSmallValue(char* data)
	{
		return hex2uint8(data);
	}
	virtual int16_t getTemperatureValue(char* data)
	{
		return (int)hex2uint8(data) - 40;
	}
	
};

#endif