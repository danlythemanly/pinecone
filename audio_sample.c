#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/interrupt.h> 
#include <util/delay.h>
#define adc_disable()  (ADCSRA &= ~(1<<ADEN))

/* All notes based on A4 (440 HZ) having a 256 sample sine wave.

   divisor = ((Clock / samples) / prescale ) / freq
           = (8000000 / 128) / 1 / 440
		   = 142

   1                             1
   --------- = interrupt rate =  ---------
   440 * 128                     freq * samples

   samples = 1 / freq * interrupt rate
           = (440 * 128) / freq
		   
  A4 = 440 --> 128
  G4 = 391.995 --> 144
  F4 = 349.228 --> 161
  E4 = 329.628 --> 171
  D4 = 293.665 --> 192
  C4 = 261.626 --> 215

*/

uint8_t wav_len = 128;


#define SIN_LEN 256 /* A4 */
static uint8_t wav[SIN_LEN];

#ifdef TWO_NOTES
uint8_t p2, last_p2;
uint8_t wav_len2 = 100;
//static uint8_t wav2[SIN_LEN];
#endif

uint8_t p, last_p;

/*
  divisor = ((Clock / samples) / prescale ) / freq
          = (8000000 / 128) / 8 / freq

  A4 = 440 --> 142 --> 17
  G4 = 391.995 --> 159
  F4 = 349.228 --> 179
  E4 = 329.628 --> 190 --> 24
  D4 = 293.665 --> 213 --> 27
  C4 = 261.626 --> 239 --> 30
  F3 = 174.614 --> 44

 */
#if 0
#define F5 89
#define E5 95
#define Eb5 100
#define D5 106
#define Db5 113
#define C5 119
#define B4 127
#define Bb4 134
#define A4 142
#define Ab4 150
#define G4 159
#define Gb4 169
#define F4 179
#define E4 190
#define Eb4 201
#define D4 213
#define Db4 225
#define C4 239
#define B3 253

#define Eb5 0 /* top TWINKLE, MOON */
#define Db5 1
#define B4  2
#define Bb4 3
#define Ab4 4 /* top GOODNIGHT */
#define Gb4 5 /* bottom TWINKLE */
#define F4  6
#define Eb4 7
#define Db4 8 /* bottom MOON */
#define B3  9 /* bottom GOODNIGHT */
#endif

#define A 0 /* top TWINKLE, MOON */
#define G 1
#define F 2
#define E 3
#define D 4 /* top GOODNIGHT */
#define C 5 /* bottom TWINKLE */
#define BL 6
#define AL 7
#define GL 8 /* bottom MOON */
#define FL 9 /* bottom GOODNIGHT */


/* only diatonic notes are needed */
uint8_t notes[10] = {100, 113, 127, 134, 150, 
					 169, 179, 201, 225, 253 };

/* sawtooth-like with full amplitude */
static void refresh(void) {
	int i;
	uint8_t mid = wav_len >> 1;
	for (i=0; i < mid; i++)
		wav[i] = i;// >> 1;
	for (i = mid; i < wav_len; i++)
		wav[i] = (i + 256 - wav_len);// >> 1;
}


#if 0
static void refresh2(void) {
	int i;
	uint8_t mid = wav_len2 >> 1;
	for (i=0; i < mid; i++)
		wav2[i] = i >> 1;
	for (i = mid; i < wav_len2; i++)
		wav2[i] = (i + 256 - wav_len2) >> 1;
}
#endif

