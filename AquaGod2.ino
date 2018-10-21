#define REQUIRESALARMS false // FOR DS18B20 library
#define MQTT_MAX_PACKET_SIZE 256 // FOR PubSubClient library
#define MQTT_SOCKET_TIMEOUT 5 // FOR PubSubClient library

#define USE_NEW_KEYPAD

// https://www.youtube.com/watch?v=uDdiMMdVb90
//http://www.atlas-scientific.com/_files/code/Arduino-sample-code-EZ-COM-MEGA.pdf
//https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home


// Read info in constants.h
#include "AquaGod2.h"
#include "AquaGodState.h"

#include <Wire.h> // 207 bytes
#include <OneWire.h>  // https://github.com/PaulStoffregen/OneWire

//#include <DallasTemperature.h>

#include <Time.h> // 68 bytes
#include <TimeAlarms.h> // 0 bytes
#include <DS1307RTC.h> // 0 bytes // a basic DS1307 library that returns time as a time_t
#include <SPI.h> // 0 bytes
#include <Ethernet.h> // 668 bytes
#include <WebServer.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include <Logging.h>
#include <DHT.h>

#ifdef USE_NEW_KEYPAD
#include <Keypad.h>
#include <Keypad_I2C.h>
#else
#include <i2ckeypad.h>
#endif

#include <NewPing.h>
#include <PWM.h>
#include <MemoryFree.h>

/////////// WARNING ///////////////
// TimeAlarms only supported 6 alarms, so TimeAlarms.h modified with this: #define dtNBR_ALARMS 30  // max is 255
///////////////////////////////////

AquaGodState aquaGodState;

String inData = "";

bool initializingBoard = true;
unsigned long secondTicks = 0;

Logger logger;

///////////////////

void setup()
{
	initializingBoard = true;

	delay(10);

	if (dtNBR_ALARMS != 30)
		aquaGodState.setErrorCode(ERR_SYSTEM);

	Log.Init(LOG_LEVEL_VERBOSE, &logger); // also calls Serial.begin
	Log.Info(F("\r\nInitializing.. ver. 1.0.0"), 0);

	pinMode(PIN_BLINKING_LED, OUTPUT);
	//pinMode(PIN_SD_CARD, OUTPUT);
	pinMode(PIN_ETHERNET, OUTPUT);

	// On the Ethernet Shield, CS is pin 4. It's set as an output by default.
	// Note that even if it's not used as the CS pin, the hardware SS pin 
	// (10 on most Arduino boards, 53 on the Mega) must be left as an output 
	// or the SD library functions will not work. 
	pinMode(SS, OUTPUT);

	digitalWrite(PIN_BLINKING_LED, HIGH);
	//digitalWrite(PIN_SD_CARD, HIGH); // Disable SD card
	digitalWrite(PIN_ETHERNET, HIGH); // Disable Ethernet card

	//analogReference(EXTERNAL); //TODO
	pinMode(PIN_WATER_LEVEL_AQUARIUM_HIGH, INPUT_PULLUP);
	pinMode(PIN_WATER_LEVEL_HOSPITAL_LOW, INPUT_PULLUP);

	pinMode(PIN_WATER_LEVEL_SUMP_HIGH, INPUT_PULLUP);
	pinMode(PIN_WATER_LEVEL_SUMP_LOW, INPUT_PULLUP);

	Wire.begin();

	initLCD();

	readSettings();

	// Sync time with RTC
	setSyncProvider(RTC.get);   // the function to get the time from the RTC
	unsigned long startMillis = millis();
	do
	{
		delay(100);
		if (timeStatus() == timeSet)
			break;
	} while (millis() - startMillis < 1000);
	if (timeStatus() != timeSet)
		aquaGodState.setErrorCode(ERR_TIME_NOT_SET);

	//	//initialize all timers except for 0, to save time keeping functions
	InitTimersSafe();
	SetPinFrequencySafe(PIN_BOARD_FAN_PWM, 25000);

	//	initSD();

	initDevices();

	initTempSensors();

	initAquarium();

	initPH();

	initWWW();

	initKeypad();

	showLcdInfo();

	aquaGodState.setStartTime(now());
	Alarm.timerRepeat(1, oncePerSecond, NULL, 0, 0);

	pinMode(PIN_BOARD_FAN_TACH, INPUT_PULLUP);
	attachInterrupt(0, boardFanTachInterrupt, RISING);

	delay(1000);

	if (aquaGodState.getErrorCode() != 0)
		Log.Info(F("Error : %x"), aquaGodState.getErrorCode());
	Log.Info(F("Start. Free memory: %d bytes"), freeMemory());

	processWaterLevels(); // duplicate. same is in loop

	initializingBoard = false;
	aquaGodState.setEventsEnabled(true);

	outputAquaGodInternalState();
}


