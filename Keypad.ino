#define PCF8574_I2C_ADDRESS 0x20

extern lcd_info_mode lcdInfoMode;

#ifdef USE_NEW_KEYPAD

const byte ROWS = 5; //four rows
const byte COLS = 4; //four columns
const char keys[ROWS][COLS] = {
	{ 'O', 'T', '#', '*' },
	{ '1', '2', '3', 'U' },
	{ '4', '5', '6', 'D' },
	{ '7', '8', '9', '\27' },
	{ 'L', '0', 'R', '\n' }
};

byte rowPins[ROWS] = { 255, 3, 2, 1, 0 }; //connect to the row pinouts of the keypad
byte colPins[COLS] = { 7, 6, 5, 4 }; //connect to the column pinouts of the keypad

Keypad_I2C kpd = Keypad_I2C(makeKeymap(keys), rowPins, colPins, ROWS, COLS, PCF8574_I2C_ADDRESS);

String keypadInput = "";


void initKeypad()
{
	kpd.begin();
	kpd.addEventListener(keypadEvent); //add an event listener for this keypad
	keypadInput = "";
}

//take care of some special events
void keypadEvent(KeypadEvent key){
	switch (kpd.getState()){
	case PRESSED:
		switch (key){
		case '#': break;
		case '*': break;
		}
		break;
	case RELEASED:
		switch (key){
		case '*': break;
		}
		break;
	case HOLD:
		switch (key){
		case '*': break;
		}
		break;
	}
}


// commands
// A, B, C, D - Lights 1-4 ON/OFF
// xx*yy# - Set device #xx state to yy, where xx < 30
// 98*yy# - Set all device state to yy
// 99*yy# - Set all device state to yy

void processKeypad()
{
#ifdef USE_NEW_KEYPAD
	char key = kpd.getKey();
#else
	char key = kpd.get_key();
#endif

	if (key == '\0')
		return;

	Log.Debug(F("Key pressed: %c"), key);

	switch (key)
	{
	case 'A':
		cycleLcdInfoMode();
		break;
	case 'B':
		clearLcdRow(3);
		lcd.print(F("H: 50"));
		deviceSetState(HEATER, 50);
		deviceSetState(SUMP_HEATER, 50);
		deviceSetState(HOSPITAL_HEATER, 50);
		break;
	case 'C':
		if (lcdInfoMode == LCD_STATE_MODE1)
			lcdScrollUp();
		break;
	case 'D':
		if (lcdInfoMode == LCD_STATE_MODE1)
			lcdScrollDown();
		break;

	case '#': // Enter
		ProcessKeyPadCode();
		keypadInput = "";
		break;
	case '*':
	default:
		keypadInput = keypadInput + key;
		break;
	}
}

void ProcessKeyPadCode()
{
	int idx = inData.indexOf('*');
	if (idx < 0)
		return;

	String spin = inData.substring(1, idx);
	String sstate = inData.substring(idx + 1);
	byte id = spin.toInt();
	int state = sstate.toInt();

	spin = "";
	sstate = "";

	if (id < DEVICE_COUNT) // id >= 0 always
		deviceSetState(id, state);
	else
	if (id == 98)
	{
		for (byte device_id = 0; device_id < RELAY_COUNT; device_id++)
		{
			deviceSetState(device_id, state);
			delay(1000);
		}
	}
	else
	if (id == 99)
	{
		initDevices();
		reInitAquarium();
	}
}



#else
// With A0, A1 and A2 of PCF8574 to ground I2C address is 0x20

#define ROWS 4
#define COLS 4

//	1  2  3  A		P7
//	4  5  6  B		P6
//	7  8  9  C		P5
//	*  0  #  D		P4
//
//	P3 P2 P1 P0			

// Keypad command code
//xxx*yyy#
i2ckeypad kpd = i2ckeypad(PCF8574_I2C_ADDRESS, ROWS, COLS);

int keyPadInputCode1;
int keyPadInputCode2;
byte keyPadCodePosition;

void initKeypad()
{
#ifdef USE_NEW_KEYPAD
	kpd.begin();
	kpd.addEventListener(keypadEvent); //add an event listener for this keypad
#else
	kpd.init();
#endif
	keyPadCodePosition = 1;
	keyPadInputCode1 = 0;
	keyPadInputCode1 = 0;
}


// commands
// A, B, C, D - Lights 1-4 ON/OFF
// xx*yy# - Set device #xx state to yy, where xx < 30
// 98*yy# - Set all device state to yy
// 99*yy# - Set all device state to yy

void processKeypad()
{
	char key = kpd.get_key();

	if (key == '\0')
		return;

	Log.Debug(F("Key pressed: %c"), key);

	switch (key)
	{
	case 'A':
		cycleLcdInfoMode();
		break;
	case 'B':
		clearLcdRow(3);
		lcd.print(F("H: 50"));
		deviceSetState(HEATER, 50);
		deviceSetState(SUMP_HEATER, 50);
		deviceSetState(HOSPITAL_HEATER, 50);
		break;
	case 'C':
		if (lcdInfoMode == LCD_STATE_MODE1)
			lcdScrollUp();
		break;
	case 'D':
		if (lcdInfoMode == LCD_STATE_MODE1)
			lcdScrollDown();
		break;

	case '*':
		keyPadInputCode2 = 0;
		keyPadCodePosition = 2;
		break;
	case '#':
		ProcessKeyPadCode();
		keyPadInputCode1 = 0;
		keyPadInputCode2 = 0;
		keyPadCodePosition = 1;
		break;
	default:
		if (keyPadCodePosition == 1)
			keyPadInputCode1 = keyPadInputCode1 * 10 + (key - '0');
		else
		if (keyPadCodePosition == 2)
			keyPadInputCode2 = keyPadInputCode2 * 10 + (key - '0');
		break;
	}
}

void ProcessKeyPadCode()
{
	if ((keyPadInputCode1 >= 0) && (keyPadInputCode1 < DEVICE_COUNT))
		deviceSetState(keyPadInputCode1, keyPadInputCode2);
	else if (keyPadInputCode1 == 98)
	{
		switch (keyPadInputCode2)
		{
		case 0:
		case 1:
			for (byte device_id = 0; device_id < RELAY_COUNT; device_id++)
			{
				deviceSetState(device_id, keyPadInputCode2);
				delay(1000);
			}
			break;
		}
	}
	else if (keyPadInputCode1 == 99)
	{
		initDevices();
		reInitAquarium();
	}
}

#endif

