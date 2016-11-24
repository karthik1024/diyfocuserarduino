#ifndef SERIALCOMM_MAXINPUTSIZE
#define MAXINPUTSIZE 32 // Maximum length of input command + arguments. 
#endif // !MAXINPUTSIZE

SerialComm::SerialComm() { 
	reset();
}

SerialComm::SerialComm(const char _cmd_termination_char) {
	cmd_termination_char = _cmd_termination_char;
	reset();
}

bool SerialComm::commandReceived() {
	return command_received;
}

void SerialComm::serialEvent() {
	char input;

	// Read the serial port for commands only if current command has been processed
	if (!command_received) {
		while ((Serial.available() > 0) && commstring_pos <= SERIALCOMM_MAXINPUTSIZE) {
			input = Serial.read();  // Read in one character.
			if (input == cmd_termination_char) { // End of command. Exit.
				commstring[commstring_pos] = '\0'; // Null terminate input communication string.
				command_received = true; // Raise flag that a command has been received and available to process.
				break;
			}
			commstring[commstring_pos] = input;
			commstring_pos++;
		}

		if (commstring_pos > SERIALCOMM_MAXINPUTSIZE) {
			// Command length exceeded while waiting for a command to come through.
			// Reset and restart the process.
			reset();
		}
	}
}

void SerialComm::reset() {
	commstring[0] = '\0';  //Null terminate start;
	commstring_pos = 0;  //Reset index
	command_received = false; 
}

void SerialComm::getCommand(char *dest) {
	// This function assumes that *dest has sufficient space for commstring to be copied over.
	strcpy(dest, commstring);
}