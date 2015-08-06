
#include <CPPM.h>

//#include <avr/io.h>
//#include <Arduino.h>

//------------------------------------------------------------------------------

void iservos_reset(bool errored);

//------------------------------------------------------------------------------

volatile uint16_t CPPM_T_W; // extended TCNT0 timer maintained by cycle function
volatile uint16_t CPPM_T_X; // extended TCNT0 timer maintained by cycle function
volatile uint16_t CPPM_T_T; // extended TCNT0 timer timeout

volatile bool CPPM_T_cycling; // GPIOR0.0 = CPPM_T_cycle() in action, could be stopped by CPPM_T_interrupt()
volatile bool CPPM_T_syncing; // GPIOR0.1 = CPPM sync falling edge detected
volatile bool CPPM_T_checking; // GPIOR0.2 = CPPM check timeout

//------------------------------------------------------------------------------

uint16_t CPPM_T_get()
{
	uint16_t tcnt_x;
	cli();
	tcnt_x = CPPM_T_X;
	sei();
	return tcnt_x;
}

void CPPM_T_set(uint16_t tcnt_x)
{
	cli();
	CPPM_T_X = tcnt_x;
	sei();
}

void CPPM_T_cycle()
{
	CPPM_T_cycling = true;
	cli();
	CPPM_T_W = TCNT1;
	sei();
	cli();
	if (CPPM_T_cycling) CPPM_T_X = CPPM_T_W;
	sei();
	CPPM_T_cycling = false;
}

uint16_t CPPM_T_interrupt()
{
	uint16_t tcnt_x;
	tcnt_x = ICR1;
	CPPM_T_X = tcnt_x;
	CPPM_T_W = tcnt_x;
	CPPM_T_cycling = false;
	return tcnt_x;
}

uint16_t CPPM_T_timeout()
{
	uint16_t tcnt_x;
	cli();
	tcnt_x = CPPM_T_X - CPPM_T_T;
	sei();
	return tcnt_x;
}

void CPPM_T_check()
{
		if (CPPM_T_checking && (uint8_t) (CPPM_T_timeout()>>8)==0) iservos_reset(true);
}

void CPPM_T_setup()
{

	ICR1 = TCNT0; // init the Input Capture Register

	OCR1A = TCNT0; // init the Output Compare Register

	// Configure timer1: disable PWM, set prescaler /8 (0.5 usec ticks)
	TCCR1A = (1<<COM1A0);	// Toggle OC1A/OC1B on Compare Match.
	TCCR1B = (1<<ICNC1) | (0<<ICES1) | (1<<CS11); // falling edge
	TCCR1C = 0;

	CPPM_T_W = CPPM_T_X = TCNT0; // init CPPM_T_X
}

//------------------------------------------------------------------------------

void iservos_reset(bool errored)
{
	// disable "pin change" interrupt from CPPM input frame...
//	cbi(PCMSK,PCINT4);
	bitClear(TIMSK1, ICIE1); // disable interrupt

//	cbi(GPIOR0,CPPM_T_syncing);
//	cbi(GPIOR0,CPPM_T_checking);
	CPPM_T_syncing = false;
	CPPM_T_checking = false;

//	PORTB = (PORTB & ~CPPM_PBMASK); // clear PWM channels

	CPPM.state = 0;
	if (errored) CPPM.errors++; else CPPM.errors = 0;
	CPPM.iservo = 0;
	CPPM.nservo = CPPM_MSERVO;
	CPPM.jservo = 0;
//	CPPM.kservo = 0;

	// enable "pin change" interrupt from CPPM input frame...
//	sbi(PCMSK,PCINT4);
	// Enable Timer1 input capture interrupt...
	TCCR1B = (1<<ICNC1) | (0<<ICES1) | (1<<CS11); // falling edge
	bitSet(TIFR1, ICF1); // clr pending interrupt
	bitSet(TIMSK1, ICIE1); // enable interrupt
}

