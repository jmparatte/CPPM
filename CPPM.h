/*	CPPM - Combined PPM
	===================

	Material of experimentation: Spektrum DX8 transmitter and Orange R615X receiver (CPPM on Batt/Bind connector)

	Time definition of counter: 16MHz/8 = 2MHz => 0.5us

	A "CPPM servo pulse" is started and ended on 2 rising edges of CPPM signal.
	A falling edge is inserted about 305us before the end of servo pulse. This is the "CPPM sync pulse".
	A "CPPM synch pulse" is started on 1 falling edge and ended on a rising edge.
	The width of synch pulse is 305ns + an undefined extra time, but no more than 20us (as experimented).
	            ____                          _______________________________________                        ____
	Pulse CPPM: Pn-1\Sn-1=synch pulse 305us+x/Pn=1520us+/-512us - synch pulse 305us+x\Sn=synch pulse 305us+x/Pn+1
	Pn-1: previous servo pulse ended by the Sn-1 synch pulse
	Sn-1: previous synch pulse ending the Pn-1 servo pulse, beginning the Pn servo pulse
	Pn: servo pulse started on rising edge
	Sn: synch pulse ending the servo pulse on rising edge (negativ pulse)
	Pn+1: next servo pulse started on rising edge
	            ___    _____    _____    ___    _____    ____    ___
	Frame CPPM: ...\Sg/P0-S0\S0/P1-S1\S1/...\S4/P5-S5\S5/G-Sg\Sg/... 21980us (43'960 counts +/-10)
	A frame starts and ends with a rising edge (end of Sg)
	A pulse starts with a rising edge (end of Sx) and ends with a Sx negativ pulse.
	The width of the pulse includes the Sx pulse, starts and ends on the rising edge of a Sx negativ pulse.

	Sg,S0,S1,S2,S3,S4,S5(,S6,S7,S8): about 305us (610 counts, varying -0/+40 counts)

	P0,P1,P2,P3,P4,P5(,P6,P7,P8): 1520us (3040 counts, stick at middle)

	G: synchronization gap (sold of frame), greater than 1520+512us (2032 counts)

	Pulse range: varying +/-100% of a maximum of +/-125% (Spektrum specifications)
	+/-125%: +/-512us (+/-1024 counts)
	+/-100%: +/-409.6us (+/-819.2 counts)

	CPPM conventional channel assignments (Spektrum specifications): aileron - elevator - throttle - rudder - gear - flaps
	aileron, elevator, rudder: stick -100%..0..+100%
	rudder: stick 0..100%
	gear: on/off switch -100%/100%
	flaps (Spektrum DX8): 0/1/2 switch -100%/0/+100%

	*/

#ifndef CPPM_h
#define CPPM_h

#include <Arduino.h>

#define CPPM_ICP1 8 // Input Capture Pin 1 of Timer1 is Arduino pin 8

#define CPPM_MSERVO 9 // 9 servos maximum in a 22ms frame

// all times are written with a granularity of 0.5us (16MHz / 8)

class CPPM_Class
{
	private:

	public:

		enum servo_names {AILE, ELEV, THRO, RUDD, GEAR, AUX1}; // standard definitions of some receivers (AR6200, R615X, ...)

		uint8_t ovf; // count of overflows (maximum 2)
		uint8_t state; // state of synchronization: 0=no signal or errored, 1=start frame, 2=synchronized
		uint8_t errors; // count of frame errors
		uint8_t berrors; // bits of errors

		uint16_t sync0; // time of falling edge of synch pulse (start)
		uint16_t sync1; // time of rising edge of synch pulse (end => start pulse)
		uint16_t sync2; // width of servo pulse (including ending synch pulse)
		uint16_t sync3; // width of synch pulse
		uint16_t sync4; // time of start of 1st pulse

		uint8_t iservo; // index servo
		uint8_t nservo; // found servos
		uint16_t servos2[CPPM_MSERVO + 2]; // servo pulse width
		uint16_t servos3[CPPM_MSERVO + 2]; // synch pulse width (included at end of servo pulse)
		// the 2 extra servos are the last gap pulse and the total frame size

		void begin(void);

		void end(void);

		bool synchronized(void);

		int read(int n);

		operator bool();

};

extern CPPM_Class CPPM;

#endif
