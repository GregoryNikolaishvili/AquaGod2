Settings settings =
{
	SETTINGS_VERSION,
	LOG_LEVEL_VERBOSE,

	3000, // Temp Aq
	700,  // PH

	0, // Temp Hospital
	3100, // Board max temp TODO

	AlarmHM_Minutes(22, 0), // O2
	AlarmHM_Minutes(11, 0),
	AlarmHM_Minutes(10, 0), // Co2
	AlarmHM_Minutes(21, 0),

	AlarmHM_Minutes(11, 0), // Light 1
	AlarmHM_Minutes(21, 45),
	AlarmHM_Minutes(11, 05), // Light 2
	AlarmHM_Minutes(21, 50),
	AlarmHM_Minutes(11, 10), // Light 3
	AlarmHM_Minutes(21, 55),
	AlarmHM_Minutes(11, 15), // Light 4
	AlarmHM_Minutes(22, 0),
	AlarmHM_Minutes(11, 20), // Light 5
	AlarmHM_Minutes(22, 5),
	AlarmHM_Minutes(22, 0), // Moon Light
	AlarmHM_Minutes(23, 0),

	AlarmHM_Minutes(10, 30), // Feed 1
	AlarmHM_Minutes(15, 0), // Feed 2
	AlarmHM_Minutes(20, 0),  // Feed 3
	8, // feed duration, sec
	B01111110, // all except sunday

	AlarmHM_Minutes(22, 15), // Water Change 1
	AlarmHM_Minutes(0, 0), // Water Change 2
	60 * 3, // Seconds drain
	60 * 3, // Seconds fill
	60 * 1, // Seconds drain 2
	80, // % level after water drain
	B11111111,

	AlarmHM_Minutes(10, 45), // Dosing macro
	15, // Seconds
	B11111111,

	AlarmHM_Minutes(10, 55), // Dosing micro
	15, // Seconds
	B11111111
};

const char* SettingNames[SETTINGS_COUNT] = 
{
	"Version",
	"Log Level",

	"Aquarium Temperature",
	"Aquarium PH",

	"Hospital Temperature",
	"Board Temperature",

	"O2 On Time",
	"O2 Off Time",
	"CO2 On Time",
	"CO2 Off Time",

	"Light 1 On Time",
	"Light 1 Off Time",
	"Light 2 On Time",
	"Light 2 Off Time",
	"Light 3 On Time",
	"Light 3 Off Time",
	"Light 4 On Time",
	"Light 4 Off Time",
	"Light 5 On Time",
	"Light 5 Off Time",
	"Moon Light On Time",
	"Moon Light Off Time",

	"Feed Start Time A",
	"Feed Start Time B",
	"Feed Start Time C",
	"Feed Duration (sec)",
	"Feed DOW",

	"Water Change Start Time A",
	"Water Change Start Time B",
	"Water Drain Duration (sec)",
	"Water Fill Duration (sec)",
	"Water Exra Drain Duration (sec)",
	"Water Drain Level (%)",
	"Water Change DOW",

	"Dosing Macro Start Time",
	"Dosing Macro Duration (sec)",
	"Dosing Macro DOW",

	"Dosing Micro Start Time",
	"Dosing Micro Duration (sec)",
	"Dosing Micro DOW"
};


void readSettings()
{
	eeprom_read_block(&settings, STORAGE_ADDRESS_SETTINGS, sizeof(settings.version));
	if (settings.version != SETTINGS_VERSION)
		saveSettings();
	else
		eeprom_read_block(&settings, STORAGE_ADDRESS_SETTINGS, sizeof(settings));

	Log.SetLogLevel(settings.logLevel);
}

void saveSettings()
{
	settings.version = SETTINGS_VERSION;
	eeprom_write_block(&settings, STORAGE_ADDRESS_SETTINGS, sizeof(settings));
	Log.Info(F("New settings applied"));
}
