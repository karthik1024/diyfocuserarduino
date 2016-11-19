/*
 Name:		DIYFocuserArduino.ino
 Created:	11/6/2016 2:39:04 PM
 Author:	Karthik
*/

#include <Arduino.h>
#include <OneWire.h>                // needed for DS18B20 temperature probe
#include <DallasTemperature.h>      // needed for DS18B20 temperature probe

#define BAUDRATE 57600 // Serial communication speed.
// DB18B20 temperature probe info
#define TEMP_PRECISION 12      // Set the DS18B20 precision to 0.25 of a degree 9=0.5, 10=0.25, 11=0.125, 12=0.0625
#define TEMP_REFRESH 1000
#define TEMP_PIN 2              // temperature probe on pin 2, use 4.7k pullup
#define MAXINPUTSIZE 32 // Maximum length of input command + arguments. 

class Temperature
{
private:
	DeviceAddress address;

	short bitPrecision;
	double current_temp = 20.0;
	long refresh_interval;
	bool _isConnected = false;
	long time_of_temp_reading = 0;

public:

	Temperature(int _bitPrecision, long _refresh_interval);
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

	void process_command(char *cmd, char *arg);
	void cmd_position();
	void cmd_home();
	void cmd_move(char * arg);
	void cmd_halt();
	void cmd_maxstep();
	void cmd_stepsize();
	void cmd_temperature();
	void cmd_hastempprobe();
	void cmd_reset();
};

Temperature tempsensor(TEMP_PRECISION, TEMP_REFRESH);
CommandProcessor cmdprocessor;

int currentposition = 0; // Absolute position of focuser
int maxstep = 1000; // Maximum steps allowed by focuser
int stepsize = 1; // Step size
char *command, *argument; // Variables to hold the input command and arguments passed via Serial comm.

// Serial Communication Processing.
char commstr[MAXINPUTSIZE + 1]; // String used to hold the Serial communication string.
bool is_correct_termination; // variable used to check if input command was properly terminated.
char input; // variable used to step through each character during serial communication processing.
short commidx; // index used during serial communication processing.

void softReset()
{
	// jump to the start of the program
	asm volatile ("jmp 0");
}

// the setup function runs once when you press reset or power the board
void setup() {
	// Communication
	Serial.begin(BAUDRATE);
	Serial.flush();
	tempsensor.begin();
}

// the loop function runs over and over again until power down or reset
void loop() {
	serial_comm(commstr, is_correct_termination);
	delay(100);
	if (is_correct_termination) {
		command = argument = commstr;
		command = strsep(&argument, " ");
		cmdprocessor.process_command(command, argument);
	}
}

void serial_comm(char *incomm, bool &is_correct_termination)
{
	// default value.
	commidx = 0;
	is_correct_termination = false;
	incomm[MAXINPUTSIZE] = '\0'; // Defensive null termination of string.

	while ((Serial.available() > 0) && commidx <= MAXINPUTSIZE) {
		input = Serial.read();
		if (input == '#') { // End of command. Exit.
			incomm[commidx] = '\0'; // Null terminate input communication string.
			is_correct_termination = true;
			break;
		}
		incomm[commidx] = input;
		commidx++;
	}

	if (!is_correct_termination) {
		// One of the following happened.
		// 1. Command termination character not encountered before MAXINPUTSIZE was reached.
		// 2. Zero length command.
		// 
		// Reset everything and exit.
		incomm[0] = '\0';
	}
}

