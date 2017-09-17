MotorControl::MotorControl(Bounce &homeButton) {
	mHomePositionButton = &homeButton;
}

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

void MotorControl::homeStepper() {
	/*The step() method ensures that if the home button is pressed, the stepper
	is disabled. In this method, we re-enable the stepper and continue stepping
	until the stepper is disabled by the step() method due to hitting the home
	position button. We then force movement in the opposite direction to ensure
	that the button reads low before we return.
	*/
	setEnable(true);
	StepperSpeed savedSpeed = mCurrentSpeed;
	setSpeed(HIGHSPEED);

	// Keep stepping till the home button gets pressed. When the button is 
	// pressed, the home position button will be grounded.
	int signIn = directionToSign(STEPPER_DIRECTION_IN);
	mIsHomed = false;
	while (mHomePositionButton->read() == HIGH) {
		step(signIn);
	}

	// Define the position where home button gets pressed as Zero.
	mCurrentStep = 0;

	// We now need to move the focuser back, away from the home button in order
	// to allow stepping to occur. However, we will need to force this to happen
	// since due to hysteresis, the home buttom will likely remain depressed
	// even as we are trying to move away from it.
	while (mHomePositionButton->read() == LOW) {
		step(-1 * signIn, true);
	}

	mIsHomed = true;
	setSpeed(savedSpeed); // Reset to older speed.
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

void MotorControl::step(int nSteps, bool force = false) {
	if (!mIsEnabled) {
		// If the stepper is not enabled, there is nothing to do.
		return;
	}

	int actualSteps = (int)abs(nSteps);

	// Set the direction of the stepper based on the sign of nSteps.
	StepperDirection actualDirection;
	if (nSteps > 0) {
		actualDirection = STEPPER_DIRECTION_OUT;
	}
	else {
		actualDirection = STEPPER_DIRECTION_IN;
	}
	setDirection(actualDirection);

	bool isHomeButtonPressed;
	bool ifMaxStepsReached;

	for (int steps = 0; steps < actualSteps; steps++) {
		// Update and check if we have reached the MaxSteps or the home position.
		// Unless forced to move, simply break out of the loop for safety. The
		// only exception is when we are trying to home the focuser, we might
		// need to reverse the focuser while the home button is still pressed.
		mHomePositionButton->update();
		isHomeButtonPressed = mHomePositionButton->read() == LOW;
		ifMaxStepsReached = mCurrentStep >= STEPPER_MAXSTEPS;

		// If the home button has been pressed, the fucuser shouldn't be allowed
		// to move in anymore, but allowed to move out. Similarly, if maxsteps
		// has been reached, the focuser should be allowed to move in but not
		// out. If this asymmetric check is not performed, once either of the
		// limits are reached, it will be impossible to move the focuser.
		if (ifMaxStepsReached && actualDirection == STEPPER_DIRECTION_OUT) {
			break;
		}

		if (isHomeButtonPressed && actualDirection == STEPPER_DIRECTION_IN) {
			break;
		}

		// Execute a single step. The direction has already been determined via setDirection()
		int led = nSteps > 0 ? STEPPER_POSITIVE_STEPINDICATOR_LED_PIN : STEPPER_NEGATIVE_STEPINDICATOR_LED_PIN;
		analogWrite(led, 1023);

		digitalWrite(STEPPER_STEP_PIN, HIGH);
		delayMicroseconds((int)STEPPER_ON_TIME);
		digitalWrite(STEPPER_STEP_PIN, LOW);

		// Update internal step count based on the sign of nSteps.
		mCurrentStep =  nSteps > 0 ? mCurrentStep + 1 : mCurrentStep - 1;

		// Wait before executing the next step. This wait time depends on whether we are 
		// micro-stepping or not.
		switch (mMicroStep) {
		case 1:
			delayMicroseconds(mCurrentSpeed);
			break;
		case 2:
			delayMicroseconds((int)(mCurrentSpeed / 2));
			break;
		default:
			delayMicroseconds((int)(mCurrentSpeed / 4));
			break;
		}

		analogWrite(led, 0); 
	}
} 

long MotorControl::getCurrentStep() {
	return mCurrentStep;
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
	mCurrentSpeed = speed;
}

StepperSpeed MotorControl::getCurrentSpeed() {
	return mCurrentSpeed;
}

void MotorControl::toggleSpeed() {
	switch (mCurrentSpeed)
	{
	case LOWSPEED:
		motorControl.setSpeed(MEDSPEED);
		break;
	case MEDSPEED:
		motorControl.setSpeed(HIGHSPEED);
		break;
	case HIGHSPEED:
		motorControl.setSpeed(LOWSPEED);
		break;
	}
}

void MotorControl::moveToTarget(int target) {
	mTarget = target;
	isExecutingMoveCommand = true;
}

void MotorControl::executeMove() {
	if (!isExecutingMoveCommand) {
		return;
	}

	int sign = (mTarget - mCurrentStep) >= 0 ? 1 : -1;
	bool targetReached = mTarget == mCurrentStep;
	if (!targetReached) {
		step(sign);
	}
	else {
		isExecutingMoveCommand = false;
		mTarget = NULL;
	}
	
}

void MotorControl::halt() {
	isExecutingMoveCommand = false;
	mTarget = NULL;
}


int MotorControl::directionToSign(StepperDirection direction) {
	int sign = direction == STEPPER_DIRECTION_OUT ? 1 : -1;
	return sign;
}