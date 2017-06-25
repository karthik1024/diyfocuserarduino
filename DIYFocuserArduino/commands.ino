CommandProcessor::CommandProcessor() {}

char * CommandProcessor::processCommand(const char *pCommandString) {
	char command[SERIALCOMM_MAXINPUTSIZE + 1];
	strcpy(command, pCommandString);

	char *pArg, *pCmd;
	pCmd = pArg = command;
	pCmd = strsep(&pArg, " ");

	if (strcmp("TEMPERATURE", pCmd) == 0) cmdTemperature();
	else if (strcmp("HASTEMPPROBE", pCmd) == 0) cmdHasTempProbe();
	else if (strcmp("RESET", pCmd) == 0) cmdReset();

	return pCmd;
}

void CommandProcessor::cmdTemperature() {
	Serial.print(tempSensor.getCurrentTemp());
	Serial.println('#');
}

void CommandProcessor::cmdHasTempProbe() {
	Serial.print(tempSensor.isConnected());
	Serial.println('#');
}

void CommandProcessor::cmdReset() {
	Serial.println("Resetting Arduino ...#");
	delay(1000);
	asm volatile ("jmp 0");
}
