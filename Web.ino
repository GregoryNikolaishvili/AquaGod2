byte LocalMac[] = { 0x00, 0x1D, 0x60, 0xE0, 0x2A, 0x31 };
IPAddress LocalIp(192, 168, 2, 9);

WebServer webserver("", 80);

void defaultCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
	if(!tail_complete) 
	{
		server.httpServerError();
		return;
	}

	/*  if (!server.checkCredentials("Z2lhOmdpYQ==")) // gia:gia
	{
	server.httpUnauthorized();
	return;
	}*/

	/* we don't output the body for a HEAD request */
	if (type == WebServer::GET)
		outputWebInfo(server);
	else
		server.httpNoContent();
}

// Get from 192.168.2.9/state
void stateCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
	if(!tail_complete) 
	{
		server.httpServerError();
		return;
	}

	if (type == WebServer::GET)
		outputJsonInfoHuman(server);
	else
		server.httpNoContent();
}

// Get from 192.168.2.9/istate
void istateCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
	if(!tail_complete) 
	{
		server.httpServerError();
		return;
	}

	if (type == WebServer::GET)
		outputJsonInfo(server);
	else
		server.httpNoContent();
}

// Post to 192.168.2.9/time
void timeCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
	if(!tail_complete) 
	{
		server.httpServerError();
		return;
	}

	if (type == WebServer::POST)
	{
		server.httpSuccess();

		bool repeat;
		char name[16], value[16];
		do
		{
			/* readPOSTparam returns false when there are no more parameters to read from the input. We pass in buffers for it to store the name and value strings along with the length of those buffers. */
			repeat = server.readPOSTparam(name, 16, value, 16);

			if (strcmp(name, "TIME") == 0)
			{
				time_t tm = strtoul(value, NULL, 10);
				RTC.set(tm);
				setTime(tm);
				Log.Info(F("New date and time set"));

				reInitAquarium();
			}
		} 
		while (repeat);

		// after procesing the POST data, tell the web browser to reload the page using a GET method. 
		//server.httpSeeOther("\\");
	}
	else /* for a GET or HEAD, send the standard "it's all OK headers" */
		server.httpNoContent();
}

// Post to 192.168.2.9/relay
void relayCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
	if(!tail_complete) 
	{
		server.httpServerError();
		return;
	}

	if (type == WebServer::POST)
	{
		server.httpSuccess();

		bool repeat;
		char name[16], value[16];
		do
		{
			/* readPOSTparam returns false when there are no more parameters to read from the input. We pass in buffers for it to store the name and value strings along with the length of those buffers. */
			repeat = server.readPOSTparam(name, 16, value, 16);
			if ((name[0] != 0) && (value[0] != 0))
			{
				byte id = strtol(name, NULL, 10); 
				int state = strtol(value, NULL, 10);
				if (id < DEVICE_COUNT) // id >= 0 always
				{
					deviceSetState(id, state);
				}
			}
			/*else 
			if (id == DEVICE_PH_CAL_4)
			PH2_Calibrate_4();
			else 
			if (id == DEVICE_PH_CAL_7)
			PH2_Calibrate_7();*/
		}
		while (repeat);

		// after procesing the POST data, tell the web browser to reload the page using a GET method. 
		//server.httpSeeOther("\\");
	}
	else /* for a GET or HEAD, send the standard "it's all OK headers" */
		server.httpNoContent();
}

// Post to 192.168.2.9/Settings 
void settingsCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
	if(!tail_complete) 
	{
		server.httpServerError();
		return;
	}

	if (type == WebServer::POST)
	{
		server.httpSuccess();

		bool repeat;
		char name[16], value[16];
		do
		{
			/* readPOSTparam returns false when there are no more parameters to read from the input. We pass in buffers for it to store the name and value strings along with the length of those buffers. */
			repeat = server.readPOSTparam(name, 16, value, 16);

			byte setting_id = strtol(name, NULL, 10); 
			int set_value = strtol(value, NULL, 10);
			if ((setting_id > 0) && (setting_id < SETTINGS_COUNT))
				*((int*)&settings + setting_id) = set_value;
		}
		while (repeat);

		saveSettings();
		reInitAquarium();

		// after procesing the POST data, tell the web browser to reload the page using a GET method. 
		//server.httpSeeOther("\\");
	}
	else /* for a GET or HEAD, send the standard "it's all OK headers" */
		server.httpNoContent();
}

