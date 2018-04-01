extern Settings settings;

water_change_state waterChangeState = WATER_CHANGE_STATE_NONE;

void reInitAquarium()
{
	initAquarium();
	Alarm.timerRepeat(1, oncePerSecond, NULL, 0, 0); // initAquarium clears all timers
}

void initAquarium()
{
	for (uint8_t id = 0; id < dtNBR_ALARMS; id++)
	{
		if (Alarm.isAllocated(id))
			Alarm.free(id);
	}

	// Turn lights if necessary
	time_t time_now = now();
	time_now = time_now - previousMidnight(time_now);

	arduinoPinSetState(PIN_HEATER_1, 0, false);
	arduinoPinSetState(PIN_HEATER_2, 0, false);
	arduinoPinSetState(PIN_SUMP_HEATER, 0, false);
	arduinoPinSetState(PIN_HOSPITAL_HEATER, 0, false);

	solenoid_SET(SOLENOID, 0);

	deviceSetState(LIGHT_1, getDeviceStateByTime(LIGHT_1, settings.light1OnTime * SECS_PER_MIN, settings.light1OffTime * SECS_PER_MIN));
	deviceSetState(LIGHT_2, getDeviceStateByTime(LIGHT_2, settings.light2OnTime * SECS_PER_MIN, settings.light2OffTime * SECS_PER_MIN));
	deviceSetState(LIGHT_3, getDeviceStateByTime(LIGHT_3, settings.light3OnTime * SECS_PER_MIN, settings.light3OffTime * SECS_PER_MIN));
	deviceSetState(LIGHT_4, getDeviceStateByTime(LIGHT_4, settings.light4OnTime * SECS_PER_MIN, settings.light4OffTime * SECS_PER_MIN));
	deviceSetState(LIGHT_5, getDeviceStateByTime(LIGHT_5, settings.light5OnTime * SECS_PER_MIN, settings.light5OffTime * SECS_PER_MIN));
	deviceSetState(MOON_LIGHT, getDeviceStateByTime(MOON_LIGHT, settings.moonLightOnTime * SECS_PER_MIN, settings.moonLightOffTime * SECS_PER_MIN));

	deviceSetState(O2, getDeviceStateByTime(O2, settings.o2OnTime * SECS_PER_MIN, settings.o2OffTime * SECS_PER_MIN));
	deviceSetState(CO2, getDeviceStateByTime(CO2, settings.co2OnTime * SECS_PER_MIN, settings.co2OffTime * SECS_PER_MIN));
	deviceSetState(CO2_PUMP, DEVICE_ON);
	deviceSetState(WATER_DRAIN_PUMP, DEVICE_OFF);

	deviceSetState(SUMP_RECIRCULATE_PUMP, DEVICE_ON);
	deviceSetState(FILTER_1, DEVICE_ON);
	deviceSetState(FILTER_2, DEVICE_ON);

	setDailyAlarm(settings.light1OnTime * SECS_PER_MIN, deviceSetState, "Light 1 ON", LIGHT_1, DEVICE_ON);
	setDailyAlarm(settings.light1OffTime * SECS_PER_MIN, deviceSetState, "Light 1 OFF", LIGHT_1, DEVICE_OFF);

	setDailyAlarm(settings.light2OnTime * SECS_PER_MIN, deviceSetState, "Light 2 ON", LIGHT_2, DEVICE_ON);
	setDailyAlarm(settings.light2OffTime * SECS_PER_MIN, deviceSetState, "Light 2 OFF", LIGHT_2, DEVICE_OFF);

	setDailyAlarm(settings.light3OnTime * SECS_PER_MIN, deviceSetState, "Light 3 ON", LIGHT_3, DEVICE_ON);
	setDailyAlarm(settings.light3OffTime * SECS_PER_MIN, deviceSetState, "Light 3 OFF", LIGHT_3, DEVICE_OFF);

	setDailyAlarm(settings.light4OnTime * SECS_PER_MIN, deviceSetState, "Light 4 ON", LIGHT_4, DEVICE_ON);
	setDailyAlarm(settings.light4OffTime * SECS_PER_MIN, deviceSetState, "Light 4 OFF", LIGHT_4, DEVICE_OFF);

	setDailyAlarm(settings.light5OnTime * SECS_PER_MIN, deviceSetState, "Light 5 ON", LIGHT_5, DEVICE_ON);
	setDailyAlarm(settings.light5OffTime * SECS_PER_MIN, deviceSetState, "Light 5 OFF", LIGHT_5, DEVICE_OFF);

	setDailyAlarm(settings.moonLightOnTime * SECS_PER_MIN, deviceSetState, "Moon Light ON", MOON_LIGHT, DEVICE_ON);
	setDailyAlarm(settings.moonLightOffTime * SECS_PER_MIN, deviceSetState, "Moon Light OFF", MOON_LIGHT, DEVICE_OFF);

	setDailyAlarm(settings.o2OnTime * SECS_PER_MIN, deviceSetState, "O2 ON", O2, DEVICE_ON);
	setDailyAlarm(settings.o2OffTime * SECS_PER_MIN, deviceSetState, "O2 OFF", O2, DEVICE_OFF);

	setDailyAlarm(settings.co2OnTime * SECS_PER_MIN, deviceSetState, "CO2 ON", CO2, DEVICE_ON);
	setDailyAlarm(settings.co2OffTime * SECS_PER_MIN, deviceSetState, "CO2 OFF", CO2, DEVICE_OFF);

	setDailyAlarm(settings.waterChange1StartTime * SECS_PER_MIN, waterChangeTimerDrain, "Start Water Change 1 (drain)", 1, DEVICE_ON);
	setDailyAlarm(settings.waterChange2StartTime * SECS_PER_MIN, waterChangeTimerDrain, "Start Water Change 2 (drain)", 2, DEVICE_ON);

	setDailyAlarm(settings.waterChange1StartTime * SECS_PER_MIN + settings.waterDrainDuration + 10, waterChangeTimerFill, "Start Water Change 1 (fill)", 1, DEVICE_ON);
	setDailyAlarm(settings.waterChange2StartTime * SECS_PER_MIN + settings.waterDrainDuration + 10, waterChangeTimerFill, "Start Water Change 2 (fill)", 2, DEVICE_ON);

	setDailyAlarm(settings.waterChange1StartTime * SECS_PER_MIN + settings.waterDrainDuration + settings.waterFillDuration + 20, waterChangeTimerDrain2, "Start Water Change 1 (extra drain)", 1, DEVICE_ON);
	setDailyAlarm(settings.waterChange2StartTime * SECS_PER_MIN + settings.waterDrainDuration + settings.waterFillDuration + 20, waterChangeTimerDrain2, "Start Water Change 2 (extra drain)", 2, DEVICE_ON);

	setDailyAlarm(settings.dosingMacroStartTime * SECS_PER_MIN, dosingMacroTimer, "Start Dosing Macro", 1, DEVICE_ON);
	setDailyAlarm(settings.dosingMicroStartTime * SECS_PER_MIN, dosingMicroTimer, "Start Dosing Micro", 1, DEVICE_ON);

	setDailyAlarm(settings.feed1StartTime * SECS_PER_MIN, fishFeederTimer, "Fish Feed 1", 1, DEVICE_ON);
	setDailyAlarm(settings.feed3StartTime * SECS_PER_MIN, fishFeederTimer, "Fish Feed 2", 2, DEVICE_ON);
	setDailyAlarm(settings.feed3StartTime * SECS_PER_MIN, fishFeederTimer, "Fish Feed 3", 3, DEVICE_ON);
}