void iservos_setup()
{
	// Configure the input capture pin
	pinMode(CPPM_ICP1, INPUT_PULLUP);

	iservos_reset(false);
}

void oservos_setup()
{
	// Configure the output compare pin
	digitalWrite(CPPM_OC1A, HIGH);
	pinMode(CPPM_OC1A, OUTPUT);

	CPPM.oservo = 0;
	for (int i=0; i<CPPM_MSERVO; i++) CPPM.oservos[i] = CPPM_T_round(R615X_PULSE_CENTER);

	// start CPPM frame after 22ms...
	OCR1A += CPPM_T_round(R615X_FRAME_LENGTH);

	// Enable Timer1 output compare interrupt...
	bitSet(TIFR1, OCF1A); // clr pending interrupt
	bitSet(TIMSK1, OCIE1A); // enable interrupt
}

//------------------------------------------------------------------------------

//ISR(PCINT0_vect)
ISR(TIMER1_CAPT_vect)
{
	uint16_t tcnt0 = CPPM_T_interrupt(); // get time from extended TCNT0 timer

//	if (PB_tst(PB4)) // ? rising edge => end 300us synchro pulse
	if (TCCR1B & (1<<ICES1)) // rising edge => end 300us synchro pulse ?
	{
		TCCR1B = (1<<ICNC1) | (0<<ICES1) | (1<<CS11); // next falling edge

//		if (tbi(GPIOR0,CPPM_T_syncing)) // ? follow a start edge
		if (CPPM_T_syncing) // ? follow a start edge
		{
//			cbi(GPIOR0,CPPM_T_syncing);
			CPPM_T_syncing = false;

			CPPM.sync2 = tcnt0 - CPPM.time0; // compute width of synch pulse
			CPPM.time1 = tcnt0;

			CPPM._sync2[CPPM.iservo] = CPPM.sync2; // store sync width of current PWM servo pulse

//			if(	CPPM.sync2 < CPPM_T_floor(FRSKY_PULSE_SYNC-10) ||
//				CPPM.sync2 > CPPM_T_ceil(FRSKY_PULSE_SYNC+50) ) // check sync width... (FRSKY_PULSE_SYNC+20) is too short with R615X
			if(	CPPM.sync2 < CPPM_PULSE_SYNC_MIN_FLOOR ||
				CPPM.sync2 > CPPM_PULSE_SYNC_MAX_CEIL )
			{
				iservos_reset(true);
			}
			else
			{
				if (CPPM.state==0) CPPM.state = 1; // 1st well formed sync pulse found.

				if (CPPM.state==1) // ? get pulse until gap pulse found
				{
//					CPPM_T_T = tcnt0 + CPPM_T_ceil(R615X_FRAME_NOTSYNC-FRSKY_PULSE_SYNC); // could be a stange frame if wait so long !
					CPPM_T_T = CPPM.time5 + CPPM_FRAME_NOTSYNC_CEIL; // could be a stange frame if wait so long !
				}
				else // : gap pulse found => start time of frame known.
				if (CPPM.state==2) // ? get pulses and compute nservo.
				{
//					CPPM_T_T = CPPM.time5 + CPPM_T_ceil(R615X_FRAME_NOTSYNC); // 2% max oscillator error
					CPPM_T_T = CPPM.time5 + CPPM_FRAME_NOTSYNC_CEIL; // set frame timeout
				}
				else // : check pulses and gap.
				{
					if (CPPM.iservo<CPPM.nservo)
//						CPPM_T_T = CPPM.time0 + CPPM_T_ceil(R615X_PULSE_CENTER+R615X_PULSE_C150PC); // middle stick+150%
//						CPPM_T_T = CPPM.time0 + CPPM_T_ceil(R615X_PULSE_CENTER+R615X_PULSE_C200PC); // middle stick+200%
						CPPM_T_T = CPPM.time0 + CPPM_PULSE_CENTER_PLUS_C200PC_CEIL; // middle stick+200%
					else
//						CPPM_T_T = CPPM.time5 + CPPM_T_ceil(R615X_FRAME_NOTSYNC);
						CPPM_T_T = CPPM.time5 + CPPM_FRAME_NOTSYNC_CEIL;
				}
//				sbi(GPIOR0,CPPM_T_checking);
				CPPM_T_syncing = true;
			}
		}
	}
	else // : falling edge => start 300us sync pulse and start PWM servo pulse.
	{
		TCCR1B = (1<<ICNC1) | (1<<ICES1) | (1<<CS11); // next rising edge

//		if (state==3) PORTB = (PORTB & ~CPPM_PBMASK) | (kservo & CPPM_PBMASK); // update PWM channels, stop current pulse, start next pulse

		CPPM._received = false;

//		sbi(GPIOR0,CPPM_T_syncing);
		CPPM_T_syncing = true;

//		CPPM_T_T = tcnt0 + CPPM_T_ceil(FRSKY_PULSE_SYNC+50);
		CPPM_T_T = tcnt0 + CPPM_PULSE_SYNC_MAX_CEIL;
//		sbi(GPIOR0,CPPM_T_checking);
		CPPM_T_checking = true;

		if (CPPM.state==0)
		{
			CPPM.time5 = CPPM.time0 = tcnt0; // set start time of next PWM servo pulse (and sync pulse)
		}
		else // : state>=1
		{
			CPPM.puls3 = tcnt0 - CPPM.time0; // compute width of elapsed pulse
			CPPM.time0 = tcnt0; // set start time of next PWM servo pulse (and sync pulse)

			CPPM._puls3[CPPM.iservo] = CPPM.puls3; // store width of servo pulse

			int puls3i = ((signed) CPPM.puls3 - R615X_PULSE_CENTER) / 4; // middle centered servo pulse
//			int puls3i = (signed) CPPM.puls3 / 4 - (CPPM.sync2 + CPPM.sync2/4); // middle centered servo pulse
			if (puls3i > 127) puls3i = 127; else if (puls3i < -128) puls3i= -128;
			CPPM._puls3i8[CPPM.iservo] = puls3i;

//			if (CPPM.puls3 < CPPM_T_floor(R615X_PULSE_CENTER-R615X_PULSE_C150PC)) // too short servo pulse (middle stick-150%) ?
//			if (CPPM.puls3 < CPPM_T_floor(R615X_PULSE_CENTER-R615X_PULSE_C200PC)) // too short servo pulse (middle stick-200%) ?
			if (CPPM.puls3 < CPPM_PULSE_CENTER_MINUS_C200PC_FLOOR) // too short servo pulse (middle stick-200%) ?
			{
				iservos_reset(true);
			}
			else
//			if (CPPM.puls3 > CPPM_T_ceil(R615X_PULSE_CENTER+R615X_PULSE_C150PC)) // is a gap pulse (middle stick+150%) ?
//			if (CPPM.puls3 > CPPM_T_ceil(R615X_PULSE_CENTER+R615X_PULSE_C200PC)) // is a gap pulse (middle stick+200%) ?
			if (CPPM.puls3 > CPPM_PULSE_CENTER_PLUS_C200PC_CEIL) // is a gap pulse (middle stick+200%) ?
			{
				CPPM.cppm4 = tcnt0 - CPPM.time5; // compute length of elapsed CPPM frame
				CPPM.time5 = tcnt0; // set start time of next CPPM frame

				CPPM._puls3[CPPM.iservo+1] = CPPM.cppm4; // store CPPM frame length
				CPPM._sync2[CPPM.iservo+1] = 0;

//				if(	(CPPM.state==3 && CPPM.cppm4 < CPPM_T_floor(R615X_FRAME_LENGTH)) || // frame length too short ?
				if(	(CPPM.state==3 && CPPM.cppm4 < CPPM_FRAME_LENGTH_FLOOR) || // frame length too short ?
//					CPPM.cppm4 > CPPM_T_ceil(R615X_FRAME_NOTSYNC) ) // frame length too long ?
					CPPM.cppm4 > CPPM_FRAME_NOTSYNC_CEIL ) // frame length too long ?
				{
					iservos_reset(true);
				}
				else
				{
					if (CPPM.state==2) CPPM.nservo = CPPM.iservo;

					CPPM.iservo = 0; // set crnt servo (1st)
					CPPM.jservo = 1; // set next servo (2nd)
//					CPPM.kservo = CPPM.lservo[1]; // set mask of next servo

					if (CPPM.state < 3) CPPM.state++;
				}
			}
			else // valid servo pulse.
			{
				CPPM.iservo = CPPM.jservo; // set index of current servo pulse

				if (CPPM.jservo > CPPM_MSERVO) // servos overflow ?
				{
					iservos_reset(true);
				}
				else
				if (CPPM.jservo > CPPM.nservo) // servos overflow ?
				{
					iservos_reset(true);
				}
				else
				if (CPPM.jservo == CPPM.nservo)
				{
					CPPM.jservo = 0;
//					kservo = lservo[0]; // set next mask of 1st servo

					CPPM._received = true;
				}
				else
				{
					CPPM.jservo++;
//					kservo = lservo[jservo]; // set next mask of next servo
				}
			}
		}
	}
}

