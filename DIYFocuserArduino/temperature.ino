#include <OneWire.h>                // needed for DS18B20 temperature probe
#include <DallasTemperature.h>      // needed for DS18B20 temperature probe

#ifndef TEMPSENSOR_PIN
#define TEMP_PIN 2              // temperature probe on pin 2, use 4.7k pullup
#endif // TEMP_PIN

OneWire _onewire(TEMPSENSOR_PIN);
DallasTemperature _sensor(&_onewire);

TemperatureSensor::TemperatureSensor(int bitPrecision, long refreshInterval)
{
	mRefreshInterval = refreshInterval;
	mBitPrecision = bitPrecision;
}

double TemperatureSensor::getTemp(bool force)
{
	if ((getTimeSinceLastTempCheck() > mRefreshInterval) || force) {
		_sensor.requestTemperatures();
		delay(750 / (1 << (12 - mBitPrecision))); // should enough time to wait
												 // get channel 1 temperature, always in celsius
		mCurrentTemp = _sensor.getTempCByIndex(0);
		mTimeOfTempReading = millis(); // Update time when temperature was measured.
	}

	mIsConnected = (abs(mCurrentTemp - DEVICE_DISCONNECTED_C) < 1e-2) ? false : true;
	return mCurrentTemp;
}

void TemperatureSensor::setRefreshInterval(long interval) {
	mRefreshInterval = interval;
}

bool TemperatureSensor::isConnected() {
	return mIsConnected;
}


long TemperatureSensor::getTimeSinceLastTempCheck() {

	return (millis() - mTimeOfTempReading);
}

void TemperatureSensor::begin()
{
	_sensor.begin();
	_sensor.getDeviceCount();
	_sensor.getAddress(address, 0);
	_sensor.setResolution(address, mBitPrecision);
	getTemp();
}