AlarmID_t setDailyAlarm(time_t value, OnTick_t onTickHandler, const char* eventName, byte id, int state)  // trigger daily at given time of day
{
	if (value != AlarmHM_Minutes(0, 0))
		Alarm.alarmRepeat(value, onTickHandler, eventName, id, state);
}

boolean getDeviceStateByTime(byte device_id, time_t onTime, time_t offTime)
{
	time_t time_now = now();
	time_now = time_now - previousMidnight(time_now);

	boolean b;
	if (onTime < offTime)
	{
		b = (time_now >= onTime) && (time_now < offTime);
	}
	else
	if (onTime > offTime)
	{
		time_t t = onTime;
		onTime = offTime;
		offTime = t;
		b = !((time_now >= onTime) && (time_now < offTime));
	}
	else // if equals
	{
		if (deviceGetDefaultState(device_id) != DEVICE_FREE)
			b = deviceGetDefaultState(device_id);
		else
			b = false;
	}
	return b;
}

unsigned long prevSumpFullSeconds = 0;

// processes water levels and shuts down Solenoid if necessary
void processWaterLevels()
{
	aquaGodState.setWaterIsOnFloor1(!digitalRead(PIN_WATER_IS_ON_FLOOR1));
	aquaGodState.setWaterIsOnFloor2(!digitalRead(PIN_WATER_IS_ON_FLOOR2));

	aquaGodState.setWaterLevelIsCriticallyHighInAquarium(digitalRead(PIN_WATER_LEVEL_AQUARIUM_HIGH));
	aquaGodState.setWaterLevelIsLowInHospital(!digitalRead(PIN_WATER_LEVEL_HOSPITAL_LOW));

	aquaGodState.setWaterLevelIsCriticallyLowInSump(digitalRead(PIN_WATER_LEVEL_SUMP_LOW));
	aquaGodState.setWaterLevelIsCriticallyHighInSump(digitalRead(PIN_WATER_LEVEL_SUMP_HIGH));

	processUltrasonicSensors();
	int sumpLevel = aquaGodState.getSumpWaterLevel();

	if (deviceGetState(SOLENOID) > 0)
	{
		// if water is on floor or sump is full (both sensors)
		if (aquaGodState.getWaterIsOnFloor() || (aquaGodState.getWaterLevelIsCriticallyHighInSump() && (sumpLevel >= 100)))
		{
			prevSumpFullSeconds = 0;

			//Turn off solenoid
			solenoid_SET(SOLENOID, DEVICE_OFF);
		}
		else
			// if water sump is full (at least one sensor)
		if (aquaGodState.getWaterLevelIsCriticallyHighInSump() || (sumpLevel >= 100))
		{
			if (prevSumpFullSeconds == 0)
				prevSumpFullSeconds = secondTicks;

			if ((secondTicks - prevSumpFullSeconds) >= 600) // 10 min
			{
				prevSumpFullSeconds = 0;

				//Turn off solenoid in 10 min
				solenoid_SET(SOLENOID, DEVICE_OFF);
			}
		}
		else
			prevSumpFullSeconds = 0;
	}

	// Sump pump
	if (aquaGodState.getWaterLevelIsCriticallyLowInSump() || aquaGodState.getWaterLevelIsCriticallyHighInAquarium() || (sumpLevel == 0))
	{
		//Turn off fill
		if (deviceGetState(WATER_FILL_PUMP) > 0)
			deviceSetState(WATER_FILL_PUMP, DEVICE_OFF);
	}

	if (aquaGodState.getWaterLevelIsCriticallyHighInAquarium())
	{
		//Turn off drain
		if (deviceGetState(WATER_DRAIN_PUMP) > 0)
			deviceSetState(WATER_DRAIN_PUMP, DEVICE_OFF);
	}


	switch (waterChangeState)
	{
	WATER_CHANGE_STATE_DRAIN:
		if (aquaGodState.getAquariumWaterLevel() < settings.waterDrainLevel)
			deviceSetState(WATER_DRAIN_PUMP, 0);
		break;
	WATER_CHANGE_STATE_FILL:
		if (aquaGodState.getAquariumWaterLevel() > 110)
			deviceSetState(WATER_FILL_PUMP, 0);
		break;
	WATER_CHANGE_STATE_DRAIN2:
		if (aquaGodState.getAquariumWaterLevel() <= 100)
			deviceSetState(WATER_DRAIN_PUMP, 0);
		break;
	default:
		break;
	}
}

