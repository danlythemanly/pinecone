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
#if 1
#define LENGTH 256
static unsigned wave[LENGTH] = {
0x80, 0x83, 0x86, 0x89, 0x8c, 0x8f, 0x92, 0x95, 
0x98, 0x9b, 0x9e, 0xa1, 0xa4, 0xa7, 0xaa, 0xad, 
0xb0, 0xb3, 0xb6, 0xb9, 0xbb, 0xbe, 0xc1, 0xc3, 
0xc6, 0xc9, 0xcb, 0xce, 0xd0, 0xd2, 0xd5, 0xd7, 
0xd9, 0xdb, 0xde, 0xe0, 0xe2, 0xe4, 0xe6, 0xe7, 
0xe9, 0xeb, 0xec, 0xee, 0xf0, 0xf1, 0xf2, 0xf4, 
0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfb, 
0xfc, 0xfd, 0xfd, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 
0xff, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfd, 0xfd, 
0xfc, 0xfb, 0xfb, 0xfa, 0xf9, 0xf8, 0xf7, 0xf6, 
0xf5, 0xf4, 0xf2, 0xf1, 0xf0, 0xee, 0xec, 0xeb, 
0xe9, 0xe7, 0xe6, 0xe4, 0xe2, 0xe0, 0xde, 0xdb, 
0xd9, 0xd7, 0xd5, 0xd2, 0xd0, 0xce, 0xcb, 0xc9, 
0xc6, 0xc3, 0xc1, 0xbe, 0xbb, 0xb9, 0xb6, 0xb3, 
0xb0, 0xad, 0xaa, 0xa7, 0xa4, 0xa1, 0x9e, 0x9b, 
0x98, 0x95, 0x92, 0x8f, 0x8c, 0x89, 0x86, 0x83, 
0x80, 0x7c, 0x79, 0x76, 0x73, 0x70, 0x6d, 0x6a, 
0x67, 0x64, 0x61, 0x5e, 0x5b, 0x58, 0x55, 0x52, 
0x4f, 0x4c, 0x49, 0x46, 0x44, 0x41, 0x3e, 0x3c, 
0x39, 0x36, 0x34, 0x31, 0x2f, 0x2d, 0x2a, 0x28, 
0x26, 0x24, 0x21, 0x1f, 0x1d, 0x1b, 0x19, 0x18, 
0x16, 0x14, 0x13, 0x11, 0x0f, 0x0e, 0x0d, 0x0b, 
0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x04, 
0x03, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 
0x03, 0x04, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 
0x0a, 0x0b, 0x0d, 0x0e, 0x0f, 0x11, 0x13, 0x14, 
0x16, 0x18, 0x19, 0x1b, 0x1d, 0x1f, 0x21, 0x24, 
0x26, 0x28, 0x2a, 0x2d, 0x2f, 0x31, 0x34, 0x36, 
0x39, 0x3c, 0x3e, 0x41, 0x44, 0x46, 0x49, 0x4c, 
0x4f, 0x52, 0x55, 0x58, 0x5b, 0x5e, 0x61, 0x64, 
0x67, 0x6a, 0x6d, 0x70, 0x73, 0x76, 0x79, 0x7c, 
};
static unsigned wave_ptr = 0;
#endif

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
	//int i;

	/* make LED an output */
	DDRB |= (1 << LED) | (1 << SPEAKER) | (1 << PB0);
	/* enable pull-up resistor for BUTTON */
	PORTB |= 1 << BUTTON;


	TCCR0A |= (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
	TCCR0B |= (1 << WGM02) | (0<<CS00);
	//TCCR0B = (1 << WGM02) | (0<<CS02) | (0<<CS01) | (1<<CS00);
	//TCCR0B = (1 << WGM02) | (0<<CS02) | (1<<CS01) | (0<<CS00);
	TCCR0B = (1 << WGM02) | (0<<CS02) | (1<<CS01) | (0<<CS00);
	TCNT0=0x00;
	//OCR0A=0x81;
	OCR0A=0xff;
	OCR0B=0x40;

	for (;;) {
		//if (((i++ % 128) == 0) && ((j++ % 128) == 0)) {

		//TCNT0 = wave[wave_ptr];
		TCNT0 = 0;
		OCR0A = wave[wave_ptr];
		wave_ptr += 2;
		//}
	}



#if 0
	/* Fast PWM, top is 0xFF, non-inverting */
	TCCR0A |= (1 << WGM01) | (1 << WGM00) | (1 << COM0A1);// | (1 << COM0A0);
	/* 64 pre-scale ~= 488 Hz*/
	TCCR0B |= (1 << CS00) | (1 << CS02);// | (1 << CS00);

	OCR0A = 128;
#endif	
#if 0

#define OCR0A_VAL 0x81

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
