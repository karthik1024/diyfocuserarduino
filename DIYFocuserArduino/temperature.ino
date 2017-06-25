#include <OneWire.h>                // needed for DS18B20 temperature probe
#include <DallasTemperature.h>      // needed for DS18B20 temperature probe

OneWire _onewire(TEMPSENSOR_PIN);
DallasTemperature _sensor(&_onewire);

TemperatureSensor::TemperatureSensor(int bitPrecision, long refreshInterval)
{
	mRefreshIntervalMilliSecond = refreshInterval;
	mBitPrecision = bitPrecision;
}

double TemperatureSensor::getCurrentTemp(bool force)
{
	if ((getTimeSinceLastTempCheck() > mRefreshIntervalMilliSecond) || force) {
		_sensor.requestTemperatures();
		delay(750 / (1 << (12 - mBitPrecision))); // should enough time to wait
												 // get channel 1 temperature, always in celsius
		mCurrentTemp = _sensor.getTempCByIndex(0);
		// TODO: Handle overflow.
		mTimeOfTempReading = millis(); // Update time when temperature was measured.
	}

	mIsConnected = (abs(mCurrentTemp - DEVICE_DISCONNECTED_C) < 1e-2) ? false : true;
	return mCurrentTemp;
}

void TemperatureSensor::setRefreshInterval(long interval) {
	mRefreshIntervalMilliSecond = interval;
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
	getCurrentTemp();
}
