#define LCD_I2C_ADDRESS		0x27  // Define I2C Address where the PCF8574A is
#define LCD_ROWS 4
#define LCD_COLUMNS 20

LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // Set the LCD I2C address


// Create a set of new characters. Only 8 characters could be created
const uint8_t charBitmap[4][8] = {
	{ 0x1C, 0x14, 0x1C, 0x10, 0x15, 0x07, 0x05, 0x00 }, // PH
	//{ 0x0,0xa,0x11,0x15,0x11,0xa,0x0,0x0 }, // Light	0x8,0x10,0x0,0x18,0x0,0x10,0x8,0x0
	//{ 0x00, 0x06, 0x1F, 0x16, 0x10, 0x10, 0x10, 0x10 }, // Sump
	{ 0x01, 0x01, 0x01, 0x01, 0x0D, 0x1F, 0x0C, 0x00 }, // Drain
	{ 0x00, 0x15, 0x0E, 0x11, 0x15, 0x11, 0x0E, 0x15 }, // Fan
	{ 0x00, 0x1F, 0x04, 0x1F, 0x04, 0x1F, 0x04, 0x1F } // Heater
};

#define ICON_PH (char)0

//#define ICON_OK '!'
//#define ICON_ERROR '?'

//#define ICON_LIGHT (char)0
//#define ICON_O2 (char)0
//#define ICON_CO2 (char)0

//#define ICON_DRAIN (char)1
//#define ICON_FAN (char)2
//#define ICON_HEATER (char)3
//
//#define ICON_FILTER 'F'
//#define ICON_UV_LIGHT ICON_LIGHT

lcd_info_mode lcdInfoMode;
byte lcdScrollValueY = 0;

void initLCD()
{
	lcdInfoMode = LCD_DEF_MODE;
	lcd.begin(LCD_COLUMNS, LCD_ROWS, LCD_5x8DOTS); // initialize the lcd 

	for (byte i = 0; i < 4; i++)
		lcd.createChar (i, (uint8_t *)charBitmap[i] );

	lcd.clear(); // go home
	lcd.print("Starting...");  
	//lcd.setBacklight(HIGH); // Backlight on

	aquaGodState.setLcdIsPresent(true);
	delay(500);
}

void lcdPrint(const __FlashStringHelper* message)
{
	clearLcdRow(3);
	lcd.setCursor (0, 3);
	lcd.print(message);
}

void clearLcdRow(int row)
{
	lcd.setCursor (0, row);
	for (byte i = 0; i < LCD_COLUMNS; i++)
		lcd.print(' ');
	lcd.setCursor (0, row);
}

void setDeviceIcon(byte device_id, char icon_on, char icon_off)
{
	if (deviceGetState(device_id) > 0)
		lcd.print(icon_on);
	else
		lcd.print(icon_off);
}

void lcdScrollUp()
{
	if (lcdScrollValueY > 0)
		lcdScrollValueY --;
	showLcdInfo();
}

void lcdScrollDown()
{
	lcdScrollValueY ++;
	if (lcdScrollValueY >= DEVICE_COUNT - 3)
		lcdScrollValueY = 0;
	showLcdInfo();
}

void cycleLcdInfoMode()
{
	if (lcdInfoMode == LCD_DEF_MODE)
		lcdInfoMode  = LCD_STATE_MODE1;
	else
		if (lcdInfoMode == LCD_STATE_MODE1)
			lcdInfoMode  = LCD_STATE_MODE2;
		else
			lcdInfoMode = LCD_DEF_MODE;

	showLcdInfo();
}

void showLcdInfo()
{
	if (!aquaGodState.getLcdIsPresent())
		return;
	lcd.clear();

	switch (lcdInfoMode)
	{
	case LCD_DEF_MODE:
		showLcdInfoDefMode();
		break;
	case LCD_STATE_MODE1:
		showLcdStateMode1();
		break;
	case LCD_STATE_MODE2:
		showLcdStateMode2();
		break;
	default:
		break;
	}
}