/*
curl "192.168.2.9/fs"
curl "192.168.2.9/fs/datafile.txt"
curl -X DELETE "192.168.2.9/fs/datafile.txt"
*/

/*
void fsAccessCmd(WebServer &server, WebServer::ConnectionType type, char **url_path, char *url_tail, bool tail_complete)
{
	if(!tail_complete) 
	{
		server.httpServerError();
		return;
	}

	boolean isJson = strcmp(url_tail, "json") == 0;

	//Only serve files under the "/fs" 
	if(strncmp(url_path[0], "fs", 3) != 0)
	{
		server.httpNoContent();
		return;
	}

	if ((url_path[1] == 0) || (url_path[1][0] == 0))
	{
		// do an ls
		if (isJson)
		{
			server.httpSuccess("application/json");
			server.print(F("{"));
		}
		else
		{
			server.httpSuccess("text/plain");
			server.println(F("Filename\t\tSize"));
			server.println();
		}

		File dir = SD.open("/", FILE_READ);
		dir.rewindDirectory();

		boolean isFirst = true;
		while(true) 
		{
			File entry = dir.openNextFile();
			if (!entry) 
				break;

			if (isJson)
			{
				if (!isFirst)
					server.print(F(","));
				isFirst = false;

				server.print(F("\""));
				server.print(entry.name());
				server.print(F("\":"));
				server.print(entry.size(), DEC);
			}
			else
			{
				server.print(entry.name());
				server.print(F("\t\t"));
				server.println(entry.size(), DEC);
			}

			entry.close();
		}
		if (isJson)
			server.print(F("}"));

		dir.close();
	}
	else{
		// access a file
		File dataFile = SD.open(url_path[1], FILE_READ);
		if (dataFile) 
		{
			if(type == WebServer::GET)
			{
				server.httpSuccess("text/plain");

				char buf[512];
				int  readed = 1;
				while(readed > 0) 
				{
					readed = dataFile.read(buf, 512);
					webserver.write(buf, readed);
				}

				//while (dataFile.available())
				//  webserver.write(dataFile.read());
				dataFile.close();
			}
			else 
				if(type == WebServer::DELETE)
				{
					dataFile.close();
					server.httpSuccess();
					SD.remove(url_path[1]);
				}
		}
		else
		{
			dataFile.close();
			server.httpNoContent();
		}
	}
}
*/

// Get from 192.168.2.9/pins
void outputPinsCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
	if(!tail_complete) 
	{
		server.httpServerError();
		return;
	}

	P(htmlHead) =
		"<html>"
		"<head>"
		"<title>AquaGod 2 Web Server</title>"
		"<style type=\"text/css\">"
		"BODY { font-family: sans-serif }"
		"H1 { font-size: 14pt; text-decoration: underline }"
		"P  { font-size: 10pt; }"
		"</style>"
		"</head>"
		"<body>";

	byte i;
	server.httpSuccess();
	server.printP(htmlHead);

	server.print("<h1>Digital Pins</h1><p>");

	for (i = 0; i <= 53; ++i)
	{
		// ignore the pins we use to talk to the Ethernet chip
		int val = digitalRead(i);
		server.print("Digital ");
		server.print(i);
		server.print(": ");
		server.print(val ? "HIGH" : "LOW");
		server.print("<br/>");
	}

	server.print("</p><h1>Analog Pins</h1><p>");
	byte j = 0;
	for (i = A0; i <= A15; ++i)
	{
		int val = analogRead(i);
		server.print("Analog A");
		server.print(j);
		server.print(": ");
		server.print(val);
		server.print("<br/>");
		j++;
	}

	server.print("</p>");

	server.print("</body></html>");
}


