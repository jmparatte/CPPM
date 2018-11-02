
#include <CPPM.h>

void cppm_cycle(void)
{
	if (CPPM.synchronized())
	{
        uint32_t previous = micros();
//      // good for DX8-R615X
//      int aile = (CPPM.read(CPPM_AILE) - 1500*2) / 8 * 125 / 128; // aile -100% .. +100%
//      int elev = (CPPM.read(CPPM_ELEV) - 1500*2) / 8 * 125 / 128; // elevator -100% .. +100%
//      int thro = (CPPM.read(CPPM_THRO) - 1500*2) / 8 * 125 / 128; // throttle -100% .. +100%
//      int rudd = (CPPM.read(CPPM_RUDD) - 1500*2) / 8 * 125 / 128; // rudder -100% .. +100%
//      int gear = (CPPM.read(CPPM_GEAR) - 1500*2) / 8 * 125 / 128; // gear -100% .. +100%
//      int aux1 = (CPPM.read(CPPM_AUX1) - 1500*2) / 8 * 125 / 128; // flap -100% .. +100%

        int aile = CPPM.read_us(CPPM_AILE) - 1500; // aile
        int elev = CPPM.read_us(CPPM_ELEV) - 1500; // elevator
        int thro = CPPM.read_us(CPPM_THRO) - 1500; // throttle
        int rudd = CPPM.read_us(CPPM_RUDD) - 1500; // rudder
        int gear = CPPM.read_us(CPPM_GEAR) - 1500; // gear
        int aux1 = CPPM.read_us(CPPM_AUX1) - 1500; // flap
        int aux2 = CPPM.read_us(CPPM_AUX2) - 1500; // flap
        int aux3 = CPPM.read_us(CPPM_AUX3) - 1500; // flap

        uint32_t now = micros();
        Serial.print("Cycle time: " + (String)(now - previous) + "\t| ");

        Serial.print(aile); Serial.print(",\t");
        Serial.print(elev); Serial.print(",\t");
        Serial.print(thro); Serial.print(",\t");
        Serial.print(rudd); Serial.print(",\t");
        Serial.print(gear); Serial.print(",\t");
        Serial.print(aux1); Serial.print(",\t");
        Serial.print(aux2); Serial.print(",\t");
        Serial.print(aux3); Serial.print("\n");
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
