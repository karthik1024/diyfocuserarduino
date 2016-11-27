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

#ifndef LIQUIDCRYSTAL_PARAMS

#define LIQUIDCRYSTAL_ADDRESS 0x3f // Address of the I2C Liquid Crystal Display module
#define LIQUIDCRYSTAL_EN_PIN 2
#define LIQUIDCRYSTAL_RW_PIN 1
#define LIQUIDCRYSTAL_RS_PIN 0
#define LIQUIDCRYSTAL_D4_PIN 4
#define LIQUIDCRYSTAL_D5_PIN 5
#define LIQUIDCRYSTAL_D6_PIN 6
#define LIQUIDCRYSTAL_D7_PIN 7
#define LIQUIDCRYSTAL_BACKLIGHT_PIN 3
#define LIQUIDCRYSTAL_BACKLIGHT_POLARITY POSITIVE
#define LIQUIDCRYSTAL_REFRESH_INTERVAL 100
#define LIQUIDCRYSTAL_PARAMS

#endif // !LIQUIDCRYSTAL_PARAMS

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

	short mBitPrecision;
	double mCurrentTemp = 20.0;
	long mRefreshInterval;
	bool mIsConnected = false;
	long mTimeOfTempReading = 0;

public:

	TemperatureSensor(int bitPrecision, long refreshInterval);
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

	char * processCommand(const char *pCommandString);
	void cmdTemperature();
	void cmdHasTempProbe();
	void cmdReset();
};

class SerialComm {

private:
	char mCommString[SERIALCOMM_MAXINPUTSIZE + 1];
	bool mCommandReceived = false;
	short mCommStringPos = 0;
	char mCommandTerminationChar = '#';

public:
	SerialComm();
	SerialComm(const char commandTerminationChar);
	bool commandReceived();
	void serialEvent();
	void reset();
	void getCommand(char *pDest);
};

struct DeviceState
{
	double mTemperature;
	char mCommand[SERIALCOMM_MAXINPUTSIZE + 1];
	SwitchState mPushButtonState;
	bool mIsJogging;
	StepperSpeed mSpeed;
	short mMicroStep;
};

class DisplayManager {
private:
	long mRefreshInterval;
	long mTimeOfDisplayUpdate = 0;
public:
	DisplayManager(long refreshInterval);
	void begin();
	void updateDisplay(DeviceState *pDS);
	void setRefreshInterval(long interval);
	long getTimeSinceLastDisplayUpdate();
};

class PushButtonState {
private:
	SwitchState mState;
	short mSwitchPin;
	long mStateChangeTimestamp = 0;
	long mJogStartTimestamp = 0;
	bool mIsJogging = false;
public:
	PushButtonState(short switchPin);
	SwitchState getState();
	bool isJogging();
};

class MotorControl {
private:
	short mMicroStep = STEPPER_DEFAULT_MICROSTEP;
	bool mIsEnabled = false;
	StepperSpeed mSpeed = MEDSPEED;
	bool mIsReversed = false;
public:
	MotorControl();
	void initalize();
	void setMicroStep(short microStep);
	short getMicroStep();
	void setEnable(bool enable);
	bool isEnabled();
	void step(StepperDirection direction, int nSteps=1);
	void setReversed(bool truefalse);
	bool isReversed();
	void setDirection(StepperDirection direction);
	void setSpeed(StepperSpeed speed);
	StepperSpeed getSpeed();
};

TemperatureSensor tempSensor(TEMPSENSOR_PRECISION, TEMPSENSOR_REFRESH_INTERVAL);
CommandProcessor cmdProcessor;
SerialComm serialComm;
DeviceState deviceState;
DisplayManager displayManager(LIQUIDCRYSTAL_REFRESH_INTERVAL);
PushButtonState pbState(PUSHBUTTON_SWITCH_PIN);
MotorControl motorControl;

// the setup function runs once when you press reset or power the board
void setup() {
	// Communication
	Serial.begin(SERIALCOMM_BAUDRATE);
	Serial.flush();
	tempSensor.begin();
	displayManager.begin();
	motorControl.initalize();
	analogWrite(Buzzer, 1023);
	delay(100);
	analogWrite(Buzzer, 0);
}

// the loop function runs over and over again until power down or reset
void loop() {
	char commandString[SERIALCOMM_MAXINPUTSIZE + 1];

	// If a command has been received, nothing will get done until it has been processed.
	if (serialComm.commandReceived()) {
		serialComm.getCommand(commandString);

		char * pCmd = cmdProcessor.processCommand(commandString);
		strcpy(deviceState.mCommand, pCmd);

		serialComm.reset();
	}

	// Read push button
	do {
		// Get the state of the device from various sensors.
		deviceState.mTemperature = tempSensor.getTemp();
		deviceState.mSpeed = motorControl.getSpeed();
		deviceState.mMicroStep = motorControl.getMicroStep();
		deviceState.mPushButtonState = pbState.getState();
		deviceState.mIsJogging = pbState.isJogging();

		if (deviceState.mPushButtonState == PBCLOCKWISE) {
			motorControl.step(CLOCKWISE);
		}
		else if (deviceState.mPushButtonState == PBANTICLOCKWISE) {
			motorControl.step(ANTICLOCKWISE);
		}

		// Update the LCD display based on the measured device state.
		displayManager.updateDisplay(&deviceState);
	} while (pbState.isJogging());


	
}

void serialEvent() {
	serialComm.serialEvent();
}

