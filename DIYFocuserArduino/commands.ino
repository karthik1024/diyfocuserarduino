CommandProcessor::CommandProcessor() {}

void CommandProcessor::process_command(char *cmd, char *arg) {

	if (strcmp("POSITON", cmd) == 0) cmd_position();
	else if (strcmp("HOME", cmd) == 0) cmd_home();
	else if (strcmp("MOVE", cmd) == 0) cmd_move(arg);
	else if (strcmp("HALT", cmd) == 0) cmd_halt();
	else if (strcmp("MAXSTEP", cmd) == 0) cmd_maxstep();
	else if (strcmp("STEPSIZE", cmd) == 0) cmd_stepsize();
	else if (strcmp("TEMPERATURE", cmd) == 0) cmd_temperature();
	else if (strcmp("HASTEMPPROBE", cmd) == 0) cmd_hastempprobe();
	else if (strcmp("RESET", cmd) == 0) cmd_reset();
}

// Get current position.
void CommandProcessor::cmd_position()
{
	Serial.print(currentposition);
	Serial.println('#');
}

// Move focuser to home position.
void CommandProcessor::cmd_home() {
	delay(2000);
	currentposition = 0;
	cmd_position();
}

void CommandProcessor::cmd_move(char * arg) {
	int moveby = atoi(arg);
	currentposition += moveby;
	CommandProcessor::cmd_position();
}

void CommandProcessor::cmd_halt() {
	Serial.print("HALTED");
	Serial.println('#');
}

void CommandProcessor::cmd_maxstep() {
	Serial.print(maxstep);
	Serial.println('#');
}

void CommandProcessor::cmd_stepsize() {
	Serial.print(stepsize);
	Serial.println('#');
}

void CommandProcessor::cmd_temperature() {
	Serial.print(tempsensor.getTemp());
	Serial.println('#');
}

void CommandProcessor::cmd_hastempprobe() {
	Serial.print(tempsensor.isConnected());
	Serial.println('#');
}

void CommandProcessor::cmd_reset() {
	Serial.println("Resetting Arduino ...#");
	delay(1000);
	softReset();
}
