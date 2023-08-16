#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/interrupt.h> 
#include <util/delay.h>

#define DECAY 4
#define EIGHTH_NOTE 70
#define SIXTEENTH_NOTE (EIGHTH_NOTE >> 1)
#define RIT 20

#define ROUNDS 8

#define LED PB4
#define BUTTON PB2
#define SPEAKER PB1

#define speaker_on() do {							\
	    DDRB |= 1 << SPEAKER;						\
		PORTB |= (1 << SPEAKER);					\
	} while (0)

#define speaker_off() do {							\
	    DDRB &= ~(1 << SPEAKER);					\
		PORTB &= ~(1 << SPEAKER);					\
	} while (0)

#define led_on() do {							\
		PORTB |= (1 << LED);					\
	} while (0)

#define led_off() do {							\
		PORTB &= ~(1 << LED);					\
	} while (0)

static int do_reset = 0;

#define SIN_LEN 256
#define A 0 
#define G 1
#define F 2
#define E 3
#define D 4 
#define C 5 

static uint8_t notes[10] = {100, 113, 127, 134, 150, 
							169, 179, 201, 225, 253 };

static uint8_t wav[SIN_LEN];
static int decay = 0;
static uint8_t p, last_p;
static uint8_t wav_len = 128;
static uint8_t octave = 0;

#define ENC(n,o,l) ((n) << 4 | (o) << 3 | (l))
#define NOTE(x) ((x) >> 4)
#define OCTAVE(x) (!!((x) & 8))
#define LEN(x) ((x) & 7)

#define TWINKLE_LEN 94
static uint8_t twinkle_enc[TWINKLE_LEN] = {ENC(C,0,2), ENC(C,1,2),
										   ENC(C,0,2), ENC(C,1,2),
										   ENC(G,0,2), ENC(E,1,2), 
										   ENC(G,0,2), ENC(E,1,2),
										   ENC(A,0,2), ENC(F,1,2),
										   ENC(A,0,2), ENC(F,1,2), 
										   ENC(G,0,2), ENC(E,1,2),
										   ENC(C,1,2), ENC(E,1,2),

										   ENC(F,0,2), ENC(D,1,2),
										   ENC(F,0,2), ENC(D,1,2),
										   ENC(E,0,2), ENC(C,1,2), 
										   ENC(E,0,2), ENC(C,1,2),
										   ENC(D,0,2), ENC(G,1,2),
										   ENC(D,0,2), ENC(F,1,2), 
										   ENC(C,0,2), ENC(E,1,2),
										   ENC(C,1,4),
										   
										   ENC(G,0,2), ENC(E,1,2),
										   ENC(G,0,2), ENC(E,1,2),
										   ENC(F,0,2), ENC(D,1,2), 
										   ENC(F,0,2), ENC(D,1,2),
										   ENC(E,0,2), ENC(C,1,2),
										   ENC(E,0,2), ENC(C,1,2), 
										   ENC(D,0,2), ENC(D,1,2),
										   ENC(E,1,2), ENC(F,1,2),
										   
										   ENC(G,0,2), ENC(E,1,2),
										   ENC(G,0,2), ENC(E,1,2),
										   ENC(F,0,2), ENC(D,1,2), 
										   ENC(F,0,2), ENC(D,1,2),
										   ENC(E,0,2), ENC(C,1,2),
										   ENC(E,0,2), ENC(C,1,2), 
										   ENC(D,0,2), ENC(G,1,2),
										   ENC(E,1,2), ENC(D,1,2),
										   
										   ENC(C,0,2), ENC(C,1,2),
										   ENC(C,0,2), ENC(C,1,2),
										   ENC(G,0,2), ENC(E,1,2), 
										   ENC(G,0,2), ENC(E,1,2),
										   ENC(A,0,2), ENC(F,1,2),
										   ENC(A,0,2), ENC(F,1,2), 
										   ENC(G,0,2), ENC(E,1,2),
										   ENC(C,1,2), ENC(E,1,2),
										   
										   ENC(F,0,2), ENC(D,1,2),
										   ENC(F,0,2), ENC(D,1,2),
										   ENC(E,0,2), ENC(C,1,2), 
										   ENC(E,0,2), ENC(C,1,2),
										   ENC(D,0,2), ENC(G,1,2),
										   ENC(D,0,2), ENC(F,1,2), 
										   ENC(C,0,2), ENC(E,1,2),
										   ENC(C,1,4)
};

#define VOLUME_DIVISOR 1

