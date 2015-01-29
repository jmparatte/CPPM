# CPPM Library for Arduino

This library provides an interface for reading a CPPM signal delivered by a RC receiver like Orange R615X.
Commonly, the 22ms frame limit the maximum number of servos/channels to 9.

### Example

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

### 22ms CPPM frame

![22ms CPPM frame](http://jean-marc.paratte.ch/wp-content/uploads/2015/03/cppm-frame.png)
```
Note 1: The trig is set on 1st rising edge of the CPPM frame.
Note 2: The horizontal position is normally centered.
```

### 1520us CPPM pulse

![1520us CPPM pulse](http://jean-marc.paratte.ch/wp-content/uploads/2015/03/cppm-pulse.png)
```
Note 1: The trig origin is set on the rising edge of the CPPM pulse.
Note 2: The horizontal position is 600us left (3 divisions).
```

### Implementation

This library works only on ATmega328 implementation of Arduino UNO, Duemillanove, Leonardo and similar. It uses the Timer1. So PWM `analogWrite()` on pin 9 and 10 are unavailable. The library `Servo` is not compatible because it also uses Timer1.

The CPPM signal is connected to the Arduino pin 8 named ICP1. In a future version, pin 9 OC1A and 10 OC1B will be 2 PPM/CPPM output signals. 

This implementation differs from https://github.com/claymation/CPPM in one significant point: the synchronization of each pulses is done on the rising edge of the CPPM signal. Another point is that edges of pulses are exactly catched by hardware counter ICR1 with a precision of 0.5 microsecond. TCNT1 counter is a true running counter and is never resetted by software.  

Implementation has been extensively tested with an Orange R615X (6 channels) receiver and a Spektrum DX8 (DSM2/DSMX) transmitter. Older DX6i DSM2 or newer DX6i DSMX work similary.

Protect the input signal from glitch that can catch ICP1. Add a 1nF capacitor between ICP1 and GND and insert a 100ohms resistor in the CPPM signal. This is particulary necessary when high currents are switched (dc motors, etc...).

The main page is http://jean-marc.paratte.ch/articles/arduino-cppm/ (in French)

### Installation

1. Download the ZIP file and expand it in the folder Documents/Arduino/libraries/CPPM. 
2. Start or Restart the Arduino environment.
3. Try the Monitor example.
