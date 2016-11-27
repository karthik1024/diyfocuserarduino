#ifndef STEPPER_MOTOR_PARAMS
#define STEPPER_STEPINDICATOR_LED_PIN A1
#define STEPPER_DIRECTION_PIN 3
#define STEPPER_STEP_PIN 4
#define STEPPER_MICROSTEP_PIN2 5  // microstepping lines
#define STEPPER_MICROSTEP_PIN1 6  // microstepping lines
#define STEPPER_MICROSTEP_PIN0 7  // microstepping lines
#define STEPPER_ENABLEPIN  8
#define STEPPER_ON_TIME 5 // stepontime - time in microseconds that coil power is ON for one step, board requires 2us pulse
// m0/m1/m2 sets stepping mode 000 = F, 100 = 1/2, 010 = 1/4, 110 = 1/8, 001 = 1/16, 101 = 1/32
#endif // !STEPPER_MOTOR_PARAMS

MotorControl::MotorControl() {}

void MotorControl::Initalize() {
	pinMode(STEPPER_DIRECTION_PIN, OUTPUT);
	pinMode(STEPPER_STEP_PIN, OUTPUT);
	pinMode(STEPPER_MICROSTEP_PIN0, OUTPUT);
	pinMode(STEPPER_MICROSTEP_PIN1, OUTPUT);
	pinMode(STEPPER_MICROSTEP_PIN2, OUTPUT);
	pinMode(STEPPER_ENABLEPIN, OUTPUT);

	SetDirection(CLOCKWISE);
	digitalWrite(STEPPER_STEP_PIN, 0); 
	SetEnable(false); // Disable stepper unless explicitly enabled.
	SetMicroStep(STEPPER_DEFAULT_MICROSTEP);
}

void MotorControl::SetMicroStep(short _MicroStep) {
	switch (_MicroStep)
	{
	case 1:
		// Full Step
		digitalWrite(STEPPER_MICROSTEP_PIN0, LOW);
		digitalWrite(STEPPER_MICROSTEP_PIN1, LOW);
		digitalWrite(STEPPER_MICROSTEP_PIN2, LOW);
		break;
	case 2:
		// 1/2 (Half) Step
		digitalWrite(STEPPER_MICROSTEP_PIN0, HIGH);
		digitalWrite(STEPPER_MICROSTEP_PIN1, LOW);
		digitalWrite(STEPPER_MICROSTEP_PIN2, LOW);
		break;
	case 4:
		// 1/4 (Quarter) Step
		digitalWrite(STEPPER_MICROSTEP_PIN0, LOW);
		digitalWrite(STEPPER_MICROSTEP_PIN1, HIGH);
		digitalWrite(STEPPER_MICROSTEP_PIN2, LOW);
		break;
	case 8:
		// 1/8 Step
		digitalWrite(STEPPER_MICROSTEP_PIN0, HIGH);
		digitalWrite(STEPPER_MICROSTEP_PIN1, HIGH);
		digitalWrite(STEPPER_MICROSTEP_PIN2, LOW);
		break;
	case 16:
		// 1/16 Step
		digitalWrite(STEPPER_MICROSTEP_PIN0, LOW);
		digitalWrite(STEPPER_MICROSTEP_PIN1, LOW);
		digitalWrite(STEPPER_MICROSTEP_PIN2, HIGH);
		break;
	case 32:
		// 1/32 Step
		digitalWrite(STEPPER_MICROSTEP_PIN0, HIGH);
		digitalWrite(STEPPER_MICROSTEP_PIN1, HIGH);
		digitalWrite(STEPPER_MICROSTEP_PIN2, HIGH);
		break;
	default:
		// Default is Full Step
		digitalWrite(STEPPER_MICROSTEP_PIN0, LOW);
		digitalWrite(STEPPER_MICROSTEP_PIN1, LOW);
		digitalWrite(STEPPER_MICROSTEP_PIN2, LOW);
		break;
	}
	microstep = _MicroStep;
}

void MotorControl::SetEnable(bool _enable) {
	if (_enable == true) {
		digitalWrite(STEPPER_ENABLEPIN, LOW); 
		isenabled = true;
	}
	else {
		digitalWrite(STEPPER_ENABLEPIN, HIGH); 
		isenabled = false;
	}
}

bool MotorControl::IsEnabled() {
	return isenabled;
}

void MotorControl::Step(StepperDirection direction, int nSteps) {
	StepperDirection actual_direction = direction;

	if (IsReversed()) {
		actual_direction = direction == CLOCKWISE ? ANTICLOCKWISE : CLOCKWISE;
	}

	SetDirection(actual_direction);

	for (int steps = 0; steps < nSteps; steps++) {
		// TODO: Implement safety here (maxsteps and minsteps)
		analogWrite(STEPPER_STEPINDICATOR_LED_PIN, 1023);  //Indicate a step via the LED
		digitalWrite(STEPPER_STEP_PIN, HIGH);
		delayMicroseconds(speed);
		digitalWrite(STEPPER_STEP_PIN, LOW);
		analogWrite(STEPPER_STEPINDICATOR_LED_PIN, 0); 
	}
} 

void MotorControl::SetReversed(bool truefalse) {
	isreversed = truefalse;
}

bool MotorControl::IsReversed() {
	return isreversed;
}

void MotorControl::SetDirection(StepperDirection direction) {
	if (direction == CLOCKWISE) {
		digitalWrite(STEPPER_DIRECTION_PIN, HIGH);
	}
	else {
		digitalWrite(STEPPER_DIRECTION_PIN, LOW);
	}
}

void MotorControl::SetSpeed(StepperSpeed _speed) {
	speed = _speed;
}

StepperSpeed MotorControl::GetSpeed() {
	return speed;
}