void initWWW()
{
	// start the Ethernet connection and the server:
	Ethernet.begin(LocalMac, LocalIp);
	webserver.begin();

	/* setup our default command that will be run when the user accesses the root page on the server */
	webserver.setDefaultCommand(&defaultCmd);
	webserver.addCommand("state", &stateCmd);
	webserver.addCommand("istate", &istateCmd);
	webserver.addCommand("relay", &relayCmd);
	webserver.addCommand("time", &timeCmd);
	webserver.addCommand("settings", &settingsCmd);
	webserver.addCommand("pins", &outputPinsCmd);
	//webserver.setUrlPathCommand(&fsAccessCmd);

	if (Ethernet.localIP() == INADDR_NONE)
		aquaGodState.setErrorCode(ERR_INTERNET);

#ifdef DEBUGGING
	Serial.print(F("Server is at: "));
	Serial.println(Ethernet.localIP());
#endif
}

//char webBuff[64];
//int webBufLen = 64;

void processWWW()
{

	/* process incoming connections one at a time forever */
	//webserver.processConnection(webBuff, &webBufLen);
	webserver.processConnection();
}

void writeJson(WebServer &server, long value, int id, boolean is_last) 
{
	server.print(F("\""));
	server.print(id);
	server.print(F("\":"));
	server.print(value);
	if (!is_last)
		server.print(F(","));
}

void writeJsonStr(WebServer &server, long value, const char* id, boolean is_last) 
{
	server.print(F("\""));
	server.print(id);
	server.print(F("\":"));
	server.print(value);
	if (!is_last)
		server.print(F(","));
}

void writeJsonStr(WebServer &server, long value, const __FlashStringHelper* id, boolean is_last) 
{
	server.print(F("\""));
	server.print(id);
	server.print(F("\":"));
	server.print(value);
	if (!is_last)
		server.print(F(","));
}

P(trtd) = "<tr><td>";

P(_td_tr) = "</td></tr>";
P(_tdtd_open) = "</td><td";


void OutputDevice(WebServer &server, int device_id)
{       
	server.printP(trtd);
	server.print(devices[device_id].name);
	server.print(F("</td><td>"));
	server.print(deviceGetState(device_id));
	server.printP(_td_tr);
}