/* sawtooth-like with half amplitude */
static void refresh(void) {
	int i;
	uint8_t mid = wav_len >> VOLUME_DIVISOR;
	for (i=0; i < mid; i++)
		wav[i] = i >> VOLUME_DIVISOR;
	for (i = mid; i < wav_len; i++)
		wav[i] = (i + 256 - wav_len) >> VOLUME_DIVISOR;
}

static void play_twinkle(void) {
	uint8_t i, j;
	for ( i = 0; i < TWINKLE_LEN; i++ ) {
		uint8_t x = twinkle_enc[i];
		wav_len = notes[NOTE(x)];
		refresh();
		octave = OCTAVE(x);
		OCR0A = (127 << octave);

		for ( j = 0; j < LEN(x); j++) {
			if (octave) {
				_delay_ms(EIGHTH_NOTE);
				_delay_ms(EIGHTH_NOTE>>2);
				_delay_ms(EIGHTH_NOTE>>3);
			}
			_delay_ms(EIGHTH_NOTE);
		}

		if ( i > TWINKLE_LEN - 7) {
			for (j = 0; j < i - (TWINKLE_LEN - 7); j++) {
				if (octave) {
					_delay_ms(RIT);
					_delay_ms(RIT>>2);
					_delay_ms(RIT>>3);
				}
				_delay_ms(RIT);
			}
		}
	}
}	

void WDT_off(void) {
	MCUSR = 0x00;
	WDTCR |= _BV(WDCE) | _BV(WDE);
	WDTCR = 0x00;
}
void WDT_on(void) {
	WDTCR &= ~(_BV(WDP1) | _BV(WDP3));
	WDTCR |= _BV(WDP2) | _BV(WDP0);
	WDTCR |= _BV(WDCE) | _BV(WDE);
	WDTCR &= ~_BV(WDIE);
}

void reset(void) {
	led_off();
	speaker_off();
	WDT_on();
	for(;;);
}

int main() {
	int i;

	WDT_off();

	DDRB &= ~(1 << BUTTON);
	DDRB |= (1 << LED);
	DDRB |= (1 << SPEAKER);
	PORTB |= 1 << BUTTON;

	/* unused pins */
	DDRB &= ~(_BV(PB0) | _BV(PB3) | _BV(PB5));
	PORTB |= _BV(PB0) | _BV(PB3) | _BV(PB5);

	// Enable 64 MHz PLL and use as source for Timer1
	PLLCSR |= 1 << PLLE;            /* Enable PLL */
	_delay_ms(100);                 /* Wait for PLL steady state */
	while ((PLLCSR | PLOCK) == 0);  /* Poll PLOCK bit */
	PLLCSR |= PCKE;                 /* Enable 64 MHz PLL for Timer 1 */
 
	TCCR1 = 1<<PWM1A | 2<<COM1A0 | 1<<CS10; // PWM A, clear on match, 1:1 prescale

	OCR1A = 128;                    // 50% duty at start

	TCCR0A = 1<<WGM01;  /* CTC mode */
	TCCR0B = 1 << CS00; /* 8 mhz no prescale */
	OCR0A = 254;        /* divisor */
	TIMSK = 1 << OCIE0A;
  
	refresh();
	p = 0;
	last_p = wav_len = 255;

	led_off();
	speaker_off();
	MCUCR &= ~(_BV(ISC01) | _BV(ISC00));

	ACSR |= _BV(ACD);           //disable the analog comparator
    ADCSRA &= ~_BV(ADEN);       // ADC off to save power

	//turn off the brown-out detector
	MCUCR |= _BV(BODS) | _BV(BODSE); 
	MCUCR &= ~_BV(BODSE);

    GIMSK |= _BV(INT0);              // Enable INT0
    GIMSK &= ~_BV(PCIE);             // disable PCIE

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	sleep_bod_disable();
	sei();
	sleep_cpu();
	cli();
	sleep_disable();
	led_on();
	speaker_on();
	_delay_ms(400);
	sei();
	do_reset = 1;

	for(i = 0; i < ROUNDS; i++){
		play_twinkle();
		_delay_ms(EIGHTH_NOTE << 3);
	}

	reset();
	return 0;
}

/* timer interrupt for sound generation */
ISR(TIMER0_COMPA_vect) {
	OCR1A = wav[p];
	if ( decay == 0 )
		wav[p] = (wav[p] >> 1) + (wav[last_p] >> 1);
	last_p = p++;
	if (p == wav_len) {
		p = 0;
		if (decay++ == DECAY) decay = 0;
	}
}

/* button interrupt */
ISR(INT0_vect) {
	if (do_reset)
		reset();
}
