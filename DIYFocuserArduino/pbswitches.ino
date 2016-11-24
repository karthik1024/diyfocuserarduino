
PBState::PBState(short SwitchPin) {
		switchpin = SwitchPin;
		state = NONE;
	}

PBState::PBState(short SwitchPin, long MinTimeBeforeStateChange) {
	switchpin = SwitchPin;
	state = NONE;
	min_time_before_change_state = MinTimeBeforeStateChange;
}

SwitchState PBState::GetState() {
	int readval = analogRead(switchpin);
	long time_in_new_state;
	SwitchState newstate;

	if (readval > 200 && readval < 400) {
		newstate = RED;
	}
	else if (readval >= 400 && readval < 600) {
		newstate = BOTH;
	}
	else if (readval >= 600 && readval < 800) {
		newstate = GREEN;
	}

	else {
		newstate = NONE;
	}

	if (newstate != state) {

		if (toggle_timestamp == 0) {  // This is the first time we are seeing the toggle
			toggle_timestamp = millis();   // Start counting how long we are in this state.
		}

		time_in_new_state = millis() - toggle_timestamp;

		if (time_in_new_state > min_time_before_change_state) { // We have been in the new state for a sufficiently long duration
			state = newstate;
			toggle_timestamp = 0;
		}
	}

	return state;
}
