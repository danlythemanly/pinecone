#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/interrupt.h> 
#include <util/delay.h>
#define adc_disable()  (ADCSRA &= ~(1<<ADEN))

#define DECAY 4
#define EIGHTH_NOTE 70
#define SIXTEENTH_NOTE (EIGHTH_NOTE >> 1)
#define RIT 20

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

/* sawtooth-like with half amplitude */
static void refresh(void) {
	int i;
	uint8_t mid = wav_len >> 1;
	for (i=0; i < mid; i++)
		wav[i] = i >> 1;
	for (i = mid; i < wav_len; i++)
		wav[i] = (i + 256 - wav_len) >> 1;
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

int main() {
	
	DDRB |= (1 << PB1) | (1 << PB0) | (1 << PB4) | (1 << PB3);

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

	sei();
	for(;;){
		play_twinkle();
		_delay_ms(EIGHTH_NOTE << 3);
	}
  
	return 0;
}

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