//------------------------------------------------------------------------------

ISR(TIMER1_COMPA_vect) // *2015-06-05,+2015-02-05
{
	static uint16_t time5 = 0;

	if (CPPM.oservo < CPPM_MSERVO) // PPM pulse ?
	{
		if ((PINB & _BV(PINB1))) // rising edge ?
		{
			OCR1A += CPPM.oservos[CPPM.oservo] - CPPM_T_round(R615X_PULSE_SYNC);

			CPPM.oservo++; // next PPM pulse (or gap)
		}
		else // falling edge.
		{
			if (CPPM.oservo == 0) time5 = OCR1A;

			OCR1A += CPPM_T_round(R615X_PULSE_SYNC);

//			CPPM._sent = false;
		}
	}
	else // gap pulse.
	{
		if ((PINB & _BV(PINB1))) // rising edge ?
		{
			OCR1A = time5 + CPPM_T_round( R615X_FRAME_LENGTH );

			CPPM.oservo = 0; // next PPM pulse
		}
		else // falling edge.
		{
			OCR1A += CPPM_T_round(R615X_GAP_SYNC);

			CPPM._sent = true;
		}
	}
}

//------------------------------------------------------------------------------

void CPPM_Class::begin()
{
	CPPM_T_setup();

	iservos_setup();

	oservos_setup();
}

