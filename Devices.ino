extern LiquidCrystal_I2C lcd;

enum log_types { NO_LOG, DO_LOG };

typedef struct device
{
	const char* name;
	int pin;
	boolean is_reversed;
	device_states default_state;
	log_types log_type;
	OnTick_t setStateFunction;
	boolean setIfMaintMode;
};

// relay module requires LOW to energize


// All pins are reversed by default. Relay energized when LOW
device devices[] =
{
	// Board 1 (0 - 7)
	{
		"Light 1", PIN_LIGHT_1, true, DEVICE_FREE, DO_LOG, NULL, false
	}
	,
	{
		"Light 2", PIN_LIGHT_2, true, DEVICE_FREE, DO_LOG, NULL, false
	}
		,
		{
			"Light 3", PIN_LIGHT_3, true, DEVICE_FREE, DO_LOG, NULL, false
		}
			,
			{
				"Light 4", PIN_LIGHT_4, true, DEVICE_FREE, DO_LOG, NULL, false
			}
				,
				{
					"Light 5", PIN_LIGHT_5, true, DEVICE_OFF, DO_LOG, NULL, false
				}
					,
					{
						"Exhaust Fan", PIN_EXHAUST_FAN, true, DEVICE_OFF, DO_LOG, NULL, false
					}
						,
						{
							"Feeder 1", PIN_FEEDER_1, true, DEVICE_OFF, DO_LOG, feedFish, false
						}
							,
							{
								"Feeder 2", PIN_FEEDER_2, true, DEVICE_OFF, DO_LOG, feedFish, false
							}
								,

									// Board 2 (8 - 15)
								{
									"O2", PIN_O2, false, DEVICE_ON, DO_LOG, NULL, false	// nearly always on, so not reversed
								}
									,
									{
										"CO2", PIN_CO2, true, DEVICE_OFF, DO_LOG, co2_SET, false
									}
										,
										{
											"CO2 Pump", PIN_CO2_PUMP, false, DEVICE_OFF, DO_LOG, NULL, false // nearly always on, so not reversed
										}
											,
											{
												"Drain Pump", PIN_WATER_DRAIN_PUMP, true, DEVICE_OFF, DO_LOG, drainFill_SET, false
											}
												,
												{
													"DEVICE_220_A", PIN_220A, true, DEVICE_FREE, DO_LOG, NULL, false
												}
													,
													{
														"DEVICE_220_B", PIN_220B, true, DEVICE_FREE, DO_LOG, NULL, false
													}
														,
														{
															"Moon Light", PIN_MOON_LIGHT, true, DEVICE_FREE, DO_LOG, NULL, false // 2 sockets
														}
															,
															{
																"B2 Relay 8", PIN_BOARD2_RELAY_FREE, true, DEVICE_FREE, DO_LOG, NULL, false // not connected
															}
																,
																	// Board 3 (15 - 23)
																{
																	"Fill Pump", PIN_WATER_FILL_PUMP, true, DEVICE_OFF, NO_LOG, drainFill_SET, false
																}
																	,
																	{
																		"Dosing Macro", PIN_DOSING_PUMP_MACRO, true, DEVICE_OFF, NO_LOG, NULL, false
																	}
																		,
																		{
																			"Dosing Micro", PIN_DOSING_PUMP_MICRO, true, DEVICE_OFF, NO_LOG, NULL, false
																		}
																			,
																			{
																				"Sump R. Pump", PIN_SUMP_RECIRCULATE_PUMP, false, DEVICE_ON, DO_LOG, NULL, false // nearly always on, so not reversed
																			}
																				,
																				{
																					"UV Light", PIN_UV_LIGHT, true, DEVICE_OFF, DO_LOG, uvlight_SET, false
																				}
																					,
																					{
																						"Filter 2", PIN_CANISTER_FILTER2, false, DEVICE_ON, DO_LOG, filter_SET, false // nearly always on, so not reversed
																					}
																						,
																						{
																							"Filter 1", PIN_CANISTER_FILTER1, false, DEVICE_ON, DO_LOG, filter_SET, false	// nearly always on, so not reversed
																						}
																							,
																							{
																								"Hospital Light", PIN_HOSPITAL_LIGHT, true, DEVICE_FREE, DO_LOG, NULL, false
																							}
																								,

																									// Other (24 - 29)
																								{
																									"Heater", -1, false, DEVICE_OFF, NO_LOG, aquariumHeater_SET, false
																								}
																									,
																									{
																										"Sump Heater", -1, false, DEVICE_OFF, NO_LOG, aquariumHeater_SET, false
																									}
																										,
																										{
																											"Hosp. Heater", -1, false, DEVICE_OFF, NO_LOG, aquariumHeater_SET, false
																										}
																											,
																											{
																												"Solenoid", -1, false, DEVICE_OFF, DO_LOG, solenoid_SET, false
																											}
																												,
																												{
																													"Board Fan", PIN_BOARD_FAN_PWM, false, DEVICE_FREE, NO_LOG, boardFanSet, true
																												}
																													,
																													{
																														"Maint. Mode", -1, false, DEVICE_FREE, DO_LOG, maintMode_SET, true
																													}
};