void loop()
{
	static unsigned long previousMillis = 0xFFFFFFFF; // will store last time LED was updated
	unsigned long _current_millis = millis();

	processKeypad();

	if ((_current_millis < previousMillis) || (_current_millis - previousMillis >= 500))
	{
		// save the last time we blinked the LED 
		previousMillis = _current_millis;
		oncePerHalfSecond();
	}

	while (Serial.available() > 0)
	{
		char serialChar = Serial.read();

		// Process message when new line character is recieved
		if (serialChar == '\n')
		{
			// You can put some if and else here to process the message juste like that:

			processCommand();
			inData = ""; // Clear recieved buffer
		}
		else
			inData += serialChar;
	}

	processWWW();
}

void oncePerHalfSecond(void)
{
	// Blinking
	static unsigned char blinkingLedState = HIGH;

	blinkingLedState = !blinkingLedState;
	digitalWrite(PIN_BLINKING_LED, blinkingLedState);

	// service the alarm timers once per blink
	Alarm.delay(0);
}


void oncePerSecond(int id, int tag)
{
	processWaterLevels(); // processes water levels and shuts down Solenoid if necessary

	if ((secondTicks + 2) % PROCESS_INTERVAL_SEC == 0) // 2 seconds before processing aquarium
	{
		startTemperatureMeasurements(true);
		startPhMeasurements();
	}

	processPumps();

	processAquariumHeaters();


	if (secondTicks % PROCESS_INTERVAL_SEC == 0)
		oncePerProcessInterval();

	if (secondTicks % 60 == 0)
		oncePerMinute();

	secondTicks++;
	return true;
}

void oncePerProcessInterval()
{
	readTemperatures();

	processAquarium();

	checkCO2();

	showLcdInfo();

#ifdef DEBUGGING
	/*Serial.print(F("Sump water level: "));
	Serial.println(aquaGodState.sumpWaterLevel);

	Serial.print(F("PH 1 & 2: "));
	Serial.print(aquaGodState.aquariumPH);
	Serial.print(" ");
	Serial.println(aquaGodState.sumpPH);
	*/

	//		Serial.println(F("State"));
	//		Serial.print("T_AQ_1: ");
	//		Serial.println(aquaGodState.aquariumTemperature1);
	//		Serial.print("T_AQ_2: ");
	//		Serial.println(aquaGodState.aquariumTemperature2);
	//		Serial.print("T_BRD: ");
	//		Serial.println(aquaGodState.boardTemperature);
#endif
}

void oncePerMinute()
{
	readRoomState();

	processFans();

	processWaterLevelForSolenoidOn();
}

void printDateTime(Print &server, time_t value)
{
	server.print(day(value));
	server.print('/');
	server.print(month(value));
	server.print('/');
	server.print(year(value) - 2000);
	server.print(' ');

	server.print(hour(value));
	server.print(':');
	server.print(minute(value));
	server.print(':');
	server.print(second(value));
	server.print(' ');
}


void writeJsonToSerial(long value, int id, bool is_last)
{
	Serial.print(F("\""));
	Serial.print(id);
	Serial.print(F("\":"));
	Serial.print(value);
	if (!is_last)
		Serial.print(F(","));
}

extern Settings settings;
extern const int DEVICE_COUNT;