#define MELODY_FRAG_LEN 7
#ifdef TWO_NOTES
uint8_t counter_a1[MELODY_FRAG_LEN] = { Ab4,Ab4,C4,C4,Db4,Db4,C4 };
#endif
//uint8_t melody_a1[MELODY_FRAG_LEN] = { Ab4,Ab4,Eb5,Eb5,F5,F5,Eb5 };
//uint8_t melody_a2[MELODY_FRAG_LEN] = { Db5,Db5,C5,C5,Bb4,Bb4,Ab4 };
//uint8_t melody_b[MELODY_FRAG_LEN] = { Eb5,Eb5,Db5,Db5,C5,C5,Bb4 };
#if 0
uint8_t melody_a1[MELODY_FRAG_LEN] = { C4,C4,G4,G4,A4,A4,G4 };
uint8_t melody_a2[MELODY_FRAG_LEN] = { F4,F4,E4,E4,D4,D4,C4 };
uint8_t melody_b[MELODY_FRAG_LEN] = { G4,G4,F4,F4,E4,E4,D4 };
#endif

#define EIGHTH_NOTE 70
#define SIXTEENTH_NOTE (EIGHTH_NOTE >> 1)
#define RIT 20

#define MOON_LEN 27
#if 0
uint8_t moon[MOON_LEN] = { A4, F4, C4,
						   A4, F4, C4,
						   Bb4, G4, C4,
						   Bb4, G4, C4,
						   C5, A4, D5,
						   C5, A4, D5,
						   C5, A4, G4,
						   F4, F4, E4, D4, E4, F4 };
#endif
#if 0
uint8_t moon[MOON_LEN] = { Bb4, Gb4, Db4,
						   Bb4, Gb4, Db4,
						   B4, Ab4, Db4,
						   B4, Ab4, Db4,
						   Db5, Bb4, Eb5,
						   Db5, Bb4, Eb5,
						   Db5, Bb4, Ab4,
						   Gb4, Gb4, F4, Eb4, F4, Gb4 };
uint8_t moon_lens[MOON_LEN] = { 2, 2, 4,
								2, 2, 4,
								2, 2, 4,
								2, 2, 4,
								2, 2, 4,
								2, 2, 4,
								2, 2, 3,
								1, 1, 1, 1, 1, 8 };
#endif
//#define TWINKLE_LEN 42
#if 0
uint8_t twinkle[TWINKLE_LEN] = { C4,C4,G4,G4,A4,A4,G4,
								 F4,F4,E4,E4,D4,D4,C4,
								 G4,G4,F4,F4,E4,E4,D4,
								 G4,G4,F4,F4,E4,E4,D4,
								 C4,C4,G4,G4,A4,A4,G4,
								 F4,F4,E4,E4,D4,D4,C4 };
uint8_t twinkle[TWINKLE_LEN] = { F4,F4,C5,C5,D5,D5,C5,
								 Bb4,Bb4,A4,A4,G4,G4,F4,
								 C5,C5,Bb4,Bb4,A4,A4,G4,
								 C5,C5,Bb4,Bb4,A4,A4,G4,
								 F4,F4,C5,C5,D5,D5,C5,
								 Bb4,Bb4,A4,A4,G4,G4,F4 };
#endif
#if 0
uint8_t twinkle[TWINKLE_LEN] = { Gb4,Gb4,Db5,Db5,Eb5,Eb5,Db5,
								 B4,B4,Bb4,Bb4,Ab4,Ab4,Gb4,
								 Db5,Db5,B4,B4,Bb4,Bb4,Ab4,
								 Db5,Db5,B4,B4,Bb4,Bb4,Ab4,
								 Gb4,Gb4,Db5,Db5,Eb5,Eb5,Db5,
								 B4,B4,Bb4,Bb4,Ab4,Ab4,Gb4};
uint8_t twinkle_lens[TWINKLE_LEN] = { 2, 2, 2, 2, 2, 2, 4,
									  2, 2, 2, 2, 2, 2, 4,
									  2, 2, 2, 2, 2, 2, 4,
									  2, 2, 2, 2, 2, 2, 4,
									  2, 2, 2, 2, 2, 2, 4,
									  2, 2, 2, 2, 2, 2, 8};
#endif

#define ENC(n,o,l) ((n) << 4 | (o) << 3 | (l))
#define NOTE(x) ((x) >> 4)
#define OCTAVE(x) (!!((x) & 8))
#define LEN(x) ((x) & 7)