void CPPM_Class::end()
{
	bitClear(TIMSK1, OCIE1A); // disable interrupt
	bitClear(TIMSK1, ICIE1); // disable interrupt
}

void CPPM_Class::cycle()
{
	CPPM_T_cycle();
	CPPM_T_check();
}

bool CPPM_Class::synchronized()
{
	return state == 3;
}

bool CPPM_Class::received(void) // +2015-02-05
{
	bool received = _received;
	_received = false;
	return received;
}

bool CPPM_Class::sent(void) // +2015-06-23
{
	bool sent = _sent;
	_sent = false;
	return sent;
}

int CPPM_Class::read(int n)
{
	uint16_t *servo2_p = &_puls3[n];
	cli();
	uint16_t servo2 = *servo2_p;
	sei();
	return (int) servo2;
}

void CPPM_Class::write(int n, int v) // +2015-04-01
{
	uint16_t *oservo_p = &oservos[n];
	cli();
	*oservo_p = v;
	sei();
}

int CPPM_Class::read_us(int n)
{
	return (CPPM_T_div*(long)read(n)+(CPPM_T_mul-1)/2)/CPPM_T_mul; //round
}

void CPPM_Class::write_us(int n, int v) // +2015-04-01
{
	write(n, CPPM_T_round(v));
}

CPPM_Class::operator bool()
{
	return true;
}

CPPM_Class CPPM;