void showLcdInfoDefMode()
{
	int X;

	///// ROW 1
	byte row = 0;
	lcd.setCursor (0, row++);

	X = aquaGodState.getAquariumTemperature1();
	lcdPrintDecimal(X);
	lcd.write(0xDF);
	lcd.print(' ');

	X = aquaGodState.getAquariumTemperature2();
	lcdPrintDecimal(X);
	lcd.write(0xDF);
	lcd.print(' ');

	X = aquaGodState.getAquariumPH();
	lcdPrintDecimal(X);
	lcd.print(ICON_PH);

	///// ROW 2
	lcd.setCursor (0, row++);

	X = aquaGodState.getSumpTemperature();
	lcdPrintDecimal(X);
	lcd.write(0xDF);
	lcd.print(' ');

	X = aquaGodState.getHospitalTemperature();
	lcdPrintDecimal(X);
	lcd.write(0xDF);
	lcd.print(' ');

	X = aquaGodState.getSumpPH();
	lcdPrintDecimal(X);
	lcd.print(ICON_PH);

	///// ROW 3
	lcd.setCursor (0, row++);

	X = aquaGodState.getBoardTemperature();
	lcdPrintDecimal(X);
	lcd.write(0xDF);
	lcd.print(' ');

	X = aquaGodState.getRoomTemperature();
	lcdPrintDecimal(X);
	lcd.write(0xDF);
	lcd.print(' ');

	X = aquaGodState.getRoomHumidity();
	lcd.print(X);
	lcd.print('%');

	///// ROW 4
	lcd.setCursor (0, row++);
	lcd.print(F("Heater: "));
	X = deviceGetState(HEATER);
	if (X == 100)
		X = 99;
	else
		if (X < 10)
			lcd.print('0');
	lcd.print(X);
	lcd.print('%');


	// STATUSES
	row = 0;
	if (deviceGetState(CO2_PUMP) == 0)
	{
		lcd.setCursor (19, row);
		lcd.print('!');
	}

	row = 1;
	/*if ((deviceGetState(SUMP_O2) == 0) || (deviceGetState(SUMP_RECIRCULATE_PUMP) == 0))
	{
		lcd.setCursor (19, row);
		lcd.print('!');
	}*/

	row = 2;
	if (deviceGetState(UV_LIGHT) > 0)
	{
		lcd.setCursor (15, row);
		lcd.print(F("UV"));
	}

	if ((deviceGetState(FILTER_1) == 0) || (deviceGetState(FILTER_1) == 0))
	{
		lcd.setCursor (17, row);
		lcd.print('F');
	}

	if (aquaGodState.getErrorCode() != 0)
	{
		lcd.setCursor (18, row);
		lcd.print('!');
	}

	if (deviceGetState(MAINTENANCE_MODE) > 0)
	{
		lcd.setCursor (19, row);
		lcd.print('!');
	}
}

void showLcdStateMode1()
{
	//int X;
	byte row = 0;

	for (byte device_id = lcdScrollValueY; device_id < lcdScrollValueY + 4; device_id++)
	{
		lcd.setCursor (0, row++);

		lcd.print(device_id);
		lcd.print(' ');
		lcd.print(devices[device_id].name);
		lcd.print(F(": "));
		lcd.print(deviceGetState(device_id));
	}
}

void showLcdStateMode2()
{
	int X;

	///// ROW 1
	byte row = 0;
	lcd.setCursor (0, row++);

	X = deviceGetState(BOARD_FAN);
	lcd.print(F("FAN PWM "));
	lcd.print(X);
	lcd.print(' ');

	X = aquaGodState.getBoardFanRPM();
	lcd.print(F("RPM "));
	lcd.print(X);
	lcd.print(' ');

	///// ROW 2
	lcd.setCursor (0, row++);

	X = aquaGodState.getSumpWaterLevel();
	lcd.print(F("SUMP "));
	lcd.print(X);
	lcd.print(F("% "));

	if (aquaGodState.getWaterLevelIsCriticallyHighInSump())
		lcd.print(F("HIGH "));
	if (aquaGodState.getWaterLevelIsCriticallyLowInSump())
		lcd.print(F("LOW "));

	///// ROW 3
	lcd.setCursor (0, row++);

	if (aquaGodState.getWaterIsOnFloor())
		lcd.print(F("WATER ON FLOOR: "));
	if (aquaGodState.getWaterIsOnFloor1())
		lcd.print(F("1 "));
	if (aquaGodState.getWaterIsOnFloor2())
		lcd.print(F("2 "));

	///// ROW 4
	lcd.setCursor (0, row++);

	if (aquaGodState.getWaterLevelIsCriticallyHighInAquarium())
		lcd.print(F("LEVEL HIGH IN AQUA"));
	else
	if (aquaGodState.getWaterLevelIsLowInHospital())
		lcd.print(F("HOSPITAL WATER LOW"));
}

void lcdPrintDecimal(int x)
{
	lcd.print(x / 100);
	lcd.print('.');
	lcd.print((x % 100) / 10);
}












