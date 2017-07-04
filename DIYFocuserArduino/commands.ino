CommandProcessor::CommandProcessor() {}

char * CommandProcessor::processCommand(const char *pCommandString) {
	char command[SERIALCOMM_MAXINPUTSIZE + 1];
	strcpy(command, pCommandString);

	char *pArg, *pCmd;
	pCmd = pArg = command;
	pCmd = strsep(&pArg, " ");

	if (strcmp("TEMPERATURE", pCmd) == 0) cmdTemperature();
	else if (strcmp("RESET", pCmd) == 0) cmdReset();
	else if (strcmp("HOME", pCmd) == 0) cmdHome();
	else if (strcmp("ENABLE", pCmd) == 0) cmdEnable();
	else if (strcmp("DISABLE", pCmd) == 0) cmdDisable();
	else if (strcmp("IDN", pCmd) == 0) cmdIdn();
	else if (strcmp("MAXSTEPS", pCmd) == 0) cmdMaxSteps();
	else if (strcmp("MOVE", pCmd) == 0) cmdMove(pArg);
	else if (strcmp("HALT", pCmd) == 0) cmdHalt(pArg);
	else if (strcmp("POSITION", pCmd) == 0) cmdPosition();
	return pCmd;
}

void CommandProcessor::cmdTemperature() {
	Serial.print(tempSensor.getCurrentTemp());
	Serial.println('#');
}

void CommandProcessor::cmdReset() {
	Serial.println("Resetting Arduino ...#");
	delay(1000);
	asm volatile ("jmp 0");
}

void CommandProcessor::cmdHome() {
	Serial.println("Homing stepper ...#");
	motorControl.homeStepper();
}

void CommandProcessor::cmdEnable() {
	Serial.println("Enabling stepper#");
	motorControl.setEnable(true);
}

void CommandProcessor::cmdDisable() {
	Serial.println("Disabling stepper#");
	motorControl.setEnable(false);
}

void CommandProcessor::cmdIdn() {
	// Identify this device. 
	// Useful when searching for the focuser hardware among available
	// serial ports.

	Serial.println("ArduinoFocuser#");
}

void CommandProcessor::cmdMaxSteps() {
	// Identify this device. 
	Serial.print(STEPPER_MAXSTEPS);
	Serial.println('#');
}

void CommandProcessor::cmdMove(char * position) {
	int pos = atoi(position);
	motorControl.moveToTarget(pos);
}

void CommandProcessor::cmdHalt() {
	motorControl.halt();
}

void CommandProcessor::cmdPosition() {
	Serial.print(motorControl.getCurrentStep());
	Serial.println('#');
}