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

//------------------------------------------------------------------------------

// all times are written with a granularity of 1us

#define FRSKY_PULSE_SYNC 300

#define R615X_FRAME_LENGTH 21980 // DX8-R615X 22ms frame length
#define R615X_PULSE_CENTER 1504 // DX8-R615X center of pulse
#define R615X_PULSE_C100PC 413 // DX8-R615X 100% centered stick mouvement
#define R615X_PULSE_C125PC 516 // DX8-R615X 125% centered stick mouvement
#define R615X_PULSE_C150PC 620 // DX8-R615X 150% centered stick mouvement
#define R615X_PULSE_C200PC 826 // DX8-R615X 200% centered stick mouvement

#define R615X_FRAME_NOTSYNC 22765 // R615X not synchronized +/-8us
#define R615X_PULSE_SYNC 304 // R615X neg sync pulse starting PPM pulse
#define R615X_GAP_SYNC 312 // R615X neg sync pulse starting GAP pulse

#define R920X_FRAME_LENGTH 21980 // DX8-R920X 22ms frame length
#define R920X_PULSE_CENTER 1509 // DX8-R920X center of pulse
#define R920X_PULSE_C100PC 455 // DX8-R920X 100% centered stick mouvement
#define R920X_PULSE_C125PC 568 // DX8-R920X 125% centered stick mouvement
#define R920X_PULSE_C150PC 682 // DX8-R920X 150% centered stick mouvement
#define R920X_PULSE_C200PC 909 // DX8-R920X 200% centered stick mouvement

#define R920X_FRAME_NOTSYNC 22121 // R920X not synchronized +/-2us
#define R920X_PULSE_SYNC 138 // R920X neg sync pulse starting PPM pulse
#define R920X_GAP_SYNC 138 // R920X neg sync pulse starting GAP pulse

#define HXT900_DEGREE 11 // HXT900 9gr servo [us/°] (+/-30° standard deviation, +/-60° extended deviation)

//------------------------------------------------------------------------------

#define CPPM_PULSE_SYNC_MIN_FLOOR CPPM_T_floor(138 - 38) //(FRSKY_PULSE_SYNC - 50) //10)
#define CPPM_PULSE_SYNC_MAX_CEIL CPPM_T_ceil(312 + 88) //(FRSKY_PULSE_SYNC + 100) //50) // check sync width... (FRSKY_PULSE_SYNC+20) is too short with R615X

//#define CPPM_FRAME_NOTSYNC_MINUS_PULSE_SYNC_CEIL CPPM_T_ceil(R615X_FRAME_NOTSYNC - FRSKY_PULSE_SYNC) // could be a stange frame if wait so long !
//#define CPPM_FRAME_NOTSYNC_CEIL CPPM_T_ceil(R615X_FRAME_NOTSYNC) // 2% max oscillator error
#define CPPM_FRAME_NOTSYNC_CEIL CPPM_T_ceil(22765 + 135) //(R615X_FRAME_NOTSYNC + 100) // 2% max oscillator error

#define CPPM_PULSE_CENTER_PLUS_C200PC_CEIL CPPM_T_ceil(1500 + 909 + 81) //(R615X_PULSE_CENTER + R615X_PULSE_C200PC) // middle stick+200%
#define CPPM_PULSE_CENTER_MINUS_C200PC_FLOOR CPPM_T_floor(1500 - 909 - 81) //(1500 (R615X_PULSE_CENTER - R615X_PULSE_C200PC) // too short servo pulse (middle stick-200%) ?

#define CPPM_FRAME_LENGTH_FLOOR CPPM_T_floor(21980 - 980) //(R615X_FRAME_LENGTH) // frame length too short ?

#define CPPM_MSERVO 9 // 9 servos maximum in a 22ms frame

enum {CPPM_AILE, CPPM_ELEV, CPPM_THRO, CPPM_RUDD, CPPM_GEAR, CPPM_AUX1, CPPM_AUX2, CPPM_AUX3, CPPM_AUX4};
// standard definitions of Spektrum receivers (AR6200, R615X, R920X,...)

//------------------------------------------------------------------------------

#define CPPM_T_err(t) (2*((long)t)/100) // +/-2% oscillator error (ATtiny85) of stick error (DX8)

#define CPPM_T_mul 2
#define CPPM_T_div 1
#define CPPM_T_ceil(t) ((CPPM_T_mul*((long)t+CPPM_T_err(t))+(CPPM_T_div-1))/CPPM_T_div) // round up
#define CPPM_T_round(t) ((CPPM_T_mul*(long)t+(CPPM_T_div-1)/2)/CPPM_T_div) // round
#define CPPM_T_floor(t) (CPPM_T_mul*((long)t-CPPM_T_err(t))/CPPM_T_div) // round down

//------------------------------------------------------------------------------

#define CPPM_ICP1 8 // Input Capture Pin of Arduino UNO is pin 8 - ICP1 (Atmega328 PB0)
#define CPPM_OC1A 9 // Output Compare Pin of Arduino UNO is pin 9 - OC1A (Atmega328 PB1)

//------------------------------------------------------------------------------

class CPPM_Class
{
	private:

	public:

		uint8_t state; // state of synchronization: 0=no signal or errored, 1=start frame, 2=synchronized
		uint8_t errors; // count of frame errors

		uint16_t time0; // start time of synch pulse (falling edge)
		uint16_t time1; // end time of synch pulse (rising edge)
		uint16_t sync2; // width of synch pulse
		uint16_t puls3; // width of servo pulse (including starting synch pulse)
		uint16_t cppm4; // length of CPPM frame
		uint16_t time5; // start time of CPPM frame (start time of 1st sync pulse)

		bool _received; // +2015-02-05
		bool _sent;		// +2015-06-23

		uint8_t iservo; // index servo
		uint8_t nservo; // found servos
		uint8_t jservo; // next index servo
//		uint8_t kservo; // next mask servo

		uint16_t _sync2[CPPM_MSERVO + 2]; // width of synch pulses
		uint16_t _puls3[CPPM_MSERVO + 2]; // width of servo pulses
		// the 2 extra servos are the gap pulse and the total frame length.

		int8_t _puls3i8[CPPM_MSERVO + 2]; // width of servo pulses

		uint8_t oservo; // cppm output servo index
		uint16_t oservos[CPPM_MSERVO]; // cppm output servo pulse width

		void begin(void);

		void end(void);

		void cycle(void);

		bool synchronized(void);

		bool received(void); // +2015-02-05

		bool sent(void); // +2015-06-23

		int read(int n);

		void write(int n, int v); // +2015-04-06

		int read_us(int n);

		void write_us(int n, int v); // +2015-04-06

		operator bool();
};

extern CPPM_Class CPPM;

#endif