void outputJsonInfoHuman(WebServer &server)
{
	server.httpSuccess("application/json");
	server.print(F("{"));

	server.print(F("\"Settings\": {"));
	int value;
	for (byte setting_id = 0; setting_id < SETTINGS_COUNT; setting_id++)
	{
		value = *((int*)&settings + setting_id);
		writeJsonStr(server, value, SettingNames[setting_id], setting_id == SETTINGS_COUNT - 1);
	}
	server.print(F("},"));

	server.print(F("\"State\": {"));
	for (byte device_id = 0; device_id < DEVICE_COUNT; device_id++)
		writeJsonStr(server, deviceGetState(device_id), devices[device_id].name, false);

	writeJsonStr(server, aquaGodState.getAquariumTemperatureMean(), F("Aquarium Temperature (mean)"), false);
	writeJsonStr(server, aquaGodState.getAquariumTemperature1(), F("Aquarium Temperature 1"), false);
	writeJsonStr(server, aquaGodState.getAquariumTemperature2(), F("Aquarium Temperature 2"), false);
	writeJsonStr(server, aquaGodState.getAquariumTemperature3(), F("Aquarium Temperature 3"), false);
	writeJsonStr(server, aquaGodState.getAquariumPH(), F("Aquarium PH"), false);
	writeJsonStr(server, aquaGodState.getAquariumWaterLevel(), F("Aquarium Water Level (%)"), false);
	writeJsonStr(server, aquaGodState.getAquariumWaterLevelMm(), F("Aquarium Water Level (mm)"), false);

	writeJsonStr(server, aquaGodState.getSumpTemperature(), F("Sump Temperature"), false);
	writeJsonStr(server, aquaGodState.getSumpPH(), F("Sump PH"), false);
	writeJsonStr(server, aquaGodState.getSumpWaterLevel(), F("Sump Water Level (%)"), false);
	writeJsonStr(server, aquaGodState.getSumpWaterLevelMm(), F("Sump Water Level (mm)"), false);
	
	writeJsonStr(server, aquaGodState.getHospitalTemperature(), F("Hospital Temperature"), false);
	writeJsonStr(server, aquaGodState.getRoomTemperature(), F("Room Temperature"), false);
	writeJsonStr(server, aquaGodState.getRoomHumidity(), F("Room Humidity"), false);
	writeJsonStr(server, aquaGodState.getBoardTemperature(), F("Board Temperature"), false);
	writeJsonStr(server, aquaGodState.getBoardFanRPM(), F("Board Fan RPM"), false);

	if (aquaGodState.getWaterLevelIsCriticallyHighInAquarium())
		writeJsonStr(server, 1, F("Water level is critically high in aquarium"), false);
	if (aquaGodState.getWaterLevelIsCriticallyLowInSump())
		writeJsonStr(server, 1, F("Water level is critically low in sump"), false);
	if (aquaGodState.getWaterLevelIsCriticallyHighInSump())
		writeJsonStr(server, 1, F("Water level is critically high in sump"), false);
	if (aquaGodState.getWaterIsOnFloor1())
		writeJsonStr(server, 1, F("Water is on floor (1)"), false);
	if (aquaGodState.getWaterIsOnFloor2())
		writeJsonStr(server, 1, F("Water is on floor (2)"), false);

	if (aquaGodState.getErrorCode() != 0)
		writeJsonStr(server, aquaGodState.getErrorCode(), F("Error Code"), false);
	writeJsonStr(server, now(), F("Now"), false); 
	writeJsonStr(server, now() - aquaGodState.getStartTime(), F("Board Up Time"), false);
	timeStatus_t tStatus = timeStatus();
	if (tStatus != timeSet)
		writeJsonStr(server, tStatus , F("Time Status"), false);
	writeJsonStr(server, freeMemory(), F("Free Memory"), true);
	server.print(F("},"));

	// Alarms
	server.print(F("\"Events\": {"));
	bool hadAlarms = false;
	for(uint8_t id = 0; id < dtNBR_ALARMS; id++)
	{
		AlarmClass* alarm = Alarm.getAlarm(id);
		if (alarm)
		{
			if (hadAlarms)
				server.print(F(","));
			hadAlarms = true;
			writeJsonStr(server, alarm->nextTrigger, alarm->eventName, true);
		}
	}
	server.print(F("}}"));
}

