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

#define TEMPSENSOR_PRECISION 10      // Set the DS18B20 precision to 0.25 of a degree 9=0.5, 10=0.25, 11=0.125, 12=0.0625
#define TEMPSENSOR_REFRESH_INTERVAL 5000 // In milli seconds.
#define TEMPSENSOR_PIN 2              // temperature probe on pin 2, use 4.7k pullup

#ifndef PUSHBUTTON_PARAMS

#define PUSHBUTTON_MIN_TIME_BEFORE_STATE_CHANGE 100
#define PUSHBUTTON_MIN_TIME_BEFORE_JOGGING 1000
#define PUSHBUTTON_SWITCH_PIN  A0   // push button switches wired to A0 via resistor divider network

#endif // !PUSHBUTTON_PARAMS

#define Buzzer A3 // Buzzer

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
#define LIQUIDCRYSTAL_IC2_REFRESH_INTERVAL 100
#define LIQUIDCRYSTAL_I2C_PARAMS

#endif // !LIQUIDCRYSTAL_I2C_PARAMS

// Stepper Motor stuff, control pins for DRV8825 board, REV 203 ONLY
#ifndef STEPPER_MOTOR_PARAMS

#define STEPPER_STEPINDICATOR_LED_PIN A1
#define STEPPER_DIRECTION_PIN 3
#define STEPPER_STEP_PIN 4
#define STEPPER_MICROSTEP_PIN2 5  // microstepping lines
#define STEPPER_MICROSTEP_PIN1 6  // microstepping lines
#define STEPPER_MICROSTEP_PIN0 7  // microstepping lines
#define STEPPER_ENABLEPIN  8

#define STEPPER_ON_TIME 5 // stepontime - time in microseconds that coil power is ON for one step, board requires 2us pulse
#define STEPPER_DEFAULT_MICROSTEP 32 // Valid values are 1, 2, 4, 8, 16, 32.
#endif // !STEPPER_MOTOR_PARAMS

typedef enum SwitchState {
	NONE = 0,
	PBCLOCKWISE = 1,
	PBANTICLOCKWISE = 2,
	BOTH = 3,
} SwitchState;

typedef enum StepperDirection { 
	CLOCKWISE = 0, 
	ANTICLOCKWISE = 1
} StepperDirection;

typedef enum StepperSpeed {
	LOWSPEED = 4000, 
	MEDSPEED = 2400, 
	HIGHSPEED = 1800
} StepperSpeed;

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
	SwitchState pbstate;
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

class PushButtonState {
private:
	SwitchState state;
	short switchpin;
	long state_change_timestamp = 0;
	long jog_start_timestamp = 0;
	bool is_jogging = false;
public:
	PushButtonState(short SwitchPin);
	SwitchState GetState();
	bool isJogging();
};

class MotorControl {
private:
	short microstep = STEPPER_DEFAULT_MICROSTEP;
	bool isenabled = false;
	StepperSpeed speed = MEDSPEED;
	bool isreversed = false;
public:
	MotorControl();
	void Initalize();
	void SetMicroStep(short MicroStep);
	void SetEnable(bool _enable);
	bool IsEnabled();
	void Step(StepperDirection direction, int nSteps=1);
	void SetReversed(bool truefalse);
	bool IsReversed();
	void SetDirection(StepperDirection direction);
	void SetSpeed(StepperSpeed _speed);
	StepperSpeed GetSpeed();
};

TemperatureSensor tempsensor(TEMPSENSOR_PRECISION, TEMPSENSOR_REFRESH_INTERVAL);
CommandProcessor cmdprocessor;
SerialComm serialcomm;
DeviceState devicestate;
DisplayManager displaymanager(LIQUIDCRYSTAL_IC2_REFRESH_INTERVAL);
PushButtonState pbstate(PUSHBUTTON_SWITCH_PIN);
MotorControl motorcontrol;

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
	motorcontrol.Initalize();
	analogWrite(Buzzer, 1023);
	delay(100);
	analogWrite(Buzzer, 0);
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

	// Read push button
	do {
		devicestate.pbstate = pbstate.GetState();

		if (devicestate.pbstate == PBCLOCKWISE) {
			motorcontrol.Step(CLOCKWISE);
		}
		else if (devicestate.pbstate == PBANTICLOCKWISE) {
			motorcontrol.Step(ANTICLOCKWISE);
		}
	} while (pbstate.isJogging());

	// Update the LCD display based on the measured device state.
	displaymanager.updateDisplay(&devicestate);
	
}

void serialEvent() {
	serialcomm.serialEvent();
}

