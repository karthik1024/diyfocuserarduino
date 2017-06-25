/*
 Name:		DIYFocuserArduino.ino
 Created:	11/6/2016 2:39:04 PM
 Author:	Karthik
*/

#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
// needed for DS18B20 temperature probe
#include <OneWire.h> 
#include <DallasTemperature.h> 

// Serial communication speed.
#define SERIALCOMM_BAUDRATE 57600
// Maximum length of input command + arguments. 
#define SERIALCOMM_MAXINPUTSIZE 32 

// Set the DS18B20 precision to 0.25 of a degree 9=0.5, 10=0.25, 
// 11=0.125, 12=0.0625.
#define TEMPSENSOR_PRECISION_BITS 10 
// Temp sensor reading is refreshed with this interval.
#define TEMPSENSOR_REFRESH_INTERVAL_MILLISECOND 5000
// temperature probe on Arduini pin D2, use 4.7k pullup.
#define TEMPSENSOR_PIN 2              

#ifndef PUSHBUTTON_PARAMS

// The minimum time a button has to be pressed to cause a state change.
#define PUSHBUTTON_MIN_TIME_MS_BEFORE_STATE_CHANGE 100
// The minimum time a button has to be pressed to consider the state to be "jogging".
#define PUSHBUTTON_MIN_TIME_MS_BEFORE_JOGGING 1000
// push button switches wired to Arduino A0 pin via resistor divider network.
#define PUSHBUTTON_SWITCH_PIN  A0   

#endif // !PUSHBUTTON_PARAMS

// The buzzer is connected to Arduino pin A3.
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
#define LIQUIDCRYSTAL_REFRESH_INTERVAL_MILLISECOND 500  // Display update interval.
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

// Time in microseconds that coil power is ON for one step, board requires 2us pulse
#define STEPPER_ON_TIME 5
#define STEPPER_DEFAULT_MICROSTEP 1 // Valid values are 1, 2, 4, 8, 16, 32.
#define STEPPER_DEFAULT_SPEED HIGHSPEED // Valid values are part of enum StepperSpeed
#endif // !STEPPER_MOTOR_PARAMS

typedef enum SwitchState {
	/*Define push button states.*/
	NONE = 0, // No push buttons are pressed.
	PBCLOCKWISE = 1, // The "clockwise" pushbutton is pressed.
	PBANTICLOCKWISE = 2, // The "anticlockwise" pushbutton is pressed.
	BOTH = 3, // Both "clockwise", and "anticlockwise" pushbuttons are pressed.
} SwitchState;

typedef enum StepperDirection { 
	/*Define "clockwise" and "anticlockwise" step directions for the stepper motor.*/
	CLOCKWISE = 0, 
	ANTICLOCKWISE = 1
} StepperDirection;

typedef enum StepperSpeed {
	/*Define speed options for the stepper motor while jogging.*/
	// Number of microseconds to wait between each step while jogging. 
	// Lower numbers results in faster jogs.
	LOWSPEED = 4800, 
	MEDSPEED = 2400, 
	HIGHSPEED = 1200
} StepperSpeed;

class TemperatureSensor
{
	/*Class to handle the temperature sensor.*/
private:
	DeviceAddress address;

	short mBitPrecision;  // Number of bits of precision.
	double mCurrentTemp = 20.0;  // Default temperature display.
	long mRefreshIntervalMilliSecond; // Temperature sensor refresh interval.
	bool mIsConnected = false;
	long mTimeOfTempReading = 0; // Timestamp of latest temperature reading.

public:

	TemperatureSensor(int bitPrecision, long refreshInterval);
	double getCurrentTemp(bool force = false);
	void setRefreshInterval(long interval);
	bool isConnected();
	long getTimeSinceLastTempCheck();
	void begin();
};

class CommandProcessor
{
	/*Class to handle command processing.*/
public:
	CommandProcessor();

	char * processCommand(const char *pCommandString);
	void cmdTemperature();
	void cmdHasTempProbe();
	void cmdReset();
};

class SerialComm {
	/*Class to handle serial communication*/
private:
	// Buffer to hold command strings.
	char mCommString[SERIALCOMM_MAXINPUTSIZE + 1];
	bool mCommandReceived = false;
	// Static allocation for variable to ensure command string doesnt' exceed max input size.
	short mCommStringPos = 0;
	// All commands need to be terminated with this character.
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
	/*Class to store and coordinate device and sensor states.*/
	double mCurrentTemperatureC; // Latest temperature in Celcius.
	char mCommand[SERIALCOMM_MAXINPUTSIZE + 1]; // Latest serial command.
	SwitchState mPushButtonState; // Latest push button state.
	bool mIsJogging;
	StepperSpeed mCurrentSpeed; // Latest stepped job speed.
	short mMicroStep; // Stepped microstep setting.
	long mCurrentStep; // Number of steps the stepper has taken.
};

