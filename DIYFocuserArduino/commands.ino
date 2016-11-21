CommandProcessor::CommandProcessor() {}

char * CommandProcessor::processCommand(const char *command_string) {
	char command[SERIALCOMM_MAXINPUTSIZE + 1];
	strcpy(command, command_string);

	char *arg, *cmd;
	cmd = arg = command;
	cmd = strsep(&arg, " ");

	if (strcmp("POSITON", cmd) == 0) cmdPosition();
	else if (strcmp("HOME", cmd) == 0) cmdHome();
	else if (strcmp("MOVE", cmd) == 0) cmdMove(arg);
	else if (strcmp("HALT", cmd) == 0) cmdHalt();
	else if (strcmp("MAXSTEP", cmd) == 0) cmdMaxstep();
	else if (strcmp("STEPSIZE", cmd) == 0) cmdStepSize();
	else if (strcmp("TEMPERATURE", cmd) == 0) cmdTemperature();
	else if (strcmp("HASTEMPPROBE", cmd) == 0) cmdHasTempProbe();
	else if (strcmp("RESET", cmd) == 0) cmdReset();

	return cmd;
}

// Get current position.
void CommandProcessor::cmdPosition()
{
	Serial.print(currentposition);
	Serial.println('#');
}

// Move focuser to home position.
void CommandProcessor::cmdHome() {
	delay(2000);
	currentposition = 0;
	cmdPosition();
}

void CommandProcessor::cmdMove(char * arg) {
	int moveby = atoi(arg);
	currentposition += moveby;
	CommandProcessor::cmdPosition();
}

void CommandProcessor::cmdHalt() {
	Serial.print("HALTED");
	Serial.println('#');
}

void CommandProcessor::cmdMaxstep() {
	Serial.print(maxstep);
	Serial.println('#');
}

void CommandProcessor::cmdStepSize() {
	Serial.print(stepsize);
	Serial.println('#');
}

void CommandProcessor::cmdTemperature() {
	Serial.print(tempsensor.getTemp());
	Serial.println('#');
}

void CommandProcessor::cmdHasTempProbe() {
	Serial.print(tempsensor.isConnected());
	Serial.println('#');
}

void CommandProcessor::cmdReset() {
	Serial.println("Resetting Arduino ...#");
	delay(1000);
	asm volatile ("jmp 0");
}