const int DEVICE_COUNT = sizeof(devices) / sizeof(device);
const int RELAY_COUNT = 24;

int deviceCurrentState[DEVICE_COUNT];


///////////////////////////////////////////////////////////////


void initDevices()
{
	deviceSetCurrentState(MAINTENANCE_MODE,  0);
	for (byte device_id = 0; device_id < DEVICE_COUNT; device_id++)
	{
		delay(10);
		if (devices[device_id].pin > 0)
		{
			digitalWrite(devices[device_id].pin, HIGH);
			pinMode(devices[device_id].pin, OUTPUT);
		}

		if (devices[device_id].default_state != DEVICE_FREE)
			deviceSetState(device_id, devices[device_id].default_state);
		else
			deviceSetCurrentState(device_id, DEVICE_OFF);
	}

	pinMode(PIN_HEATER_1, OUTPUT);
	pinMode(PIN_HEATER_2, OUTPUT);
	pinMode(PIN_SUMP_HEATER, OUTPUT);
	pinMode(PIN_HOSPITAL_HEATER, OUTPUT);
	pinMode(PIN_SOLENOID_IN1, OUTPUT);
	pinMode(PIN_SOLENOID_IN2, OUTPUT);
}

void deviceSwitchState(byte device_id)
{
	if (deviceGetState(device_id) > 0)
		deviceSetState(device_id, 0);
	else
		deviceSetState(device_id, 1);
}

// returns TRUE if state changed
boolean deviceSetState(byte device_id, int state)
{
	if ((state > 0) && deviceGetState(MAINTENANCE_MODE) > 0)
		return false;

	return deviceSetStateNoMainModeCheck(device_id, state);
}

// returns TRUE if state changed
boolean deviceSetStateNoMainModeCheck(byte device_id, int state)
{
	if (devices[device_id].setStateFunction != NULL)
		return devices[device_id].setStateFunction(device_id, state);
	else
		return deviceRelaySetState(device_id, state);
}

void arduinoPinSetState(byte pin, boolean on, boolean isReversed)
{
	if (isReversed)
		on = !on;

	if (pin > 0)
	{
		if (on)
			digitalWrite(pin, HIGH);
		else
			digitalWrite(pin, LOW);
	}
}

// returns TRUE if state changed
boolean deviceRelaySetState(byte device_id, int state)
{
	boolean stateChanged = deviceGetState(device_id) != state;
	if (stateChanged || initializingBoard)
	{
		arduinoPinSetState(devices[device_id].pin, state > 0, devices[device_id].is_reversed);
		deviceSetCurrentState(device_id, state);

		deviceLog(device_id, state);
	}
	return stateChanged;
}


