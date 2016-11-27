SerialComm::SerialComm() { 
	reset();
}

SerialComm::SerialComm(const char commandTerminationChar) {
	mCommandTerminationChar = commandTerminationChar;
	reset();
}

bool SerialComm::commandReceived() {
	return mCommandReceived;
}

void SerialComm::serialEvent() {
	char input;

	// Read the serial port for commands only if current command has been processed
	if (!mCommandReceived) {
		while ((Serial.available() > 0) && mCommStringPos <= SERIALCOMM_MAXINPUTSIZE) {
			input = Serial.read();  // Read in one character.
			if (input == mCommandTerminationChar) { // End of command. Exit.
				mCommString[mCommStringPos] = '\0'; // Null terminate input communication string.
				mCommandReceived = true; // Raise flag that a command has been received and available to process.
				break;
			}
			mCommString[mCommStringPos] = input;
			mCommStringPos++;
		}

		if (mCommStringPos > SERIALCOMM_MAXINPUTSIZE) {
			// Command length exceeded while waiting for a command to come through.
			// Reset and restart the process.
			reset();
		}
	}
}

void SerialComm::reset() {
	mCommString[0] = '\0';  //Null terminate start;
	mCommStringPos = 0;  //Reset index
	mCommandReceived = false; 
}

void SerialComm::getCommand(char *pDest) {
	// This function assumes that *dest has sufficient space for commstring to be copied over.
	strcpy(pDest, mCommString);
}