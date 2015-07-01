#include <avr/io.h>
#include <util/delay.h>


#define LED PB4
#define BUTTON PB3
#define SPEAKER PB1
#define DEBOUNCE 100

#define led_on() do {							\
		PORTB |= (1 << LED);					\
	} while (0)

#define led_off() do {							\
		PORTB &= ~(1 << LED);					\
	} while (0)

struct button {
	int raw1;  /* raw input value */
	int raw2;  /* old raw value */
	int val1; /* input value after debounce */
	int val2; /* old value */
	int debounce; /* debounce "timer" */
};

static int state = 0;

void button_init(struct button *b) {
	b->raw1 = 1;
	b->raw2 = 1;
	b->val1 = 1;
	b->val2 = 1;
	b->debounce = 0;
}

void check_button(struct button *b) {
	b->raw1 = PINB & (1 << BUTTON);

	if ( b->raw1 != b->raw2 )
		b->debounce = 0;
		
	if ( b->debounce++ > DEBOUNCE ) {
		b->val2 = b->val1;
		b->val1 = b->raw1;

		/* the leading edge toggles state */
		if ( !b->val1 && b->val2 ) {
			state = state ? 0 : 1;
			if ( state ) 
				led_on();
			else 
				led_off();
		} 

		b->debounce = 0;
	}
	b->raw2 = b->raw1;
}

int main(void) {
	struct button b;

	/* make LED an output */
	DDRB |= (1 << LED) | (1 << SPEAKER);
	/* enable pull-up resistor for BUTTON */
	PORTB |= 1 << BUTTON;

#if 0
	/* Fast PWM, top is 0xFF, non-inverting */
	TCCR0A |= (1 << WGM01) | (1 << WGM00) | (1 << COM0A1);// | (1 << COM0A0);
	/* 64 pre-scale ~= 488 Hz*/
	TCCR0B |= (1 << CS00) | (1 << CS02);// | (1 << CS00);

	OCR0A = 128;
#endif	

#define OCR0A_VAL 0x81

#if 1
	/* fast PWM non-inverting, prescaler /256 */
	TCCR0A = (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
	TCCR0B = (1 << WGM02) | (1<<CS02) | (0<<CS01) | (0<<CS00);
	TCNT0=0x00;
	OCR0A=0x81;
	//OCR0B=0x41;
	OCR0B=0x00;
#endif

	led_off();
	button_init(&b);

	int count = 0;
	//int count2 = 1000;
#if 1
#define NUM_NOTES 16
	int note[NUM_NOTES];
	int n = 0;
#endif
	//	note[0] = 0xff;
	//	note[1] = 0xb0;
	//note[0] = 0x80;
	//note[1] = 0x70;

#if 0
	note[0] = 239;
	note[1] = 225;  
	note[2] = 213; 
	note[3] = 201;
	note[4] = 190;
	note[5] = 179;
	note[6] = 169;
	note[7] = 159;  
	note[8] = 150; 
	note[9] = 142;
	note[10] = 134;
	note[11] = 127;
#endif

#if 0
	note[0] = 0x60;
	note[1] = 0x55;  
	note[2] = 0x4c; 
	note[3] = 0x48; /* 4th */
	note[4] = 0x40; /* 5th */
	note[5] = 0x00;
#endif

#if 1
	note[0] = 0x60;
	note[1] = 0xc0;
	note[2] = 0x60;
	note[3] = 0xc0;
	note[4] = 0x40;
	note[5] = 0x0;
	note[6] = 0x40;
	note[7] = 0x0;
	note[8] = 0x39;
	note[9] = 0x0;
	note[10] = 0x39;
	note[11] = 0x0;
	note[12] = 0x40;
	note[13] = 0x40;
	note[14] = 0x40;
	note[15] = 0x0;
#endif
	while(1) {
		check_button(&b);
#if 0
		if (count++ % 1000 == 0){
		    OCR0B = (OCR0B + 1) % (OCR0A_VAL - 1) + 1;
		}
#endif

#if 1
		//if ((count++ == 0) && (count2++ % 2 == 0)){
		if ((count++ == 0)){
			OCR0A = note[n];
			n = (n + 1) % NUM_NOTES;
		}
#endif

#if 0
		if (count++ > 30000){
			TCCR0B ^= (1 << CS02);
 			TCNT0 =0;
			count = 0;
		}
#endif

		/* I don't think we need to use a second timer to advance
		   through the waveform with the appropriate divider for a
		   particular note... */
	}


#if 0
		_delay_ms(1000);
#endif

	return 0;
}
