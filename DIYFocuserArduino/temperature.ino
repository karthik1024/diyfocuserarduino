#include <OneWire.h>                // needed for DS18B20 temperature probe
#include <DallasTemperature.h>      // needed for DS18B20 temperature probe

#ifndef TEMP_PIN
#define TEMP_PIN 2              // temperature probe on pin 2, use 4.7k pullup
#endif // TEMP_PIN

OneWire _onewire(TEMP_PIN);
DallasTemperature _sensor(&_onewire);

Temperature::Temperature(int _bitPrecision, long _refresh_interval)
{
	refresh_interval = _refresh_interval;
	bitPrecision = _bitPrecision;
}

double Temperature::getTemp(bool force)
{
	if ((getTimeSinceLastTempCheck() > refresh_interval) || force) {
		_sensor.requestTemperatures();
		delay(600 / (1 << (12 - bitPrecision))); // should enough time to wait
												 // get channel 1 temperature, always in celsius
		current_temp = _sensor.getTempCByIndex(0);
		time_of_temp_reading = millis(); // Update time when temperature was measured.
	}

	_isConnected = (abs(current_temp - DEVICE_DISCONNECTED_C) < 1e-2) ? false : true;
	return current_temp;
}

void Temperature::setRefreshInterval(long interval) {
	refresh_interval = interval;
}

bool Temperature::isConnected() {
	return _isConnected;
}


long Temperature::getTimeSinceLastTempCheck() {

	return (millis() - time_of_temp_reading);
}

void Temperature::begin()
{
	_sensor.begin();
	_sensor.getDeviceCount();
	_sensor.getAddress(address, 0);
	_sensor.setResolution(address, bitPrecision);
	getTemp();
}
