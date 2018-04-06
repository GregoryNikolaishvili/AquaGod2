#include <arduino.h>
#include <DallasTemperature.h>

//#define DEBUGGING //TODO

#define REQUIRESALARMS false // FOR DS18B20 library

#define PROCESS_INTERVAL_SEC 10 // how frequentrly data will be checked and processed

#define DS3231_I2C_ADDRESS 0x68

#define MAX_DS1820_SENSORS 8


// Error Code Bits
const int ERR_TIME_NOT_SET = 0x01;
const int ERR_TEMPERATURES = 0x02;
const int ERR_INTERNET = 0x04;
const int ERR_SYSTEM = 0x80;
const int ERR_ULTRASONIC = 0x80;

// For TimeAlarms Class
#define USE_SPECIALIST_METHODS
#define AlarmHM_Minutes(_hr_, _min_) (_hr_ * 60 + _min_)

enum water_change_state
{
	WATER_CHANGE_STATE_NONE = 0,
	WATER_CHANGE_STATE_DRAIN = 1,
	WATER_CHANGE_STATE_FILL = 2,
	WATER_CHANGE_STATE_DRAIN2 = 3
};

//* DEVICES

enum device_states
{
	DEVICE_OFF = 0,
	DEVICE_ON = 1,
	DEVICE_FREE = 99
};

const int LIGHT_1 = 0;
const int LIGHT_2 = 1;
const int LIGHT_3 = 2;
const int LIGHT_4 = 3;
const int LIGHT_5 = 4;
const int EXHAUST_FAN = 5;
const int FEEDER_1 = 6;
const int FEEDER_2 = 7;

// 2nd board
const int O2 = 8;
const int CO2 = 9;
const int CO2_PUMP = 10;
const int WATER_DRAIN_PUMP = 11;
const int DEVICE_220_A = 12;
const int DEVICE_220_B = 13;
const int MOON_LIGHT = 14;
const int BOARD_2_RELAY_FREE = 15;

// 3rd board
const int WATER_FILL_PUMP = 16;
const int DOSING_PUMP_MACRO = 17; // BOARD_3_RELAY_4
const int DOSING_PUMP_MICRO = 18; // SUMP_O2
const int SUMP_RECIRCULATE_PUMP = 19;
const int UV_LIGHT = 20;
const int FILTER_2 = 21;
const int FILTER_1 = 22;
const int HOSPITAL_LIGHT = 23;

// SSR
const int HEATER = 24;
const int SUMP_HEATER = 25;
const int HOSPITAL_HEATER = 26;

const int SOLENOID = 27;
const int BOARD_FAN = 28;
const int MAINTENANCE_MODE = 29;
/////////////////

#define PIN_BOARD_FAN_TACH 2

//#define PIN_SD_CARD 4	// Fixed by shield?
#define PIN_SOLENOID_IN1 5
#define PIN_SOLENOID_IN2 6
#define PIN_HEATER_1 7
#define PIN_HEATER_2 8
#define PIN_DHT21 9
#define PIN_ETHERNET 10	// Fixed by shield
#define PIN_BLINKING_LED LED_BUILTIN // 13


// Not all pins on the Mega and Mega 2560 support change interrupts, so only the following can be used for RX: 10, 11, 12, 13, 14, 15, 50, 51, 52, 53, A8 (62), A9 (63), A10 (64), A11 (65), A12 (66), A13 (67), A14 (68), A15 (69)
// The Arduino Mega has three additional serial ports: Serial1 on pins 19 (RX) and 18 (TX), Serial2 on pins 17 (RX) and 16 (TX), Serial3 on pins 15 (RX) and 14 (TX).
#define PIN_ULTRASONIC_SUMP_TX 14
#define PIN_ULTRASONIC_SUMP_RX 15
#define PIN_PH2_SERIAL_TX 16
#define PIN_PH2_SERIAL_RX 17
#define PIN_PH1_SERIAL_TX 18
#define PIN_PH1_SERIAL_RX 19
// PIN_20 SDA
// PIN_21 SCL


#define PIN_LIGHT_1 22
#define PIN_LIGHT_2 23
#define PIN_LIGHT_3 24
#define PIN_LIGHT_4 25
#define PIN_LIGHT_5 26
#define PIN_EXHAUST_FAN 27
#define PIN_FEEDER_1 28
#define PIN_FEEDER_2 29