void deviceSetCurrentState(byte device_id, int state)
{
	if (deviceGetState(device_id) != state)
	{
		deviceCurrentState[device_id] = state;

		//Inform host
		if (!initializingBoard)
		{
			Serial.print("@S{\"");
			Serial.print(device_id);
			Serial.print("\"");
			Serial.print(':');
			Serial.print(state);
			Serial.println("}");
		}
	}
}

// returns TRUE if state changed
boolean relayOn(byte device_id)
{
	return deviceRelaySetState(device_id, DEVICE_ON);
}

// returns TRUE if state changed
boolean relayOff(byte device_id)
{
	return deviceRelaySetState(device_id, DEVICE_OFF);
}

inline int deviceGetState(byte device_id)
{
	return deviceCurrentState[device_id];
}

inline int deviceGetDefaultState(byte device_id)
{
	return devices[device_id].default_state;
}

void deviceLog(byte device_id, int state)
{
	if (devices[device_id].log_type != NO_LOG)
		Log.Info(F("%s: %d"), devices[device_id].name, state);
	else
		Log.Debug(F("%s: %d"), devices[device_id].name, state);

	if (aquaGodState.getLcdIsPresent())
	{
		clearLcdRow(3);
		lcd.print(devices[device_id].name);
		lcd.print(F(": "));
		lcd.print(state);
	}
}

unsigned long prevSolenoidOff = 0;

boolean solenoid_SET(byte device_id, int state)
{
	arduinoPinSetState(PIN_SOLENOID_IN1, 0, false);
	arduinoPinSetState(PIN_SOLENOID_IN2, 0, false);
	delay(10);
	if (state)
		arduinoPinSetState(PIN_SOLENOID_IN1, true, false);
	else
		arduinoPinSetState(PIN_SOLENOID_IN2, true, false);
	deviceSetCurrentState(device_id, state);

	delay(100);

	arduinoPinSetState(PIN_SOLENOID_IN1, 0, false);
	arduinoPinSetState(PIN_SOLENOID_IN2, 0, false);

	if (devices[device_id].log_type != NO_LOG)
		Log.Info(F("%s: %d"), devices[device_id].name, state);

	if (state == 0)
		prevSolenoidOff = secondTicks;
	return true;
}


boolean maintMode_SET(byte id, int state)
{
	deviceSetCurrentState(MAINTENANCE_MODE, state);
	if (state == 0)
	{
		Log.Info(F("Maintenence Mode: OFF"));
		reInitAquarium();
	}
	else
	{
		Log.Info(F("Maintenence Mode: ON"));

		deviceSetState(HEATER, 0);
		deviceSetState(SUMP_HEATER, 0);
		deviceSetState(HOSPITAL_HEATER, 0);

		for (byte device_id = 8; device_id < 24; device_id++) // from 1st relay of board 2 to last relay of board 3
		{
			if ((device_id != FILTER_1) && (device_id != FILTER_2))
				deviceSetState(device_id, DEVICE_OFF);
		}
	}
	return true;
}

boolean boardFanSet(byte id, int state) // state = %
{
	if (state < 0)
		state = 0;
	else if (state > 100)
		state = 100;

	if ((state > 0) && (state < 20))
		state = 20;

	pwmWrite(PIN_BOARD_FAN_PWM, state * 255 / 100);
	deviceSetCurrentState(BOARD_FAN, state);
	return true;
}


volatile unsigned int board_fan_half_revolutions = 0;

void boardFanTachInterrupt()
{
	board_fan_half_revolutions++;
	//Each rotation, this interrupt function is run twice
}

void processFans()
{
	aquaGodState.setBoardFanRPM(board_fan_half_revolutions * 30 / PROCESS_INTERVAL_SEC);
	board_fan_half_revolutions = 0;

	// PID
	int deltaT = (aquaGodState.getBoardTemperature() - settings.boardTempMax) / 10;  // REVERSE. 10 degree = 1000 in delta. So divide it by 10 to get 0 - 100 range
	if (deltaT < 0)
		deltaT = 0;
	else if (deltaT > 100)
		deltaT = 100;

	deviceSetState(BOARD_FAN, deltaT);
}