extern unsigned long prevSolenoidOff;

void processWaterLevelForSolenoidOn()
{
	if (secondTicks < (prevSolenoidOff + 300)) // 5 min
		return;

	if (deviceGetState(SOLENOID) > 0)
		return;

	int sumpLevel = aquaGodState.getSumpWaterLevel();
	// if water is on floor or sump is full (both sensors)
	if (aquaGodState.getWaterIsOnFloor() || aquaGodState.getWaterLevelIsCriticallyHighInSump() || (sumpLevel > 92))
		return;

	//Turn on solenoid
	solenoid_SET(SOLENOID, DEVICE_ON);
}

// called every second
void processPumps()
{
	int state = deviceGetState(WATER_FILL_PUMP);
	state--;
	if (state >= 0)
		deviceSetState(WATER_FILL_PUMP, state);

	state = deviceGetState(WATER_DRAIN_PUMP);
	state--;
	if (state >= 0)
		deviceSetState(WATER_DRAIN_PUMP, state);

	state = deviceGetState(DOSING_PUMP_MACRO);
	state--;
	if (state >= 0)
		deviceSetState(DOSING_PUMP_MACRO, state);

	state = deviceGetState(DOSING_PUMP_MICRO);
	state--;
	if (state >= 0)
		deviceSetState(DOSING_PUMP_MICRO, state);
}


