
#include <avr/io.h>
#include <Arduino.h>

#define CPPM_ICP1 8 // Input Capture Pin 1 of Timer1 is Arduino pin 8 (Atmega328 PB0)

#define CPPM_OC1A 9 // Output Compare Pin A of Timer1 is Arduino pin 9 (Atmega328 PB1)

#define CPPM_MSERVO 9 // 9 servos maximum in a 22ms frame

// all times are written with a granularity of 0.5us (16MHz / 8)

#define R615X_FRAME_LENGTH (21980*2)
#define R615X_PULSE_CENTER (1500*2)
#define R615X_PULSE_C125PC (512*2)

void ServoMeter_begin()
{
	// Configure the input capture pin
	pinMode(CPPM_ICP1, INPUT_PULLUP);

	// Configure the output compare pin
	digitalWrite(CPPM_OC1A, LOW);
	pinMode(CPPM_OC1A, OUTPUT);

	// Configure timer1: disable PWM, set prescaler /8 (0.5 usec ticks)
//	TCCR1A = 0;
//	TCCR1B = _BV(ICNC1) | _BV(ICES1)*0 | _BV(CS11);
//	TCCR1C = 0;
	TCCR1A = _BV(COM1A0);	// Toggle OC1A/OC1B on Compare Match.
	TCCR1B = _BV(ICNC1) | _BV(ICES1)*0 | _BV(CS11);
	TCCR1C = 0;

	OCR1A = 305*2; // start rising edge in 305us...

	// Enable Timer1 input capture interrupt...
	bitSet(TIFR1, ICF1); // clr pending interrupt
	bitSet(TIMSK1, ICIE1); // enable interrupt

	// Enable Timer1 output compare interrupt...
	bitSet(TIFR1, OCF1A); // clr pending interrupt
	bitSet(TIMSK1, OCIE1A); // enable interrupt

	// Enable Timer1 overflow...
	bitSet(TIFR1, TOV1); // clr pending interrupt
	bitSet(TIMSK1, TOIE1); // enable interrupt
}

void ServoMeter_end()
{
	bitClear(TIMSK1, ICIE1); // disable interrupt
	bitClear(TIMSK1, TOIE1); // disable interrupt
}

uint16_t t0,t1,t2,h,l,w;

ISR(TIMER1_CAPT_vect)
{
	if(TCCR1B &	_BV(ICES1)) // rising edge ?
	{
		TCCR1B = _BV(ICES1)*0 | _BV(CS11); // next falling edge.

		t2 = ICR1;

		h = t1 - t0;
		l = t2 - t1;
		w = t2 - t0;

		t0 = t2;
	}
	else // falling edge => start 300us synch pulse.
	{
		TCCR1B = _BV(ICES1)*1 | _BV(CS11); // next rising edge.

		t1 = ICR1;
	}
}

ISR(TIMER1_COMPA_vect)
{
	if ((PINB & _BV(PINB1)))
	{
		OCR1A += R615X_PULSE_CENTER;
	}
	else
	{
		OCR1A += R615X_FRAME_LENGTH - R615X_PULSE_CENTER;
	}
}

ISR(TIMER1_OVF_vect)
{
}

////////////////////////////////////////////////////////////////////////////////

void setup()
{
	Serial.begin(9600);

	ServoMeter_begin();
}

void loop()
{
	Serial.print(h); Serial.print(", ");
	Serial.print(l); Serial.print(", ");
	Serial.print(w); Serial.print("\n");
	Serial.flush();

	delay(100);
}
