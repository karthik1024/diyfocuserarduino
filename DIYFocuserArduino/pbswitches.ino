
PushButtonState::PushButtonState(short switchPin) {
		mSwitchPin = switchPin;
		mState = NONE;
	}

SwitchState PushButtonState::getState() {
	int readVal = analogRead(mSwitchPin);
	long timeInNewState;
	long timeJogging;
	SwitchState newState;

	if (readVal > 200 && readVal < 400) {
		newState = PBCLOCKWISE;
	}
	else if (readVal >= 400 && readVal < 600) {
		newState = BOTH;
	}
	else if (readVal >= 600 && readVal < 800) {
		newState = PBANTICLOCKWISE;
	}

	else {
		newState = NONE;
	}

	if (newState != mState) {

		// If state changed, we cannot be jogging.
		mJogStartTimestamp = 0;
		mIsJogging = false;

		if (mStateChangeTimestamp == 0) {  // This is the first time we are seeing the toggle
			mStateChangeTimestamp = millis();   // Start counting how long we are in this state.
		}

		timeInNewState = millis() - mStateChangeTimestamp;

		// Check if we have been in the new state for a sufficiently long duration
		if (timeInNewState > PUSHBUTTON_MIN_TIME_BEFORE_STATE_CHANGE) { 
			mState = newState;
			mStateChangeTimestamp = 0;
		}
	} 
	else if (!mIsJogging)
	{
		if (mJogStartTimestamp == 0) { // This is the first time we are checking for jogging
			mJogStartTimestamp = millis();
		}

		timeJogging = millis() - mJogStartTimestamp;
		
		// Check if we have been in the current state for a sufficiently long duration
		if (timeJogging > PUSHBUTTON_MIN_TIME_BEFORE_JOGGING) { 
			mIsJogging = true;
		}
	}

	return mState;
}

bool PushButtonState::isJogging() {
	return mIsJogging;
}