boolean aquariumHeater_SET(byte device_id, int state)
{
	if ((state < 0) || ((device_id == HEATER) && (deviceGetState(FILTER_2) <= 0))) // if filter 2 is not working, turn off main heaters
		state = 0;
	else if (state > 100)
		state = 100;

	deviceSetCurrentState(device_id, state);
}

boolean co2_SET(byte device_id, int state)
{
	if (deviceGetState(FILTER_2) > 0)
		return deviceRelaySetState(device_id, state);
	else
		return deviceRelaySetState(device_id, 0);
}

boolean uvlight_SET(byte device_id, int state)
{
	if (deviceGetState(FILTER_1) > 0)
		return deviceRelaySetState(device_id, state);
	else
		return deviceRelaySetState(device_id, 0);
}

boolean filter_SET(byte device_id, int state)
{
	if (state <= 0)
	{
		if (device_id == FILTER_1)
		{
			deviceSetState(UV_LIGHT, 0); // Turn of UV light
		}
		else
		if (device_id == FILTER_2)
		{
			deviceSetState(HEATER, 0); // Turn of main heaters
			deviceSetState(CO2, 0); // Turn of co2
		}
	}

	deviceRelaySetState(device_id, state);
}

boolean drainFill_SET(byte device_id, int state)
{
	if (state <= 0)
		waterChangeState = WATER_CHANGE_STATE_NONE;

	deviceRelaySetState(device_id, state);
}

void processAquariumHeaters()
{
	static unsigned long windowStartTime = 0;

	if (secondTicks - windowStartTime > 100) //time to shift the Relay Window
		windowStartTime += 100;

	int output = deviceGetState(HEATER);
	if (output < secondTicks - windowStartTime)
	{
		arduinoPinSetState(PIN_HEATER_1, false, false);
		arduinoPinSetState(PIN_HEATER_2, false, false);
	}
	else
	{
		arduinoPinSetState(PIN_HEATER_1, true, false);
		arduinoPinSetState(PIN_HEATER_2, true, false);
	}

	output = deviceGetState(SUMP_HEATER);
	if (output < secondTicks - windowStartTime)
	{
		arduinoPinSetState(PIN_SUMP_HEATER, false, false);
	}
	else
	{
		arduinoPinSetState(PIN_SUMP_HEATER, true, false);
	}

	output = deviceGetState(HOSPITAL_HEATER);
	if (output < secondTicks - windowStartTime)
	{
		arduinoPinSetState(PIN_HOSPITAL_HEATER, false, false);
	}
	else
	{
		arduinoPinSetState(PIN_HOSPITAL_HEATER, true, false);
	}
}