void outputAquaGodInternalState()
{
	int value;

	Serial.print(F("@I"));
	Serial.print(F("{\"Settings\": {"));
	for (byte setting_id = 0; setting_id < SETTINGS_COUNT; setting_id++)
	{
		value = *((int*)&settings + setting_id);
		writeJsonToSerial(value, setting_id, setting_id == SETTINGS_COUNT - 1);
	}
	Serial.print(F("},"));

	Serial.print(F("\"State\": {"));
	for (byte device_id = 0; device_id < DEVICE_COUNT; device_id++)
		writeJsonToSerial(deviceGetState(device_id), device_id, false);

	int id = 102;
	writeJsonToSerial(aquaGodState.getAquariumPH(), id++, false);
	id++;
	writeJsonToSerial(aquaGodState.getSumpPH(), id++, false);
	writeJsonToSerial(aquaGodState.getSumpWaterLevel(), id++, false);	// 105
	writeJsonToSerial(aquaGodState.getSumpWaterLevelMm(), id++, false); // 106
	id++;
	writeJsonToSerial(aquaGodState.getRoomTemperature(), id++, false); // 108
	writeJsonToSerial(aquaGodState.getRoomHumidity(), id++, false);  // 109
	writeJsonToSerial(aquaGodState.getBoardTemperature(), id++, false); // 110
	writeJsonToSerial(aquaGodState.getBoardFanRPM(), id++, false); // 111
	writeJsonToSerial(aquaGodState.getAquariumWaterLevel(), id++, false);	// 112
	writeJsonToSerial(aquaGodState.getAquariumWaterLevelMm(), id++, false); // 113

	writeJsonToSerial(aquaGodState.getAquariumTemperatureMean(), 399, false); // 399
	id = 400;
	writeJsonToSerial(aquaGodState.getAquariumTemperature1(), id++, false);
	writeJsonToSerial(aquaGodState.getAquariumTemperature2(), id++, false);
	writeJsonToSerial(aquaGodState.getAquariumTemperature3(), id++, false);
	writeJsonToSerial(aquaGodState.getSumpTemperature(), id++, false);
	writeJsonToSerial(aquaGodState.getHospitalTemperature(), id++, false);

	//if (aquaGodState.getWaterLevelIsCriticallyHighInAquarium())
	writeJsonToSerial(aquaGodState.getWaterLevelIsCriticallyHighInAquarium(), 200, false);
	//if (aquaGodState.getWaterLevelIsCriticallyLowInSump())
	writeJsonToSerial(aquaGodState.getWaterLevelIsCriticallyLowInSump(), 201, false);
	//if (aquaGodState.getWaterLevelIsCriticallyHighInSump())
	writeJsonToSerial(aquaGodState.getWaterLevelIsCriticallyHighInSump(), 202, false);
	//if (aquaGodState.getWaterLevelIsLowInHospital())
	writeJsonToSerial(aquaGodState.getWaterLevelIsLowInHospital(), 203, false);
	//if (aquaGodState.getWaterIsOnFloor1())
	writeJsonToSerial(aquaGodState.getWaterIsOnFloor1(), 204, false);
	//if (aquaGodState.getWaterIsOnFloor2())
	writeJsonToSerial(aquaGodState.getWaterIsOnFloor2(), 205, false);

	//if (aquaGodState.getErrorCode() != 0)
	writeJsonToSerial(aquaGodState.getErrorCode(), 300, false);
	writeJsonToSerial(now(), 301, false);  // Now
	writeJsonToSerial(now() - aquaGodState.getStartTime(), 302, false); // UpTime
	timeStatus_t tStatus = timeStatus();
	//if (tStatus != timeSet)
	writeJsonToSerial(tStatus, 303, false);
	writeJsonToSerial(freeMemory(), 304, true);
	Serial.println(F("}}"));
}

extern int ds18b20SensorCount;
extern DS18B20Sensors tempSensors;

void outputDS18B20Data()
{
	Serial.print(F("TSL"));
	Serial.print(F("{\"Data\": ["));
	for (byte i = 0; i < MAX_DS1820_SENSORS; i++)
	{
		Serial.print(F("{\"id\":"));
		Serial.print(i);
		Serial.print(F(",\"1wire_id\":"));
		Serial.print(tempSensors[i].oneWireId);
		Serial.print(F(",\"act\":"));
		Serial.print(tempSensors[i].isActive);
		Serial.print(F(",\"req\":"));
		Serial.print(tempSensors[i].isRequired);
		Serial.print(F(",\"T\":"));
		Serial.print(getTemperatureById(i));
		Serial.print(F("}"));
		if (i != MAX_DS1820_SENSORS - 1)
			Serial.print(F(","));
	}
	Serial.print(F("],"));

	Serial.print(F("\"Sensors\": ["));
	for (byte i = 0; i < ds18b20SensorCount; i++)
	{
		Serial.print(F("{\"1wire_id\":"));
		Serial.print(i);
		Serial.print(F(",\"T\":"));
		Serial.print(getTemperatureByOneWireId(i));
		Serial.print(F("}"));
		if (i != ds18b20SensorCount - 1)
			Serial.print(F(","));
	}
	Serial.print(F("]"));
	Serial.println(F("}"));
}

