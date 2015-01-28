# CPPM Library for Arduino
This library provides an interface for reading a CPPM signal delivered by a RC receiver like Orange R615X.
Commonly, the 22ms frame limit the maximum number of servos/channels to 9.

Example:

    #include <CPPM.h>
    
    // your code...
    
    void setup(void)
    {
      CPPM.begin();
    }

    void loop(void)
    {
      if (CPPM.synchronized())
      {
        int throttle = CPPM.servo(CPPM.THRO);
        
        // do something funny...
      }
    }

This library works only on Atmega328 implementation of Arduino UNO, Duemillanove and similar. It uses the Timer1, so PWM on pin 8 and 9 are unavailable.

This implementation differs from https://github.com/claymation/CPPM in one significant point: the synchronization of each pulses is done on the rising edge of the CPPM signal. 

Implementation has been extensively tested on a Orange R615X (6 channels) receiver and with a Spektrum DX8 (DSM2/DSMX) transmitter. Others DX6i DSM2 or newer DSMX/DSM" work similary.

The main page is http://jean-marc.paratte.ch/articles/arduino-cppm/
