LiquidCrystal_I2C _lcd(
	LIQUIDCRYSTAL_ADDRESS,
	LIQUIDCRYSTAL_EN_PIN,
	LIQUIDCRYSTAL_RW_PIN,
	LIQUIDCRYSTAL_RS_PIN,
	LIQUIDCRYSTAL_D4_PIN,
	LIQUIDCRYSTAL_D5_PIN,
	LIQUIDCRYSTAL_D6_PIN,
	LIQUIDCRYSTAL_D7_PIN,
	LIQUIDCRYSTAL_BACKLIGHT_PIN,
	LIQUIDCRYSTAL_BACKLIGHT_POLARITY
);

DisplayManager::DisplayManager(long refreshInterval) {
	mRefreshInterval = refreshInterval;
}

void DisplayManager::begin() {
	_lcd.begin(16, 2);
	_lcd.setCursor(0, 0);
	_lcd.print("Hello World!");
	delay(1000);
}

void DisplayManager::updateDisplay(DeviceState *pDS) {
	if (getTimeSinceLastDisplayUpdate() > mRefreshInterval) {

		// Temperature
		_lcd.clear();
		_lcd.print(pDS->mTemperature);
		_lcd.print("C ");

		// Push button state
		switch (pDS->mPushButtonState)
		{
		case PBCLOCKWISE:
			_lcd.print("CK ");
			break;
		case PBANTICLOCKWISE:
			_lcd.print("ACK");
			break;
		case NONE:
			_lcd.print("   ");
			break;
		default:
			break;
		}

		// Jogging
		if (pDS->mIsJogging) {
			_lcd.print("-J ");
		}
		else {
			_lcd.print("   ");
		}
		
		// Stepper Speed
		switch (pDS->mSpeed)
		{
		case LOWSPEED:
			_lcd.print("L");
			break;
		case MEDSPEED:
			_lcd.print("M");
			break;
		case HIGHSPEED:
			_lcd.print("H");
			break;
		default:
			_lcd.print(" ");
			break;
		}

		// Stepper Microstep
		_lcd.print(pDS->mMicroStep);

		// Command
		_lcd.setCursor(0, 1);
		_lcd.print(pDS->mCommand);
		mTimeOfDisplayUpdate = millis(); // Update time when temperature was measured. 
	}
}

void DisplayManager::setRefreshInterval(long interval) {
	mRefreshInterval = interval;
}

long DisplayManager::getTimeSinceLastDisplayUpdate() {

	return (millis() - mTimeOfDisplayUpdate);
}