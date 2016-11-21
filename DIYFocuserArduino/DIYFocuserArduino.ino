/*
 Name:		DIYFocuserArduino.ino
 Created:	11/6/2016 2:39:04 PM
 Author:	Karthik
*/

#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <OneWire.h>                // needed for DS18B20 temperature probe
#include <DallasTemperature.h>      // needed for DS18B20 temperature probe

#define SERIALCOMM_BAUDRATE 57600 // Serial communication speed.
#define SERIALCOMM_MAXINPUTSIZE 32 // Maximum length of input command + arguments. 

#define TEMPSENSOR_PRECISION 12      // Set the DS18B20 precision to 0.25 of a degree 9=0.5, 10=0.25, 11=0.125, 12=0.0625
#define TEMPSENSOR_REFRESH_INTERVAL 1000 // In milli seconds.
#define TEMPSENSOR_PIN 2              // temperature probe on pin 2, use 4.7k pullup

#ifndef LIQUIDCRYSTAL_I2C_PARAMS

#define LIQUIDCRYSTAL_IC2_ADDRESS 0x3f // Address of the I2C Liquid Crystal Display module
#define LIQUIDCRYSTAL_IC2_EN_PIN 2
#define LIQUIDCRYSTAL_IC2_RW_PIN 1
#define LIQUIDCRYSTAL_IC2_RS_PIN 0
#define LIQUIDCRYSTAL_IC2_D4_PIN 4
#define LIQUIDCRYSTAL_IC2_D5_PIN 5
#define LIQUIDCRYSTAL_IC2_D6_PIN 6
#define LIQUIDCRYSTAL_IC2_D7_PIN 7
#define LIQUIDCRYSTAL_IC2_BACKLIGHT_PIN 3
#define LIQUIDCRYSTAL_IC2_BACKLIGHT_POLARITY POSITIVE
#define LIQUIDCRYSTAL_IC2_REFRESH_INTERVAL 1000
#define LIQUIDCRYSTAL_I2C_PARAMS

#endif // !LIQUIDCRYSTAL_I2C_PARAMS



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

	char * processCommand(const char *command_string);
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
	char commstring[SERIALCOMM_MAXINPUTSIZE + 1];
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

struct DeviceState
{
	double temperature;
	char command[SERIALCOMM_MAXINPUTSIZE + 1];
};

class DisplayManager {
private:
	long refresh_interval;
	long time_of_display_update = 0;
public:
	DisplayManager(long _refresh_interval);
	void begin();
	void updateDisplay(DeviceState *ds);
	void setRefreshInterval(long interval);
	long getTimeSinceLastDisplayUpdate();
};

TemperatureSensor tempsensor(TEMPSENSOR_PRECISION, TEMPSENSOR_REFRESH_INTERVAL);
CommandProcessor cmdprocessor;
SerialComm serialcomm;
DeviceState devicestate;
DisplayManager displaymanager(LIQUIDCRYSTAL_IC2_REFRESH_INTERVAL);

int currentposition = 0; // Absolute position of focuser
int maxstep = 1000; // Maximum steps allowed by focuser
int stepsize = 1; // Step size

// the setup function runs once when you press reset or power the board
void setup() {
	// Communication
	Serial.begin(SERIALCOMM_BAUDRATE);
	Serial.flush();
	tempsensor.begin();
	displaymanager.begin();
}

// the loop function runs over and over again until power down or reset
void loop() {
	char command_string[SERIALCOMM_MAXINPUTSIZE + 1];

	// If a command has been received, nothing will get done until it has been processed.
	if (serialcomm.commandReceived()) {
		serialcomm.getCommand(command_string);

		char * cmd = cmdprocessor.processCommand(command_string);
		strcpy(devicestate.command, cmd);

		serialcomm.reset();
	}

	// Get the state of the device from various sensors.
	devicestate.temperature = tempsensor.getTemp();

	// Update the LCD display based on the measured device state.
	displaymanager.updateDisplay(&devicestate);
}

void serialEvent() {
	serialcomm.serialEvent();
}