void outputJsonInfo(WebServer &server)
{
	int value;

	server.httpSuccess("application/json");

	server.print(F("{\"Settings\": {"));
	for (byte setting_id = 0; setting_id < SETTINGS_COUNT; setting_id++)
	{
		value = *((int*)&settings + setting_id);
		writeJson(server, value, setting_id, setting_id == SETTINGS_COUNT - 1);
	}
	server.print(F("},"));

	server.print(F("\"State\": {"));
	for (byte device_id = 0; device_id < DEVICE_COUNT; device_id++)
		writeJson(server, deviceGetState(device_id), device_id, false);

	int id = 102;
	writeJson(server, aquaGodState.getAquariumPH(), id++, false);
	id++;
	writeJson(server, aquaGodState.getSumpPH(), id++, false);
	writeJson(server, aquaGodState.getSumpWaterLevel(), id++, false);
	writeJson(server, aquaGodState.getSumpWaterLevelMm(), id++, false);
	id++;
	writeJson(server, aquaGodState.getRoomTemperature(), id++, false);
	writeJson(server, aquaGodState.getRoomHumidity(), id++, false);
	writeJson(server, aquaGodState.getBoardTemperature(), id++, false);
	writeJson(server, aquaGodState.getBoardFanRPM(), id++, false);
	writeJson(server, aquaGodState.getAquariumWaterLevel(), id++, false);	// 112
	writeJson(server, aquaGodState.getAquariumWaterLevelMm(), id++, false); // 113

	writeJson(server, aquaGodState.getAquariumTemperatureMean(), 399, false); // 399
	id = 400;
	writeJson(server, aquaGodState.getAquariumTemperature1(), id++, false);
	writeJson(server, aquaGodState.getAquariumTemperature2(), id++, false);
	writeJson(server, aquaGodState.getAquariumTemperature3(), id++, false);
	writeJson(server, aquaGodState.getSumpTemperature(), id++, false);
	writeJson(server, aquaGodState.getHospitalTemperature(), id++, false);


	if (aquaGodState.getWaterLevelIsCriticallyHighInAquarium())
		writeJson(server, 1, 200, false);
	if (aquaGodState.getWaterLevelIsCriticallyLowInSump())
		writeJson(server, 1, 201, false);
	if (aquaGodState.getWaterLevelIsCriticallyHighInSump())
		writeJson(server, 1, 202, false);
	if (aquaGodState.getWaterIsOnFloor1())
		writeJson(server, 1, 203, false);
	if (aquaGodState.getWaterIsOnFloor2())
		writeJson(server, 1, 204, false);

	if (aquaGodState.getErrorCode() != 0)
		writeJson(server, aquaGodState.getErrorCode(), 300, false);
	writeJson(server, now(), 301, false);  // Now
	writeJson(server, now() - aquaGodState.getStartTime(), 302, false); // UpTime
	timeStatus_t tStatus = timeStatus();
	if (tStatus != timeSet)
		writeJson(server, tStatus, 303, false);
	writeJson(server, freeMemory(), 304, true);
	server.print(F("}}"));
}

void uptime(unsigned long milli, char* buffer)
{
	//static char _return[32];
	unsigned long secs = milli / 1000, mins = secs / 60;
	unsigned int hours = mins / 60, days = hours / 24;
	milli -= secs * 1000;
	secs -= mins * 60;
	mins -= hours * 60;
	hours -= days * 24;
	sprintf(buffer, "%d days %2.2d:%2.2d:%2.2d", (byte)days, (byte)hours, (byte)mins, (byte)secs);
}

void printX(WebServer &server, const __FlashStringHelper * title, float X, float X0, const __FlashStringHelper * measure)
{
	server.printP(trtd);
	server.print(title);
	server.printP(_tdtd_open);
	if (X0 > 0)
	{
		if (X < (X0 - 0.25))
			server.print(F(" class=\"cold\""));
		else
			if (X > (X0 + 0.25))
				server.print(F(" class=\"hot\""));
	}
	server.print(">");

	server.print(X);
	server.print(measure);

	server.printP(_td_tr);
}

void printT(WebServer &server, const __FlashStringHelper * title, int T, int T0)
{
	printX(server, title, T / 100.0, T0 / 100.0, F(" &deg;C"));
}

void printPH(WebServer &server, const __FlashStringHelper * title, int ph, int ph0)
{
	printX(server, title, ph / 100.0, ph0 / 100.0, F(""));
}