void processAquarium()
{
	if (deviceGetState(MAINTENANCE_MODE) > 0)
		return;

	checkCO2();

	int T = aquaGodState.getAquariumTemperatureMean();

	// PID Heater
	int deltaT = (settings.aquariumTemperature - T) * 2;  // DIRECT. 0.5 degree = 50 in delta. So multiply it by 2 to get 0 - 100 range
	if (deltaT < 0)
		deltaT = 0;
	else if (deltaT > 100)
		deltaT = 100;

	deviceSetState(HEATER, deltaT);

	// PID Sump Heater
	deltaT = (settings.aquariumTemperature - aquaGodState.getSumpTemperature()) * 2;  // DIRECT. 0.5 degree = 50 in delta. So multiply it by 2 to get 0 - 100 range
	if (deltaT < 0)
		deltaT = 0;
	else if (deltaT > 100)
		deltaT = 100;

	deviceSetState(SUMP_HEATER, deltaT);

	if (settings.hospitalTemperature > 0)
	{
		// PID Hospital Heater
		deltaT = (settings.hospitalTemperature - aquaGodState.getHospitalTemperature()) * 2;  // DIRECT. 0.5 degree = 50 in delta. So multiply it by 2 to get 0 - 100 range
		if (deltaT < 0)
			deltaT = 0;
		else if (deltaT > 100)
			deltaT = 100;

		deviceSetState(HOSPITAL_HEATER, deltaT);
	}
	else
		deviceSetState(HOSPITAL_HEATER, 0);
}

boolean waterChangeTimerDrain(byte id, int tag)
{
	if (bitRead(settings.waterChangeDOW, weekday() - 1) != 0) // (Sunday is day 1)  
	{
		int TA = aquaGodState.getAquariumTemperatureMean();
		int TS = aquaGodState.getSumpTemperature();

		if ((TA >= 9000) || (TS >= 9000))
			return false;
		if (((TA - TS) > 100) || (TS - TA) > 200) // if more than 1 or 2 degree celsius difference
			return false;

		Log.Info(F("Changing water (drain)"));
		waterChangeState = WATER_CHANGE_STATE_DRAIN;
		return deviceSetState(WATER_DRAIN_PUMP, settings.waterDrainDuration);
	}
	else
		return false;
}

boolean waterChangeTimerFill(byte id, int tag)
{
	if (bitRead(settings.waterChangeDOW, weekday() - 1) != 0) // (Sunday is day 1)  
	{
		int TA = aquaGodState.getAquariumTemperatureMean();
		int TS = aquaGodState.getSumpTemperature();

		if ((TA >= 9000) || (TS >= 9000))
			return false;
		if (((TA - TS) > 100) || (TS - TA) > 200) // if more than 1 or 2 degree celsius difference
			return false;

		Log.Info(F("Changing water (fill)"));
		waterChangeState = WATER_CHANGE_STATE_FILL;
		return deviceSetState(WATER_FILL_PUMP, settings.waterFillDuration);
	}
	else
		return false;
}

boolean waterChangeTimerDrain2(byte id, int tag)
{
	if (bitRead(settings.waterChangeDOW, weekday() - 1) != 0) // (Sunday is day 1)  
	{
		Log.Info(F("Changing water (extra drain)"));
		waterChangeState = WATER_CHANGE_STATE_DRAIN2;
		return deviceSetState(WATER_DRAIN_PUMP, settings.waterDrainDuration2);
	}
	else
		return false;
}



boolean dosingMacroTimer(byte id, int tag)
{
	if (bitRead(settings.dosingMacroDOW, weekday() - 1) != 0) // (Sunday is day 1)  
	{
		Log.Info(F("Dosing macro fertilizers"));
		return deviceSetState(DOSING_PUMP_MACRO, settings.dosingMacroDuration);
	}
	else
		return false;
}

boolean dosingMicroTimer(byte id, int tag)
{
	if (bitRead(settings.dosingMicroDOW, weekday() - 1) != 0) // (Sunday is day 1)  
	{
		Log.Info(F("Dosing micro fertilizers"));
		return deviceSetState(DOSING_PUMP_MICRO, settings.dosingMicroDuration);
	}
	else
		return false;
}


boolean fishFeederTimer(byte id, int tag)
{
	if (bitRead(settings.feedDOW, weekday() - 1) != 0) // (Sunday is day 1)  
		return feedFish(FEEDER_1, DEVICE_ON);
	else
		return false;
}

boolean feedFish(byte id, int state)
{
	if (state == 0)
		return false;

	relayOn(id);
	delay(500);
	relayOff(id);
	return true;
}


