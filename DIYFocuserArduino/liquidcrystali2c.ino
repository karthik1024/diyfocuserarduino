#ifndef LIQUIDCRYSTAL_I2C_PARAMS

#define LIQUIDCRYSTAL_IC2_ADDRESS 0X3F // Address of the I2C Liquid Crystal Display module
#define LIQUIDCRYSTAL_IC2_EN_PIN 2
#define LIQUIDCRYSTAL_IC2_RW_PIN 1
#define LIQUIDCRYSTAL_IC2_RS_PIN 0
#define LIQUIDCRYSTAL_IC2_D4_PIN 4
#define LIQUIDCRYSTAL_IC2_D5_PIN 5
#define LIQUIDCRYSTAL_IC2_D6_PIN 6
#define LIQUIDCRYSTAL_IC2_D7_PIN 7
#define LIQUIDCRYSTAL_IC2_BACKLIGHT_PIN 3
#define LIQUIDCRYSTAL_IC2_BACKLIGHT_POLARITY POSITIVE
#define LIQUIDCRYSTAL_IC2_REFRESH_INTERVAL 1000
#define LIQUIDCRYSTAL_I2C_PARAMS

#endif // !LIQUIDCRYSTAL_I2C_PARAMS

LiquidCrystal_I2C _lcd(
	LIQUIDCRYSTAL_IC2_ADDRESS,
	LIQUIDCRYSTAL_IC2_EN_PIN,
	LIQUIDCRYSTAL_IC2_RW_PIN,
	LIQUIDCRYSTAL_IC2_RS_PIN,
	LIQUIDCRYSTAL_IC2_D4_PIN,
	LIQUIDCRYSTAL_IC2_D5_PIN,
	LIQUIDCRYSTAL_IC2_D6_PIN,
	LIQUIDCRYSTAL_IC2_D7_PIN,
	LIQUIDCRYSTAL_IC2_BACKLIGHT_PIN,
	LIQUIDCRYSTAL_IC2_BACKLIGHT_POLARITY
);

DisplayManager::DisplayManager(long _refresh_interval) {
	refresh_interval = _refresh_interval;
}

void DisplayManager::begin() {
	_lcd.begin(16, 2);
	_lcd.setCursor(0, 0);
	_lcd.print("Hello World!");
	delay(1000);
}

void DisplayManager::updateDisplay(DeviceState *ds) {
	if (getTimeSinceLastDisplayUpdate() > refresh_interval) {

		_lcd.clear();
		_lcd.print("T=");
		_lcd.print(ds->temperature);
		_lcd.print("C");
		time_of_display_update = millis(); // Update time when temperature was measured. 
	}
}

void DisplayManager::setRefreshInterval(long interval) {
	refresh_interval = interval;
}

long DisplayManager::getTimeSinceLastDisplayUpdate() {

	return (millis() - time_of_display_update);
}