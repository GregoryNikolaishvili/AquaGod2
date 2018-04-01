
NewPing sumpSonar(PIN_ULTRASONIC_SUMP_TX, PIN_ULTRASONIC_SUMP_RX, 100); // NewPing setup of pins and maximum distance.

NewPing aquariumSonar(PIN_ULTRASONIC_AQUARIUM_TX, PIN_ULTRASONIC_AQUARIUM_RX, 100); // NewPing setup of pins and maximum distance.

const int SUMP_FULL_DISTANCE = 110; // mm
const int SUMP_EMPTY_DISTANCE = 350; // mm

const int AQUARIUM_FULL_DISTANCE = 110; // mm
const int AQUARIUM_EMPTY_DISTANCE = 550; // mm

void processUltrasonicSensors()
{
	int distance = aquariumSonar.ping_mm();
	if (distance == 0)
	{
		aquaGodState.setAquariumWaterLevelMm(2550);
		aquaGodState.setAquariumWaterLevel(255);
	}
	else
	{
		aquaGodState.setAquariumWaterLevelMm(distance);
		if (distance > AQUARIUM_EMPTY_DISTANCE)
			aquaGodState.setAquariumWaterLevel(0);
		else
			aquaGodState.setAquariumWaterLevel(100 - 100 * (distance - AQUARIUM_FULL_DISTANCE) / (AQUARIUM_EMPTY_DISTANCE - AQUARIUM_FULL_DISTANCE));
	}

	distance = sumpSonar.ping_mm();
	if (distance == 0)
	{
		aquaGodState.setSumpWaterLevelMm(2550);
		aquaGodState.setSumpWaterLevel(255);
	}
	else
	{
		aquaGodState.setSumpWaterLevelMm(distance);
		if (distance > SUMP_EMPTY_DISTANCE)
			aquaGodState.setSumpWaterLevel(0);
		else
			aquaGodState.setSumpWaterLevel(100 - 100 * (distance - SUMP_FULL_DISTANCE) / (SUMP_EMPTY_DISTANCE - SUMP_FULL_DISTANCE));
		aquaGodState.unsetErrorCode(ERR_ULTRASONIC);
	}

	if ((aquaGodState.getAquariumWaterLevel() >= 255) || (aquaGodState.getSumpWaterLevel() >= 255))
		aquaGodState.setErrorCode(ERR_ULTRASONIC);
	else
		aquaGodState.unsetErrorCode(ERR_ULTRASONIC);
}














