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
        int throttle = CPPM.read(CPPM.THRO);
        
        // do something funny...
      }
    }

This library works only on ATmega328 implementation of Arduino UNO, Duemillanove, Leonardo and similar. It uses the Timer1. So PWM on pin 9 and 10 are unavailable. The CPPM signal is connected to the Arduino pin 8 named ICP1. In a future revision, pins 8 and 9 could be 2 output CPPM signals.

This implementation differs from https://github.com/claymation/CPPM in one significant point: the synchronization of each pulses is done on the rising edge of the CPPM signal. The other point is that the edges of pulses are exactly catched by hardware counter with a precision of 0.5 microsecond. TCNT1 counter is a true running counter and is never resetted.  

Implementation has been extensively tested on a Orange R615X (6 channels) receiver and a Spektrum DX8 (DSM2/DSMX) transmitter. Older DX6i DSM2 or newer DX6i DSMX work similary.

Protect the input signal from glitch that can catch ICR1. Add a 1nF capacitor between Arduino pin 8 and GND and insert a 470ohms resistor between pin 8 and signal.

The main page is http://jean-marc.paratte.ch/articles/arduino-cppm/

### Installation

1. Download the ZIP file and expand it in the folder Documents/Arduino/libraries/CPPM. 
2. Start or Restart the Arduino environment.
3. Try the Monitor example.
