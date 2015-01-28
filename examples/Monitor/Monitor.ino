
#include <CPPM.h>

void cppm_cycle(void)
{
	if (CPPM.synchronized())
	{
		int aile = (CPPM.read(CPPM.AILE) - 1520*2) / 8 * 125 / 128; // aile -100% .. +100%
		int elev = (CPPM.read(CPPM.ELEV) - 1520*2) / 8 * 125 / 128; // elevator -100% .. +100%
		int thro = (CPPM.read(CPPM.THRO) - 1520*2) / 8 * 125 / 128; // aile -100% .. +100%
		int rudd = (CPPM.read(CPPM.RUDD) - 1520*2) / 8 * 125 / 128; // rudder -100% .. +100%
		int gear = (CPPM.read(CPPM.GEAR) - 1520*2) / 8 * 125 / 128; // aile -100% .. +100%

		Serial.print(aile); Serial.print(", ");
		Serial.print(elev); Serial.print(", ");
		Serial.print(thro); Serial.print(", ");
		Serial.print(rudd); Serial.print(", ");
		Serial.print(gear); Serial.print("\n");
		Serial.flush();
	}
	else
	{
		// if not synchronized, do something...
	}
}

void setup()
{
	Serial.begin(9600);

	CPPM.begin();
}

void loop()
{
	cppm_cycle();

	delay(100);
}