#define TWINKLE_LEN 94
uint8_t twinkle_enc[TWINKLE_LEN] = {ENC(C,0,2), ENC(C,1,2),
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


#define SET_NOTE(a,i,o,l) \
	a[i>>1] = ((o << 3) | (l - 1)) << ((i & 1) << 2)
#define GET_NOTE(a,i) \
	(a[i>>1] >> ((i & 1) << 2))
#define GET_NOTE_LEN(a,i) \
	(GET_NOTE(a,i) & 7)
#define GET_NOTE_OCTAVE(a,i) \
	((GET_NOTE(a,i) & 8) >> 3)

/* XXX Lengths need to be encoded the same way as note (there are only
       8 combos used */

/* XXX change to this:

 7 6 5 4 3 2 1 0
|-note--|o|-len-|
*/




#define GOODNIGHT_LEN 11
#if 0
uint8_t goodnight[GOODNIGHT_LEN] = { Gb4,Db4,Ab4,F4,Gb4,B3,Eb4,
									 F4,Eb4,Eb4,Db4};
uint8_t goodnight_lens[GOODNIGHT_LEN] = { 4,2,4,2,4,2,6,
											  4,4,4,4};	  
#endif
#if 0
void play_goodnight(void) {
	uint8_t i, j;
	for ( i = 0; i < GOODNIGHT_LEN; i++ ) {
		wav_len = goodnight[i];
		refresh();
		for ( j = 0; j < goodnight_lens[i]; j++)
			_delay_ms(EIGHTH_NOTE);
	}
}	
#endif
#if 0
void play_moon(void) {
	uint8_t i, j;
	for ( i = 0; i < MOON_LEN; i++ ) {
		wav_len = moon[i];
		refresh();
		for ( j = 0; j < moon_lens[i]; j++)
			_delay_ms(EIGHTH_NOTE);
		if ( i > MOON_LEN - 4 )
			for (j = 0; j < i - (MOON_LEN - 4); j++)
				_delay_ms(RIT);
	}
}	
#endif
uint8_t octave = 0;
#if 0
void play_twinkle(void) {
	uint8_t i, j, o;
	for ( i = 0; i < TWINKLE_LEN; i++ ) {
		wav_len = twinkle[i];
		refresh();
		octave = octave ? 0 : 1;
		OCR0A = (127 << octave);
		for ( o = 0; o <= octave; o++)
			for ( j = 0; j < twinkle_lens[i]; j++)
				_delay_ms(EIGHTH_NOTE);

		if ( i > TWINKLE_LEN - 7)
			for (j = 0; j < i - (TWINKLE_LEN - 7); j++)
				_delay_ms(RIT);

	}
}	
#endif
void play_twinkle(void) {
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
				//				_delay_ms(EIGHTH_NOTE>>4);
			}
			_delay_ms(EIGHTH_NOTE);
		}

		if ( i > TWINKLE_LEN - 7) {
			for (j = 0; j < i - (TWINKLE_LEN - 7); j++) {
				if (octave) {
					_delay_ms(RIT);
					_delay_ms(RIT>>2);
					_delay_ms(RIT>>3);
					//	_delay_ms(RIT>>4);
				}
				_delay_ms(RIT);
			}
		}
	}
}	

					   
#define DELAY 200
#if 0
void twinkle_a(void) {
	uint8_t i;
	for ( i = 0; i < MELODY_FRAG_LEN; i++ ) {
		wav_len = melody_a1[i];
#ifdef TWO_NOTES
		wav_len2 = counter_a1[i];
#endif
		refresh();
		_delay_ms(DELAY);
	}
	_delay_ms(DELAY);
	for ( i = 0; i < MELODY_FRAG_LEN; i++ ) {
		wav_len = melody_a2[i];
		refresh();
		_delay_ms(DELAY);
	}
	_delay_ms(DELAY);
}

