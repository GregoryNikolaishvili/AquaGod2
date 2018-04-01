OneWire wire1(PIN_ONE_WIRE_BUS);
DallasTemperature ds18b20Sensors(&wire1);
DHT dht;

int ds18b20SensorCount = 0;

DS18B20Sensors tempSensors;

bool getSensorAddress(DeviceAddress addr, int id)
{
	return ds18b20Sensors.getAddress(addr, id);
}

void readDS18B20SensorsData()
{
	memset(&tempSensors, 0, sizeof(tempSensors));
	eeprom_read_block(&tempSensors, STORAGE_ADDRESS_DS18B20_ADDRESS_SETTINGS, sizeof(tempSensors));
}

void saveDS18B20SensorsData()
{
	eeprom_write_block(&tempSensors, STORAGE_ADDRESS_DS18B20_ADDRESS_SETTINGS, sizeof(tempSensors));
	Log.Info(F("New DS18B20 settings applied"));
}

bool isSameSensorAddress(const DeviceAddress addr1, const DeviceAddress addr2)
{
	for (int i = 0; i < sizeof(DeviceAddress); i++)
	{
		if (addr1[i] != addr2[i])
			return false;
	}
	return true;
}

void clearSensorAddress(DeviceAddress addr)
{
	for (int i = 0; i < sizeof(DeviceAddress); i++)
		addr[i] = 0;
}

//bool isZeroAddress(const DeviceAddress addr)
//{
//	for (int i = 0; i < sizeof(DeviceAddress); i++)
//	{
//		if (addr[i] != 0)
//			return false;
//	}
//	return true;
//}

void copySensorAddress(const DeviceAddress addrFrom, DeviceAddress addrTo)
{
	for (int i = 0; i < sizeof(DeviceAddress); i++)
		addrTo[i] = addrFrom[i];
}

void initTempSensors()
{
	readDS18B20SensorsData();

	dht.setup(PIN_DHT21); // data pin 2

	ds18b20Sensors.begin();

	ds18b20SensorCount = ds18b20Sensors.getDeviceCount();

	Log.Verbose(F("DS18B20 sensors found: %d"), ds18b20SensorCount);

	if (ds18b20SensorCount == 0)
	{
		aquaGodState.setErrorCode(ERR_TEMPERATURES);
		return;
	}

	Log.Verbose(F("DS18B20 parasite mode: %t"), ds18b20Sensors.isParasitePowerMode());

	ds18b20Sensors.setWaitForConversion(false);

	for (int j = 0; j < MAX_DS1820_SENSORS; j++)
	{
		tempSensors[j].isActive = false;
		tempSensors[j].oneWireId = -1;
	}

	DeviceAddress addr;
	// first pass
	for (int i = 0; i < ds18b20SensorCount; i++)
	{
		if (ds18b20Sensors.getAddress(addr, i))
		{
			for (int j = 0; j < MAX_DS1820_SENSORS; j++)
			{
				if (isSameSensorAddress(tempSensors[j].address, addr))
				{
					tempSensors[j].isActive = true;
					tempSensors[j].oneWireId = i;
					break;
				}
			}
		}
	}

	// second pass. fill gaps if possible
	for (int i = 0; i < ds18b20SensorCount; i++)
	{
		if (ds18b20Sensors.getAddress(addr, i))
		{
			bool isUsed = false;
			for (int j = 0; j < MAX_DS1820_SENSORS; j++)
			{
				if (tempSensors[j].oneWireId == i)
				{
					isUsed = true;
					break;
				}
			}

			if (!isUsed)
			{
				for (int j = 0; j < MAX_DS1820_SENSORS; j++)
				{
					if (tempSensors[j].oneWireId == -1)
					{
						tempSensors[j].isActive = true;
						tempSensors[j].oneWireId = i;
						copySensorAddress(addr, tempSensors[j].address);
						saveDS18B20SensorsData();
						break;
					}
				}
			}
		}
	}

	aquaGodState.unsetErrorCode(ERR_TEMPERATURES);
	for (int j = 0; j < MAX_DS1820_SENSORS; j++)
	{
		if (!tempSensors[j].isActive && tempSensors[j].isRequired)
			aquaGodState.setErrorCode(ERR_TEMPERATURES);
	}
}

void startTemperatureMeasurements(bool reinitIfError)
{
	//if (reinitIfError && ((aquaGodState.getErrorCode() & ERR_TEMPERATURES) != 0))
	//	initTempSensors();

	//if ((aquaGodState.getErrorCode() & ERR_TEMPERATURES) != 0)
	//	return;

	ds18b20Sensors.requestTemperatures();
}

// Returns temperature multiplied by 100
int getTemperatureById(int id)
{
	int T = 9900;
	if (tempSensors[id].isActive)
	{
		//T = ds18b20Sensors.getTempCByIndex(id) * 100;
		T = ds18b20Sensors.getTempC(tempSensors[id].address) * 100;
		if (T < 0)
			T = 9800;

		if (T >= 9000)
			aquaGodState.setErrorCode(ERR_TEMPERATURES);
	}
	return T;
}

int getTemperatureByOneWireId(int id)
{
	int T = ds18b20Sensors.getTempCByIndex(id) * 100;
	if (T < 0)
		T = 9800;
	return T;
}

void readTemperatures()
{
	aquaGodState.setAquariumTemperature1(getTemperatureById(TEMP_SENSOR_AQUARIUM_1));
	aquaGodState.setAquariumTemperature2(getTemperatureById(TEMP_SENSOR_AQUARIUM_2));
	aquaGodState.setAquariumTemperature3(getTemperatureById(TEMP_SENSOR_AQUARIUM_3));
	aquaGodState.setSumpTemperature(getTemperatureById(TEMP_SENSOR_SUMP));
	aquaGodState.setHospitalTemperature(getTemperatureById(TEMP_SENSOR_HOSPITAL));
}

void readRoomState()
{
	aquaGodState.setRoomTemperature(dht.getTemperature());
	aquaGodState.setRoomHumidity(dht.getHumidity());
	aquaGodState.setBoardTemperature(getBoardTemperature());
}

// Returns temperature multiplied by 100
int getBoardTemperature(void)
{
	//temp registers (11h-12h) get updated automatically every 64s
	Wire.beginTransmission(DS3231_I2C_ADDRESS);
	Wire.write(0x11);
	Wire.endTransmission();
	Wire.requestFrom(DS3231_I2C_ADDRESS, 2);

	if (Wire.available()) // 1st byte
	{
		byte Th = Wire.read(); // receive a byte
		Wire.available(); // 2nd byte
		byte Tl = Wire.read(); // receive a byte

		byte sign = Th & 0x80;
		int raw = ((Th << 8) | Tl) >> 6;
		if (sign) // negative
			raw = (raw ^ 0xFF) + 1; // 2's complement the answer
		int T = (((Th << 8) | Tl) >> 6) * 25; // 100 / 4
		if (sign)
			T = -T;
		return T;
	}
	else
		return 9999;
}

//TODO
// Returns board 2 temperature multiplied by 100
//int getBoardTemperature2(void)
//{
//	unsigned long value = analogRead(PIN_BOARD2_TEMP);
//	delay(1);
//	value = 0;
//	for (byte i = 0; i < 10; i++)
//	{
//		value = value + analogRead(PIN_BOARD2_TEMP);
//		delay(1);
//	}
//
//	return (5000ul * value / 1023ul);
//}
//

