//#define PH2_I2C_ADDRESS 0x4D // MCP3221 A5 I2C address
//#define PH2_VREF 4.98 //Our vRef into the ADC wont be exactly 5v best to measure and adjust here

//pH (PIN_PH1_SERIAL_RX, PIN_PH1_SERIAL_TX);
//SERIAL1 on mega 2560
//pH (PIN_PH2_SERIAL_RX, PIN_PH2_SERIAL_TX);
//SERIAL2 on mega 2560
//Atlas PH stamp (Serial1)



String ph1ensorStrings[2];

void initPH()
{
	ph1ensorStrings[0] = "";
	ph1ensorStrings[1] = "";

	// Atlas PH stamp
	Serial1.begin(38400); // PH EZO is on serial 1
	Serial2.begin(38400); // PH 5.0 is on serial 2

	//if the Arduino just booted up, we need to set some things up first. take the pH Circuit out of continues mode. 
	//on start up sometimes the first command is missed, so, lets send it twice.
	delay(100);
	PH1_StopReading();
	PH2_StopReading();
	delay(100);
	PH1_StopReading();
	PH2_StopReading();
}

//SerialEvent occurs whenever a new data comes in the hardware serial RX.  This routine is run between each time loop() runs, so using delay inside loop can delay response.  
//Multiple bytes of data may be available.

//if the hardware serial port receives a char, get the char we just received add it to the inputString. if the incoming character is a <CR>, set the flag 
void serialEvent1()
{
	char inchar = (char)Serial1.read();
	int ph = processPhCircuitResponse(inchar, 0);
	if (ph > 0)
		aquaGodState.setAquariumPH(ph);
}

void serialEvent2()
{
	char inchar = (char)Serial2.read();
	int ph = processPhCircuitResponse(inchar, 1);
	if (ph > 0)
		aquaGodState.setSumpPH(ph);
}


int processPhCircuitResponse(char inchar, int circuitId)
{
	int ph = -1;

	if (inchar == '\r')
	{
		char floatbuf[24]; // make this at least big enough for the whole string
		ph1ensorStrings[circuitId].toCharArray(floatbuf, sizeof(floatbuf));

		if ((ph1ensorStrings[circuitId] == "check probe") /*old circuit 5.0*/ || ph1ensorStrings[circuitId].startsWith("*")) /*new circuit EZO*/
			Log.Error(F("PH #%d: %s"), circuitId + 1, floatbuf);
		else
		{
			ph = atof(floatbuf) * 100;
			if (ph == 0) // not a valid PH
				Log.Info(F("PH #%d: %s"), circuitId + 1, floatbuf);
		}

		ph1ensorStrings[circuitId] = ""; // Clear recieved buffer
	}
	else
	{
		ph1ensorStrings[circuitId] += inchar;
	}
	return ph;
}

void startPhMeasurements()
{
	Serial1.print("R\r"); //command to take a single reading.
	Serial2.print("R\r"); //command to take a single reading.
}

void PH1_GetInfo()
{
	Serial1.print("I\r");
}

void PH2_GetInfo()
{
	Serial2.print("I\r");
}


//stop readings
void PH1_StopReading()
{
	Serial1.print("C,0\r");
}

void PH2_StopReading()
{
	Serial2.print("e\r");
}


//calibrate to a pH of 7. 
void PH1_Calibrate_7()
{
	Serial1.print("Cal,mid,7.00\r");
}

void PH2_Calibrate_7()
{
	Serial2.print("s\r");
}

//calibrate to a pH of 4
void PH1_Calibrate_4()
{
	Serial1.print("Cal,low,4.00\r");
}

void PH2_Calibrate_4()
{
	Serial2.print("f\r");
}

//calibrate to a pH of 10
void PH1_Calibrate_10()
{
	Serial1.print("Cal,high,10.00\r");
}

void PH2_Calibrate_10()
{
	Serial2.print("t\r");
}

void PH1_FactoryDefault()
{
	Serial1.print("X\r");
}

void PH2_FactoryDefault()
{
	Serial2.print("X\r");
}

void PH1_ReadInfo()
{
	Serial1.print("I\r");
}

void PH2_ReadInfo()
{
	Serial2.print("I\r");
}

void PH1_SetLEDs(boolean enabled)
{
	if (enabled)
		Serial1.print("L,1\r");
	else
		Serial1.print("L,0\r");
}

void PH2_SetLEDs(boolean enabled)
{
	if (enabled)
		Serial2.print("L1\r");
	else
		Serial2.print("L0\r");
}