void twinkle_b(void) {
	uint8_t i, j;
	for ( j = 0; j < 2; j++ ) {
		for ( i = 0; i < MELODY_FRAG_LEN; i++ ) {
			wav_len = melody_b[i];
			refresh();
			_delay_ms(DELAY);
		}
		_delay_ms(DELAY);
	}
}
#endif
#if 0
void twinkle(void) {
	twinkle_a();
	twinkle_b();
	twinkle_a();
	_delay_ms(DELAY);
	_delay_ms(DELAY);	
}
#endif

int main() {
	
	DDRB |= (1 << PB1) | (1 << PB0) | (1 << PB4) | (1 << PB3);

	// Enable 64 MHz PLL and use as source for Timer1
	PLLCSR |= 1 << PLLE;            /* Enable PLL */
	_delay_ms(100);                 /* Wait for PLL steady state */
	while ((PLLCSR | PLOCK) == 0);  /* Poll PLOCK bit */
	PLLCSR |= PCKE;                 /* Enable 64 MHz PLL for Timer 1 */
 
  // Set up Timer/Counter1 for PWM output
	//TCCR1 = 1<<PWM1A | 2<<COM1A0 | 1<<CS10; // PWM A, clear on match, 1:1 prescale
	TCCR1 = 1<<PWM1A | 2<<COM1A0 | 1<<CS10; // PWM A, clear on match, 1:1 prescale

  OCR1A = 128;                    // 50% duty at start

#ifdef TWO_SPEAKERS
  GTCCR = 1<<PWM1B | 1<<COM1B0;           // PWM B, clear on match
  OCR1B = 128;
#endif

  // Set up Timer/Counter0 for 8kHz interrupt to output samples.
  //TCCR0A = 3<<WGM00;                      // Fast PWM
  //TCCR0B = 1<<WGM02 | 2<<CS00;            // 1/8 prescale
  //TCCR0B = 1 << WGM02 | 1<<CS00;
  //TIMSK = 1<<OCIE0A;                      // Enable compare match
  //OCR0A = 124;                            // Divide by 1000
  //OCR0A = 32;
  TCCR0A = 1<<WGM01;  /* CTC mode */
  TCCR0B = 1 << CS00; /* 8 mhz no prescale */
  //OCR0A = 142;        /* divisor */
  OCR0A = 254;
  TIMSK = 1 << OCIE0A;
  
  /* set_sleep_mode(SLEEP_MODE_PWR_DOWN); */
  /* pinMode(4, OUTPUT); */
  /* pinMode(1, OUTPUT); */
  refresh();
  p = 0;
  last_p = wav_len = 255;

#ifdef TWO_SPEAKERS
  p2 = 0;
  last_p2 = wav_len2 = melody[0];

  wav_len2 = 100;
  p2 = 0;
  last_p2 = wav_len2;
#endif
  sei();
  for(;;){
	  //play_moon();
	  //play_goodnight();
	  //play_goodnight();
	  play_twinkle();
  }
  
  return 0;
}


//#define DECAY 4
#define DECAY 3
int decay = 0;

#ifdef TWO__NOTES
#define DECAY2 3
int decay2 = 0;
#endif

uint8_t low_c = 0;

// Sample interrupt
ISR(TIMER0_COMPA_vect) {
	OCR1A = wav[p];
	if ( decay == 0 )
		wav[p] = (wav[p] >> 1) + (wav[last_p] >> 1);
	last_p = p++;
	if (p == wav_len) {
		p = 0;
		if (decay++ == DECAY) decay = 0;
	}

#ifdef TWO_NOTES
	OCR1A = wav[p] + wav[p2];
	if ( decay == 0 ) {
		wav[p] = (wav[p] >> 1) + (wav[last_p] >> 1);
		wav[p2] = (wav[p2] >> 1) + (wav[last_p2] >> 1);
	}
		
	last_p = p++;
	last_p2 = p2++;
	if (p == wav_len) {
		p = 0;
		if (decay++ == DECAY) decay = 0;
	}
	if (p2 == wav_len2) p2 = 0;
#endif
}