class DisplayManager {
	/*Class to coordinate what is displayed on the LCD display.*/
private:
	long mRefreshIntervalMilliSecond;  // Amount of time to wait before updating display.
	long mTimeOfDisplayUpdate = 0; // Timestamp of latest display update.
public:
	DisplayManager(long refreshInterval);
	void begin();
	void updateDisplay(DeviceState *pDS);
	void setRefreshInterval(long interval);
	long getTimeSinceLastDisplayUpdate();
};

class PushButtonState {
	/*Class to store and update push button state.*/
private:
	SwitchState mState;  // Current button state.
	short mSwitchPin;  // The pin on which the push bottons are connected.
	long mStateChangeTimestamp = 0;  // Timestamp of latest state change.
	long mJogStartTimestamp = 0;  // Timestamp of latest jog start.
	bool mIsJogging = false;
public:
	PushButtonState(short switchPin);
	SwitchState determinePushButtonState();
	bool isJogging();
};

class MotorControl {
	/*Class to control the stepper motor.*/
private:
	short mMicroStep = STEPPER_DEFAULT_MICROSTEP;
	bool mIsEnabled = false;
	StepperSpeed mCurrentSpeed = STEPPER_DEFAULT_SPEED;
	bool mIsReversed = false;
	long mCurrentStep;  // Current number of steps taken from home position.
public:
	MotorControl();
	void initalize();
	void setMicroStep(short microStep);
	short getMicroStep();
	void setEnable(bool enable);
	bool isEnabled();
	void step(StepperDirection direction, int nSteps=1);
	long getCurrentStep();
	void setReversed(bool truefalse);
	bool isReversed();
	void setDirection(StepperDirection direction);
	void setSpeed(StepperSpeed speed);
	StepperSpeed getCurrentSpeed();
};

/*Create instates of all control classes that will be used throughout the life of the program
to control the focuser.
*/
TemperatureSensor tempSensor(TEMPSENSOR_PRECISION_BITS, TEMPSENSOR_REFRESH_INTERVAL_MILLISECOND);
CommandProcessor cmdProcessor;
SerialComm serialComm;
DeviceState deviceState;
DisplayManager displayManager(LIQUIDCRYSTAL_REFRESH_INTERVAL_MILLISECOND);
PushButtonState pbState(PUSHBUTTON_SWITCH_PIN);
MotorControl motorControl;

// the setup function runs once when you press reset or power the board
void setup() {
	// Setup serial communication.
	Serial.begin(SERIALCOMM_BAUDRATE);
	Serial.flush();
	// Start the temperature sensor.
	tempSensor.begin();
	// Start the LCD display
	displayManager.begin();
	// Initialize the stepper motor.
	motorControl.initalize();
	// Beep to indicate startup.
	analogWrite(Buzzer, 1023);
	delay(100);
	analogWrite(Buzzer, 0);
}

// the loop function runs over and over again until power down or reset
void loop() {
	/*In each iteration of the loop, check if a command has been received. If 
	so, handle it before doing anything else. If not, update display and
	perform actions if any of the push buttons are pressed.
	*/
	char commandString[SERIALCOMM_MAXINPUTSIZE + 1];

	// If a command has been received, nothing will get done until it has been processed.
	if (serialComm.commandReceived()) {
		serialComm.getCommand(commandString);

		char * pCmd = cmdProcessor.processCommand(commandString);
		strcpy(deviceState.mCommand, pCmd);

		serialComm.reset();
	}

	do {
		// Update the state of the device from various sensors.
		deviceState.mCurrentTemperatureC = tempSensor.getCurrentTemp();
		deviceState.mCurrentSpeed = motorControl.getCurrentSpeed();
		deviceState.mMicroStep = motorControl.getMicroStep();
		deviceState.mCurrentStep = motorControl.getCurrentStep();
		deviceState.mPushButtonState = pbState.determinePushButtonState();
		deviceState.mIsJogging = pbState.isJogging();
		
		// If the pushbuttons are pressed, perform the appropriate action.
		switch (deviceState.mPushButtonState) 
		{
		case PBCLOCKWISE:
			motorControl.step(CLOCKWISE);
			break;
		case PBANTICLOCKWISE:
			motorControl.step(ANTICLOCKWISE);
			break;
		}
		
		// Update the LCD display based on the device state.
		displayManager.updateDisplay(&deviceState);
	} while (pbState.isJogging()); // Update display even when jogging.
}

void serialEvent() {
	serialComm.serialEvent();
}