// echo
// @ refresh
// # set device
// $ set setting
// TSR reset DS18B20 sensor data
// TSL list DS18B20 sensor data
// TSS set DS18B20 sensor data
// ph ph command
void processCommand()
{
	if (inData.length() == 0)
		return;

	if (inData == "echo") // echo it back
	{
		Serial.println("echo");
		return;
	}

	if (inData == "@") // Refresh
	{
		outputAquaGodInternalState();
		return;
	}

	if (inData == "TSR") // Reset temp sensor data
	{
		initTempSensors();
		return;
	}

	if (inData == "TSL") // List temp sensor data
	{
		outputDS18B20Data();
		return;
	}

	if (inData.startsWith("TSS "))	// TSS 2=1 1 // sensor_id=sensor_one_wire_id  is_required
	{
		inData = inData.substring(4);

		int eqIdx = inData.indexOf('=');
		if (eqIdx < 0)
			return;
		int spaceIdx = inData.indexOf(' ', eqIdx + 1);
		if (spaceIdx < 0)
			return;

		String ssensor_id = inData.substring(0, eqIdx);
		String ssensor_1wire_id = inData.substring(eqIdx + 1, spaceIdx);
		String sIsRequired = inData.substring(spaceIdx + 1);

		int sensor_id = ssensor_id.toInt();
		int sensor_1wire_id = ssensor_1wire_id.toInt();
		bool isRequired = sIsRequired.toInt() != 0;

		ssensor_id = "";
		ssensor_1wire_id = "";
		sIsRequired = "";

		if (sensor_id >= 0 && sensor_id < MAX_DS1820_SENSORS)
		{
			if ((sensor_1wire_id == -1) || (sensor_1wire_id > 0 && sensor_1wire_id < ds18b20SensorCount))
			{
				DeviceAddress addr;
				if ((sensor_1wire_id == -1) || getSensorAddress(addr, sensor_1wire_id))
				{
					if (sensor_1wire_id == -1)
						clearSensorAddress(tempSensors[sensor_id].address);
					else
						copySensorAddress(addr, tempSensors[sensor_id].address);
					tempSensors[sensor_id].isActive = sensor_1wire_id != -1;
					tempSensors[sensor_id].isRequired = isRequired;
					tempSensors[sensor_id].oneWireId = sensor_1wire_id;

					saveDS18B20SensorsData();
					return;
				}
			}
			else
				Log.Error(F("Invalid 1wire_id"));
		}
		else
			Log.Error(F("Invalid sensor_id"));
		return;
	}

	if (inData.startsWith("#"))	// #20=1 // #device=state
	{
		int eqIdx = inData.indexOf('=');
		if (eqIdx < 0)
			return;

		String spin = inData.substring(1, eqIdx);
		String sstate = inData.substring(eqIdx + 1);
		byte id = spin.toInt();
		int state = sstate.toInt();

		spin = "";
		sstate = "";

		if (id < DEVICE_COUNT) // id >= 0 always
			deviceSetState(id, state);
		return;
	}

	if (inData.startsWith("$"))	// $20=1,21=7 // #setting=value
	{
		inData = inData.substring(1);
		while (true)
		{
			int eqIdx = inData.indexOf('=');
			if (eqIdx < 0)
				return;
			int commaIdx = inData.indexOf(',', eqIdx + 1);

			String ssetting = inData.substring(0, eqIdx);
			String svalue = "";
			if (commaIdx < 0)
				svalue = inData.substring(eqIdx + 1);
			else
				svalue = inData.substring(eqIdx + 1, commaIdx);

			byte setting_id = ssetting.toInt();
			int set_value = svalue.toInt();

			ssetting = "";
			svalue = "";

			inData = inData.substring(commaIdx + 1);

			Log.Debug(F("Set setting: %d = %d"), setting_id, set_value);
			if ((setting_id > 0) && (setting_id < SETTINGS_COUNT))
				*((int*)&settings + setting_id) = set_value;

			if (commaIdx < 0)
				break;
		}
		saveSettings();
		Log.SetLogLevel(settings.logLevel);
		reInitAquarium();
		return;
	}

	if (inData.startsWith("ph")) // ph1 X // ph# command
	{
		inData = inData.substring(2);

		String sprobe = inData.substring(0, 1);
		String scomand = inData.substring(2);

		if (scomand == "C") // continous mode
			aquaGodState.setphContinousModeEnabled(true);
		else
		if (scomand == "E") // non continous mode
			aquaGodState.setphContinousModeEnabled(false);

		if (sprobe == "1")
		{
			Serial1.print(scomand);
			Serial1.print('\r');
		}
		else
		if (sprobe == "2")
		{
			Serial2.print(scomand);
			Serial2.print('\r');
		}

		sprobe = "";
		scomand = "";
		return;
	}
}


size_t Logger::write(uint8_t c)
{
	return Serial.write(c);
}