#define PIN_O2 30
#define PIN_BOARD2_RELAY_FREE 31
#define PIN_CO2 32
#define PIN_MOON_LIGHT 33
#define PIN_WATER_DRAIN_PUMP 34
#define PIN_220B 35
#define PIN_CO2_PUMP 36
#define PIN_220A 37

#define PIN_WATER_FILL_PUMP 38
#define PIN_DOSING_PUMP_MACRO 39 // PIN_BOARD3_RELAY4
#define PIN_DOSING_PUMP_MICRO 40 //PIN_SUMP_O2 
#define PIN_SUMP_RECIRCULATE_PUMP 41
#define PIN_UV_LIGHT 42
#define PIN_CANISTER_FILTER2 43
#define PIN_CANISTER_FILTER1 44
#define PIN_HOSPITAL_LIGHT 45

#define PIN_BOARD_FAN_PWM 46
#define PIN_ULTRASONIC_AQUARIUM_TX 47
#define PIN_ONE_WIRE_BUS 48 // 1Wire
#define PIN_ULTRASONIC_AQUARIUM_RX 49

#define PIN_WATER_LEVEL_SUMP_LOW A8
#define PIN_WATER_LEVEL_SUMP_HIGH A9
#define PIN_SUMP_HEATER	A10
#define PIN_WATER_IS_ON_FLOOR1 A11
#define PIN_WATER_IS_ON_FLOOR2 A12
#define PIN_HOSPITAL_HEATER A13

#define PIN_WATER_LEVEL_AQUARIUM_HIGH A14
#define PIN_WATER_LEVEL_HOSPITAL_LOW A15


/////////////////////////////////

//#define DEVICE_PH_CAL_4 21 //TODO
//#define DEVICE_PH_CAL_7 22 //TODO


//* SETTINGS

#define SETTINGS_VERSION 0x2324
#define STORAGE_ADDRESS_SETTINGS (uint8_t *)100

#define DS18B20_ADDRESS_SETTINGS_VERSION 0x2323
#define STORAGE_ADDRESS_DS18B20_ADDRESS_SETTINGS (uint8_t *)700

enum lcd_info_mode { LCD_DEF_MODE, LCD_STATE_MODE1, LCD_STATE_MODE2 };

typedef struct
{
	int version;
	int logLevel;

	int aquariumTemperature;
	int aquariumPH;
	int hospitalTemperature;
	int boardTempMax;

	int o2OnTime;
	int o2OffTime;
	int co2OnTime;
	int co2OffTime;

	int light1OnTime;
	int light1OffTime;
	int light2OnTime;
	int light2OffTime;
	int light3OnTime;
	int light3OffTime;
	int light4OnTime;
	int light4OffTime;
	int light5OnTime;
	int light5OffTime;
	int moonLightOnTime;
	int moonLightOffTime;

	int feed1StartTime;
	int feed2StartTime;
	int feed3StartTime;
	int feedDuration;
	int feedDOW;

	int waterChange1StartTime;
	int waterChange2StartTime;
	int waterDrainDuration;
	int waterFillDuration;
	int waterDrainDuration2;
	int waterDrainLevel;
	int waterChangeDOW;

	int dosingMacroStartTime;
	int dosingMacroDuration;
	int dosingMacroDOW;

	int dosingMicroStartTime;
	int dosingMicroDuration;
	int dosingMicroDOW;
}  __attribute__((__packed__)) Settings;

const int SETTINGS_COUNT = sizeof(Settings) / sizeof(int);


typedef struct
{
	DeviceAddress address;
	bool isActive;
	bool isRequired;
	int oneWireId;
}  __attribute__((__packed__)) DS18B20Sensor;

typedef DS18B20Sensor DS18B20Sensors[MAX_DS1820_SENSORS];

// DS18B20 Temp. Sensor roles

const int TEMP_SENSOR_AQUARIUM_1 = 0;
const int TEMP_SENSOR_AQUARIUM_2 = 1;
const int TEMP_SENSOR_AQUARIUM_3 = 2;

const int TEMP_SENSOR_SUMP = 3;
const int TEMP_SENSOR_HOSPITAL = 4;
const int TEMP_SENSOR_ROOM = 5;
const int TEMP_SENSOR_RESERVE_1 = 6;
const int TEMP_SENSOR_RESERVE_2 = 7;

class Logger : public Print
{
public:
	Logger() { }

	size_t write(uint8_t c);
};
