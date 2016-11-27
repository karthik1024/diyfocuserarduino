MotorControl::MotorControl() {}

void MotorControl::initalize() {
	pinMode(STEPPER_DIRECTION_PIN, OUTPUT);
	pinMode(STEPPER_STEP_PIN, OUTPUT);
	pinMode(STEPPER_MICROSTEP_PIN0, OUTPUT);
	pinMode(STEPPER_MICROSTEP_PIN1, OUTPUT);
	pinMode(STEPPER_MICROSTEP_PIN2, OUTPUT);
	pinMode(STEPPER_ENABLEPIN, OUTPUT);

	setDirection(CLOCKWISE);
	digitalWrite(STEPPER_STEP_PIN, 0); 
	setEnable(true);
	setMicroStep(STEPPER_DEFAULT_MICROSTEP);
}

void MotorControl::setMicroStep(short microStep) {
	switch (microStep)
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
	mMicroStep = microStep;
}

short MotorControl::getMicroStep() {
	return mMicroStep;
}

void MotorControl::setEnable(bool enable) {
	if (enable == true) {
		digitalWrite(STEPPER_ENABLEPIN, LOW); 
		mIsEnabled = true;
	}
	else {
		digitalWrite(STEPPER_ENABLEPIN, HIGH); 
		mIsEnabled = false;
	}
}

bool MotorControl::isEnabled() {
	return mIsEnabled;
}

void MotorControl::step(StepperDirection direction, int nSteps) {
	StepperDirection actualDirection = direction;

	if (isReversed()) {
		actualDirection = direction == CLOCKWISE ? ANTICLOCKWISE : CLOCKWISE;
	}

	setDirection(actualDirection);

	for (int steps = 0; steps < nSteps; steps++) {
		// TODO: Implement safety here (maxsteps and minsteps)
		analogWrite(STEPPER_STEPINDICATOR_LED_PIN, 1023);  //Indicate a step via the LED
		digitalWrite(STEPPER_STEP_PIN, HIGH);
		delayMicroseconds((int)STEPPER_ON_TIME);
		digitalWrite(STEPPER_STEP_PIN, LOW);

		switch (mMicroStep) {
		case 1:
			delayMicroseconds(mSpeed);
			break;
		case 2:
			delayMicroseconds((int)(mSpeed / 2));
			break;
		default:
			delayMicroseconds((int)(mSpeed / 4));
			break;
		}

		analogWrite(STEPPER_STEPINDICATOR_LED_PIN, 0); 
	}
} 

void MotorControl::setReversed(bool truefalse) {
	mIsReversed = truefalse;
}

bool MotorControl::isReversed() {
	return mIsReversed;
}

void MotorControl::setDirection(StepperDirection direction) {
	if (direction == CLOCKWISE) {
		digitalWrite(STEPPER_DIRECTION_PIN, HIGH);
	}
	else {
		digitalWrite(STEPPER_DIRECTION_PIN, LOW);
	}
}

void MotorControl::setSpeed(StepperSpeed speed) {
	mSpeed = speed;
}

StepperSpeed MotorControl::getSpeed() {
	return mSpeed;
}

