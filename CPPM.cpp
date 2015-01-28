
#include <CPPM.h>

void CPPM_Class::begin()
{
	// Configure the input capture pin
	pinMode(CPPM_ICP1, INPUT_PULLUP);

	ovf = 0;
	state = 0;
	errors = 0;
	berrors = 0;

	// Configure timer1: disable PWM, set prescaler /8 (0.5 usec ticks)
	TCCR1A = 0;
	TCCR1B = _BV(ICNC1) | _BV(ICES1)*0 | _BV(CS11);
	TCCR1C = 0;

	// Enable Timer1 overflow...
	bitSet(TIFR1, TOV1); // clr pending interrupt
	bitSet(TIMSK1, TOIE1); // enable interrupt

	// Enable Timer1 input capture interrupt...
	bitSet(TIFR1, ICF1); // clr pending interrupt
	bitSet(TIMSK1, ICIE1); // enable interrupt

//	Serial.println("CPPM_Class::begin(): OK.");
//	Serial.flush();
}

void CPPM_Class::end()
{
	bitClear(TIMSK1, ICIE1); // disable interrupt
	bitClear(TIMSK1, TOIE1); // disable interrupt
}

bool CPPM_Class::synchronized()
{
	return state == 2;
}

int CPPM_Class::read(int n)
{
	uint16_t *servo2_p = &servos2[n];
	cli();
	uint16_t servo2 = *servo2_p;
	sei();
	return (int) servo2;
}

CPPM_Class::operator bool()
{
	return true;
}

ISR(TIMER1_OVF_vect)
{
	// 1 more overflow...
	if(CPPM.ovf == 0) // 1 overflow could be a fault, see TIMER1_CAPT
		CPPM.ovf++;
	else
	{
		if (CPPM.ovf == 1) // 2 overflows and more are a true overflow and it's a fault
		{
			CPPM.ovf++;
//			CPPM.errors++;
			CPPM.berrors |= 1;
		}

		CPPM.state = 0;
//		CPPM.errors++;
		CPPM.iservo = 0;
	}
}

ISR(TIMER1_CAPT_vect)
{
	if(TCCR1B &	_BV(ICES1)) // rising edge => end 300us synchro pulse ?
	{
		TCCR1B = _BV(ICES1)*0 | _BV(CS11); // next falling edge.

		if (CPPM.ovf == 2 || CPPM.ovf == 1 && ICR1 >= CPPM.sync1) // pulse overflow ?
		{
			CPPM.sync1 = ICR1;
			CPPM.ovf = 0;
			CPPM.state = 0;
			CPPM.errors++;
			CPPM.berrors |= 2;
			CPPM.iservo = 0;
			return;
		}
		CPPM.ovf = 0;

		CPPM.sync2 = ICR1 - CPPM.sync1; // compute width of servo pulse
		CPPM.sync3 = ICR1 - CPPM.sync0; // compute width of synch pulse
		CPPM.sync1 = ICR1;

//		if (CPPM.sync3 < (610 - 16) || CPPM.sync3 > (643 + 16) || CPPM.sync2 < (1520*2 - 1024 - 16) || CPPM.sync2 > (1520*2 + 1024 + 16))
		if (CPPM.sync3 < (610 - 16) || CPPM.sync3 > (643 + 16))
//		if (CPPM.sync3 < (610 - 32) || CPPM.sync3 > (643 + 32))
		{
			CPPM.state = 0;
			CPPM.errors++;
			CPPM.berrors |= 4;
			CPPM.iservo = 0;
			return;
		}

		CPPM.servos2[CPPM.iservo] = CPPM.sync2;
		CPPM.servos3[CPPM.iservo] = CPPM.sync3;

		if(CPPM.sync2 > 2*1520+2*512+2*512) // gap pulse ?
		{
			CPPM.nservo = CPPM.iservo;

			CPPM.iservo++;
			CPPM.servos2[CPPM.iservo] = ICR1 - CPPM.sync4;
			CPPM.servos3[CPPM.iservo] = CPPM.sync3;

			CPPM.sync4 = ICR1;

			CPPM.iservo = 0;

			if (CPPM.state < 2) CPPM.state++;
		}
		else // next servo
		{
			if (CPPM.iservo == CPPM_MSERVO) // n servo overflow ?
			{
				CPPM.state = 0;
				CPPM.errors++;
				CPPM.berrors |= 8;
				CPPM.iservo = 0;
				return;
			}
			CPPM.iservo++;
		}
	}
	else // falling edge => start 300us synch pulse.
	{
		TCCR1B = _BV(ICES1)*1 | _BV(CS11); // next rising edge.

		CPPM.sync0 = ICR1;
	}
}

CPPM_Class CPPM;
