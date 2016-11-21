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


DisplayManager::DisplayManager(long _refresh_interval) {
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

	p_lcd = &_lcd;
	refresh_interval = _refresh_interval;
}

void DisplayManager::begin() {
	p_lcd->begin(16, 2);
	p_lcd->backlight();
	p_lcd->home();
	p_lcd->print("Hello World!");
	delay(1000);
}

void DisplayManager::updateDisplay(DeviceState *ds) {
	if (getTimeSinceLastDisplayUpdate() > refresh_interval) {

		p_lcd->clear();
		p_lcd->print("T=");
		p_lcd->print(ds->temperature);

		time_of_display_update = millis(); // Update time when temperature was measured. 
	}
}

void DisplayManager::setRefreshInterval(long interval) {
	refresh_interval = interval;
}

long DisplayManager::getTimeSinceLastDisplayUpdate() {

	return (millis() - time_of_display_update);
}