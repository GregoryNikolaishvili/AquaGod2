class AquaGodState
{
private:
	int aquariumTemperature1;
	int aquariumTemperature2;
	int aquariumTemperature3;
	int aquariumPH;
	int sumpWaterLevel; // % 255 = Not valid
	int sumpWaterLevelMm; // 2550 = Not valid

	int sumpTemperature;
	int sumpPH;

	int aquariumWaterLevel; // % 255 = Not valid
	int aquariumWaterLevelMm; // 2550 = Not valid

	int hospitalTemperature;

	int roomTemperature;
	int roomHumidity;

	int boardTemperature;
	unsigned int boardFanRPM;

	bool waterLevelIsCriticallyHighInAquarium;
	bool waterLevelIsCriticallyLowInSump;
	bool waterLevelIsCriticallyHighInSump;
	bool waterLevelIsLowInHospital;
	bool waterIsOnFloor1;
	bool waterIsOnFloor2;

	unsigned int errorCode;
	unsigned long startTime;

	bool lcdIsPresent;

private:
	bool eventsEnabled;
	bool phContinousModeEnabled;

	void fireStateEvent(int id, long value);
public:

	AquaGodState();

	inline void setEventsEnabled(bool value) { eventsEnabled = value; };
	inline void setphContinousModeEnabled(bool value) { phContinousModeEnabled = value; };

	void setAquariumTemperature1(int value);
	void setAquariumTemperature2(int value);
	void setAquariumTemperature3(int value);
	void setAquariumPH(int value);
	void setAquariumWaterLevel(int value);
	void setAquariumWaterLevelMm(int value);

	void setSumpTemperature(int value);
	void setSumpPH(int value);
	void setSumpWaterLevel(int value);
	void setSumpWaterLevelMm(int value);

	void setHospitalTemperature(int value);
	void setRoomTemperature(int value);
	void setRoomHumidity(int value);
	void setBoardTemperature(int value);
	void setBoardFanRPM(unsigned int value);
	void setWaterLevelIsCriticallyHighInAquarium(bool value);
	void setWaterLevelIsCriticallyLowInSump(bool value);
	void setWaterLevelIsCriticallyHighInSump(bool value);
	void setWaterLevelIsLowInHospital(bool value);
	void setWaterIsOnFloor1(bool value);
	void setWaterIsOnFloor2(bool value);
	void setErrorCode(unsigned int value);
	void unsetErrorCode(unsigned int value);
	void setStartTime(unsigned long value);
	void setLcdIsPresent(bool value);

	int getAquariumTemperatureMean() {
		if ((aquariumTemperature1 < 9000) && (aquariumTemperature2 < 9000))
			return (aquariumTemperature1 + aquariumTemperature2) / 2;
		if (aquariumTemperature2 < 9000)
			return aquariumTemperature2;
		else
			return aquariumTemperature1; 
	}

	inline int getAquariumTemperature1() { return aquariumTemperature1; }
	inline int getAquariumTemperature2() { return aquariumTemperature2; }
	inline int getAquariumTemperature3() { return aquariumTemperature3; }
	inline int getAquariumPH() { return aquariumPH; }
	inline int getAquariumWaterLevel()  { return aquariumWaterLevel; }
	inline int getAquariumWaterLevelMm()  { return aquariumWaterLevelMm; }

	inline int getSumpTemperature()  { return sumpTemperature; }
	inline int getSumpPH()  { return sumpPH; }
	inline int getSumpWaterLevel()  { return sumpWaterLevel; }
	inline int getSumpWaterLevelMm()  { return sumpWaterLevelMm; }

	inline int getHospitalTemperature()  { return hospitalTemperature; }

	inline int getRoomTemperature()  { return roomTemperature; }
	inline int getRoomHumidity()  { return roomHumidity; }

	inline int getBoardTemperature() { return boardTemperature; }
	inline unsigned int getBoardFanRPM()  { return boardFanRPM; }

	inline bool getWaterLevelIsCriticallyHighInAquarium() { return waterLevelIsCriticallyHighInAquarium; }
	inline bool getWaterLevelIsCriticallyHighInSump() { return waterLevelIsCriticallyHighInSump; }
	inline bool getWaterLevelIsCriticallyLowInSump() { return waterLevelIsCriticallyLowInSump; }
	inline bool getWaterLevelIsLowInHospital() { return waterLevelIsLowInHospital; }

	inline bool getWaterIsOnFloor1()	{ return waterIsOnFloor1; }
	inline bool getWaterIsOnFloor2()	{ return waterIsOnFloor2; }
	inline bool getWaterIsOnFloor()	{ return waterIsOnFloor1 || waterIsOnFloor2; }

	inline unsigned int getErrorCode() { return errorCode; }

	inline unsigned long getStartTime() { return startTime; }
	inline bool getLcdIsPresent() { return lcdIsPresent; }
};


