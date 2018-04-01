#include "AquaGodState.h"
#include "Arduino.h"

AquaGodState::AquaGodState()
{
	eventsEnabled = false;
	phContinousModeEnabled = false;

	errorCode = 0;
	lcdIsPresent = false;

	waterLevelIsCriticallyHighInAquarium = false;
	waterLevelIsCriticallyLowInSump = false;
	waterLevelIsCriticallyHighInSump = false;
	waterLevelIsLowInHospital = false;
	waterIsOnFloor1 = false;
	waterIsOnFloor2 = false;
}


void AquaGodState::fireStateEvent(int id, long value)
{
	if (eventsEnabled)
	{
		Serial.print("@S{\"");
		Serial.print(id);
		Serial.print("\"");
		Serial.print(':');
		Serial.print(value);
		Serial.println("}");
	}
}

void AquaGodState::setAquariumTemperature1(int value) {
	if (value != aquariumTemperature1) { aquariumTemperature1 = value; fireStateEvent(400, value); fireStateEvent(399, value); }
}
void AquaGodState::setAquariumTemperature2(int value) {
	if (value != aquariumTemperature2) { aquariumTemperature2 = value; fireStateEvent(401, value); fireStateEvent(399, value); }
}
void AquaGodState::setAquariumTemperature3(int value) {
	if (value != aquariumTemperature3) { aquariumTemperature3 = value; fireStateEvent(402, value); }
}

void AquaGodState::setAquariumPH(int value) {
	if ((value != aquariumPH) || phContinousModeEnabled) { aquariumPH = value; fireStateEvent(102, value); }
}

void AquaGodState::setSumpTemperature(int value) {
	if (value != sumpTemperature) { sumpTemperature = value; fireStateEvent(403, value); }
}

void AquaGodState::setSumpPH(int value) {
	if ((value != sumpPH) || phContinousModeEnabled) { sumpPH = value; fireStateEvent(104, value); }
}

void AquaGodState::setSumpWaterLevel(int value) {
	static unsigned long prevMillis = 0;
	static int prevValueSent = -1000;

	sumpWaterLevel = value;

	if (eventsEnabled && (prevValueSent != sumpWaterLevel))
	{
		unsigned long _now = millis();
		if ((abs(prevValueSent - sumpWaterLevel) >= 5) || ((_now - prevMillis) > 60000L) || (_now < prevMillis)) // once in a minute only or major change
		{
			prevValueSent = value;
			prevMillis = _now;
			fireStateEvent(105, value);
		}
	}
}

void AquaGodState::setSumpWaterLevelMm(int value) {
	if (value != sumpWaterLevelMm) { sumpWaterLevelMm = value; /*fireStateEvent(106, value);*/ }
}

void AquaGodState::setAquariumWaterLevel(int value) {
	static unsigned long prevMillis = 0;
	static int prevValueSent = -1000;

	aquariumWaterLevel = value;

	if (eventsEnabled && (prevValueSent != aquariumWaterLevel))
	{
		unsigned long _now = millis();
		if ((abs(prevValueSent - sumpWaterLevel) >= 5) || ((_now - prevMillis) > 60000L) || (_now < prevMillis)) // once in a minute only or major change
		{
			prevValueSent = value;
			prevMillis = _now;
			fireStateEvent(112, value);
		}
	}
}

void AquaGodState::setAquariumWaterLevelMm(int value) {
	if (value != aquariumWaterLevelMm) { aquariumWaterLevelMm = value; /*fireStateEvent(113, value);*/ }
}

void AquaGodState::setHospitalTemperature(int value) {
	if (value != hospitalTemperature) { hospitalTemperature = value; fireStateEvent(404, value); }
}

void AquaGodState::setRoomTemperature(int value) {
	value = value * 100;
	if (value != roomTemperature) {
		roomTemperature = value;
		fireStateEvent(108, value);
	}
}

void AquaGodState::setRoomHumidity(int value) {
	if (value != roomHumidity) {
		roomHumidity = value;
		fireStateEvent(109, value);
	}
}

void AquaGodState::setBoardTemperature(int value) {
	if (value != boardTemperature) { boardTemperature = value; fireStateEvent(110, value); }
}

void AquaGodState::setBoardFanRPM(unsigned int value) {
	boardFanRPM = value;
	if ((value / 10) != (boardFanRPM / 10))
		fireStateEvent(111, value);
}

void AquaGodState::setWaterLevelIsCriticallyHighInAquarium(bool value) {
	if (value != waterLevelIsCriticallyHighInAquarium) { waterLevelIsCriticallyHighInAquarium = value; fireStateEvent(200, value); }
}

void AquaGodState::setWaterLevelIsCriticallyLowInSump(bool value) {
	if (value != waterLevelIsCriticallyLowInSump) { waterLevelIsCriticallyLowInSump = value; fireStateEvent(201, value); }
}

void AquaGodState::setWaterLevelIsCriticallyHighInSump(bool value) {
	if (value != waterLevelIsCriticallyHighInSump) { waterLevelIsCriticallyHighInSump = value; fireStateEvent(202, value); }
}

void AquaGodState::setWaterLevelIsLowInHospital(bool value) {
	if (value != waterLevelIsLowInHospital) { waterLevelIsLowInHospital = value; fireStateEvent(203, value); }
}

void AquaGodState::setWaterIsOnFloor1(bool value) {
	if (value != waterIsOnFloor1) { waterIsOnFloor1 = value; fireStateEvent(204, value); }
}

void AquaGodState::setWaterIsOnFloor2(bool value) {
	if (value != waterIsOnFloor2) { waterIsOnFloor2 = value; fireStateEvent(205, value); }
}

void AquaGodState::setErrorCode(unsigned int value) {
	if ((value & errorCode) == 0) { errorCode = errorCode | value; fireStateEvent(300, value); }
}

void AquaGodState::unsetErrorCode(unsigned int value) {
	if ((value & errorCode) != 0) { errorCode = errorCode & ~value; fireStateEvent(300, value); }
}

void AquaGodState::setStartTime(unsigned long value) {
	if (value != startTime) { startTime = value; fireStateEvent(302, value); }
}

void AquaGodState::setLcdIsPresent(bool value) {
	if (value != lcdIsPresent) { lcdIsPresent = value; fireStateEvent(309, value); }
}
