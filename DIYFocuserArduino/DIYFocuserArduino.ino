/*
 Name:		DIYFocuserArduino.ino
 Created:	11/6/2016 2:39:04 PM
 Author:	Karthik
*/

#include <Arduino.h>
#include <OneWire.h>                // needed for DS18B20 temperature probe
#include <DallasTemperature.h>      // needed for DS18B20 temperature probe

#define BAUDRATE 57600 // Serial communication speed.
#define TEMP_PRECISION 12      // Set the DS18B20 precision to 0.25 of a degree 9=0.5, 10=0.25, 11=0.125, 12=0.0625
#define TEMP_REFRESH 1000
#define TEMP_PIN 2              // temperature probe on pin 2, use 4.7k pullup
#define MAXINPUTSIZE 32 // Maximum length of input command + arguments. 

class TemperatureSensor
{
private:
	DeviceAddress address;

	short bitPrecision;
	double current_temp = 20.0;
	long refresh_interval;
	bool _isConnected = false;
	long time_of_temp_reading = 0;

public:

	TemperatureSensor(int _bitPrecision, long _refresh_interval);
	double getTemp(bool force = false);
	void setRefreshInterval(long interval);
	bool isConnected();
	long getTimeSinceLastTempCheck();
	void begin();
};

class CommandProcessor
{
public:
	CommandProcessor();

	void processCommand(char *command_string);
	void cmdPosition();
	void cmdHome();
	void cmdMove(char * arg);
	void cmdHalt();
	void cmdMaxstep();
	void cmdStepSize();
	void cmdTemperature();
	void cmdHasTempProbe();
	void cmdReset();
};

class SerialComm {

private:
	char commstring[MAXINPUTSIZE + 1];
	bool command_received = false;
	short commstring_pos = 0;
	char cmd_termination_char = '#';

public:
	SerialComm();
	SerialComm(const char cmd_termination_char);
	bool commandReceived();
	void serialEvent();
	void reset();
	void getCommand(char *dest);
};

TemperatureSensor tempsensor(TEMP_PRECISION, TEMP_REFRESH);
CommandProcessor cmdprocessor;
SerialComm serialcomm;

int currentposition = 0; // Absolute position of focuser
int maxstep = 1000; // Maximum steps allowed by focuser
int stepsize = 1; // Step size

// the setup function runs once when you press reset or power the board
void setup() {
	// Communication
	Serial.begin(BAUDRATE);
	Serial.flush();
	tempsensor.begin();
}

// the loop function runs over and over again until power down or reset
void loop() {

	// If a command has been received, nothing will get done until it has been processed.
	if (serialcomm.commandReceived()) {
		char command_string[MAXINPUTSIZE + 1];
		serialcomm.getCommand(command_string);

		cmdprocessor.processCommand(command_string);
		serialcomm.reset();
	}
}

void serialEvent() {
	serialcomm.serialEvent();
}

