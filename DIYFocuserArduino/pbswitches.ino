
PushButtonState::PushButtonState(short SwitchPin) {
		switchpin = SwitchPin;
		state = NONE;
	}

SwitchState PushButtonState::GetState() {
	int readval = analogRead(switchpin);
	long time_in_new_state;
	long time_jogging;
	SwitchState newstate;

	if (readval > 200 && readval < 400) {
		newstate = PBCLOCKWISE;
	}
	else if (readval >= 400 && readval < 600) {
		newstate = BOTH;
	}
	else if (readval >= 600 && readval < 800) {
		newstate = PBANTICLOCKWISE;
	}

	else {
		newstate = NONE;
	}

	if (newstate != state) {

		// If state changed, we cannot be jogging.
		jog_start_timestamp = 0;
		is_jogging = false;

		if (state_change_timestamp == 0) {  // This is the first time we are seeing the toggle
			state_change_timestamp = millis();   // Start counting how long we are in this state.
		}

		time_in_new_state = millis() - state_change_timestamp;

		// Check if we have been in the new state for a sufficiently long duration
		if (time_in_new_state > PUSHBUTTON_MIN_TIME_BEFORE_STATE_CHANGE) { 
			state = newstate;
			state_change_timestamp = 0;
		}
	} 
	else if (!is_jogging)
	{
		if (jog_start_timestamp == 0) { // This is the first time we are checking for jogging
			jog_start_timestamp = millis();
		}

		time_jogging = millis() - jog_start_timestamp;
		
		// Check if we have been in the current state for a sufficiently long duration
		if (time_jogging > PUSHBUTTON_MIN_TIME_BEFORE_JOGGING) { 
			is_jogging = true;
		}
	}

	return state;
}

bool PushButtonState::isJogging() {
	return is_jogging;
}