void outputWebInfo(WebServer &server)
{
	P(htmlHead) = "<html><head><title>AquaGod 2</title><style type=\"text/css\">body {width:288px;zoom:250%} button {width: 100px;}.cold {color:blue;}.hot {color:red;}table{border-collapse: collapse;}table td{border-bottom: 1px dotted black;padding: 4px;width: 150px;}</style></head><body><table>"; 
	P(table) = "<table>";
	P(_table) = "</table>";
	//P(separator) = "<tr style=\"height:10px\"><td colspan=\"2\"></td></tr>";

	server.httpSuccess();
	server.printP(htmlHead);

	printT(server, F("Aquarium T"), aquaGodState.getAquariumTemperatureMean(), settings.aquariumTemperature);
	printT(server, F("Aquarium T1"), aquaGodState.getAquariumTemperature1(), settings.aquariumTemperature);
	printT(server, F("Aquarium T2"), aquaGodState.getAquariumTemperature2(), settings.aquariumTemperature);
	printT(server, F("Aquarium T3"), aquaGodState.getAquariumTemperature3(), settings.aquariumTemperature);
	printT(server, F("Sump T"), aquaGodState.getSumpTemperature(), settings.aquariumTemperature);
	printT(server, F("Hostpital T"), aquaGodState.getHospitalTemperature(), settings.hospitalTemperature);

	printPH(server, F("Aquarium PH"), aquaGodState.getAquariumPH(), settings.aquariumPH);
	printPH(server, F("Sump PH"), aquaGodState.getSumpPH(), settings.aquariumPH);

	printX(server, F("Aquarium Level"), aquaGodState.getAquariumWaterLevel(), 0, F(" %"));
	printX(server, F("Sump Level"), aquaGodState.getSumpWaterLevel(), 0, F(" %"));

	printT(server, F("Room T"), aquaGodState.getRoomTemperature(), 0);
	printX(server, F("Room H"), aquaGodState.getRoomHumidity(), 0, F(" %"));
	printT(server, F("Board T"), aquaGodState.getBoardTemperature(), settings.boardTempMax);
	printX(server, F("Board Fan RPM"), aquaGodState.getBoardFanRPM(), 0, F(""));

	server.printP(_table);
	server.print(F("<br/>"));

	server.printP(table);

	OutputDevice(server, HEATER);
	OutputDevice(server, SUMP_HEATER);
	OutputDevice(server, HOSPITAL_HEATER);

	OutputDevice(server, LIGHT_1);
	OutputDevice(server, LIGHT_2);
	OutputDevice(server, LIGHT_3);
	OutputDevice(server, LIGHT_4);
	OutputDevice(server, LIGHT_5);
	OutputDevice(server, MOON_LIGHT);
	OutputDevice(server, EXHAUST_FAN);

	OutputDevice(server, O2);
	OutputDevice(server, CO2);
	OutputDevice(server, CO2_PUMP);
	OutputDevice(server, WATER_DRAIN_PUMP);

	OutputDevice(server, WATER_FILL_PUMP);
	OutputDevice(server, SUMP_RECIRCULATE_PUMP);

	OutputDevice(server, FILTER_1);
	OutputDevice(server, FILTER_2);

	OutputDevice(server, BOARD_FAN);
	OutputDevice(server, SOLENOID);

	OutputDevice(server, DOSING_PUMP_MACRO);
	OutputDevice(server, DOSING_PUMP_MICRO);

	server.printP(_table);

	server.print(F("<br/>"));
	server.printP(table);

	server.print(F("<tr><td>Curent time</td><td>")); 
	printDateTime(server, now());
	server.printP(_td_tr);

	server.print(F("<tr><td>Up time</td><td>"));
	char buffer[32];
	uptime(millis(), buffer);
	server.print(buffer);
	server.printP(_td_tr);

	if (timeStatus() != timeSet)
	{
		server.print(F("<tr><td>Time status</td><td>"));
		switch (timeStatus())
		{
		case 0: 
			server.print(F("Time not set")); 
			break;
		case 1: 
			server.print(F("Time needs sync")); 
			break;
		default: 
			server.print(F("Unknown state")); 
			break;
		}
		server.printP(_td_tr);
	}

	server.print(F("<tr><td>Free memory</td><td>"));
	server.print(freeMemory());
	server.print(F(" Bytes"));
	server.printP(_td_tr);

	if (aquaGodState.getErrorCode() != 0)
	{
		server.print(F("<tr><td>Error code</td><td>"));
		server.print(aquaGodState.getErrorCode());
		server.printP(_td_tr);
	}

	AlarmClass* nextAlarm = Alarm.getNextTriggerAlarm();
	if (nextAlarm)
	{
		server.print(F("<tr><td>Next event</td><td>"));
		printDateTime(server, nextAlarm->nextTrigger);
		server.print(nextAlarm->eventName);
		server.printP(_td_tr);
	}

	server.printP(_table);

	server.print(F("</br><a href=\"https://xively.com/feeds/68067\" target=\"_blank\">Xively Link</a></br>"));
	server.print(F("</body></html>"));
}













