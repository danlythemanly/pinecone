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

#if 0
#if 1
static int8_t sin_wav[SIN_LEN] = { /* sin */
0x80, 0x86, 0x8c, 0x92, 0x98, 0x9e, 0xa4, 0xaa, 
0xb0, 0xb6, 0xbb, 0xc1, 0xc6, 0xcb, 0xd0, 0xd5, 
0xd9, 0xde, 0xe2, 0xe6, 0xe9, 0xec, 0xf0, 0xf2, 
0xf5, 0xf7, 0xf9, 0xfb, 0xfc, 0xfd, 0xfe, 0xfe, 
0xff, 0xfe, 0xfe, 0xfd, 0xfc, 0xfb, 0xf9, 0xf7, 
0xf5, 0xf2, 0xf0, 0xec, 0xe9, 0xe6, 0xe2, 0xde, 
0xd9, 0xd5, 0xd0, 0xcb, 0xc6, 0xc1, 0xbb, 0xb6, 
0xb0, 0xaa, 0xa4, 0x9e, 0x98, 0x92, 0x8c, 0x86, 
0x80, 0x79, 0x73, 0x6d, 0x67, 0x61, 0x5b, 0x55, 
0x4f, 0x49, 0x44, 0x3e, 0x39, 0x34, 0x2f, 0x2a, 
0x26, 0x21, 0x1d, 0x19, 0x16, 0x13, 0x0f, 0x0d, 
0x0a, 0x08, 0x06, 0x04, 0x03, 0x02, 0x01, 0x01, 
0x01, 0x01, 0x01, 0x02, 0x03, 0x04, 0x06, 0x08, 
0x0a, 0x0d, 0x0f, 0x13, 0x16, 0x19, 0x1d, 0x21, 
0x26, 0x2a, 0x2f, 0x34, 0x39, 0x3e, 0x44, 0x49, 
0x4f, 0x55, 0x5b, 0x61, 0x67, 0x6d, 0x73, 0x79, 
};
#else
static uint8_t sin_wav[SIN_LEN] = { /* noise */
0xbd, 0x9e, 0x17, 0xa1, 0xee, 0x2c, 0x63, 0x7a, 
0xb6, 0x25, 0xba, 0x76, 0xc8, 0x09, 0x9d, 0xc0, 
0x23, 0xbc, 0x96, 0xfc, 0xcd, 0xa0, 0x55, 0xd0, 
0x73, 0xa8, 0x49, 0x7c, 0x27, 0xde, 0xee, 0x95, 
0x48, 0x07, 0xbc, 0x9b, 0xb0, 0xf1, 0x8b, 0xcb, 
0x12, 0x14, 0x9b, 0x07, 0x4c, 0x55, 0x69, 0x01,
0x44, 0xd8, 0x3a, 0x7e, 0x46, 0x48, 0x28, 0xa8, 
0x23, 0x11, 0xc2, 0x5f, 0x4d, 0x33, 0xdf, 0xae, 
0x0e, 0x5b, 0x5d, 0x9a, 0xc5, 0x06, 0x7d, 0x3f,
0xcd, 0xe8, 0x6b, 0xb8, 0x51, 0xe4, 0x60, 0x05, 
0x25, 0x93, 0x3f, 0x0a, 0x18, 0x66, 0x6d, 0xf8, 
0xad, 0xee, 0xbf, 0x55, 0x7c, 0xd7, 0x1d, 0x40, 
0xa8, 0x26, 0xc3, 0x51, 0x09, 0x1e, 0xc5, 0xd4, 
0xe5, 0x09, 0xf9, 0xfd, 0x0f, 0xb5, 0x06, 0x3a, 
0xd0, 0x65, 0x1b, 0x13, 0x01, 0x10, 0xb2, 0x1c, 
};
#endif
#if 0
0xa8, 0x29, 0xc8, 0x5c, 0xcb, 0x65, 0x81, 0x04,
0x23, 0x1e, 0xf6, 0x28, 0x01, 0xa3, 0xa5, 0xe7,
0x25, 0xd3, 0x2f, 0xdd, 0x93, 0x51, 0xd4, 0xea,
0xb1, 0xed, 0x0f, 0xaf, 0x45, 0x8c, 0x0b, 0xe7,
0xc9, 0x38, 0xb6, 0xfa, 0x44, 0x1b, 0xd1, 0x9d,
0x3e, 0x8c, 0xd7, 0x13, 0xb2, 0xbb, 0x87, 0xed,
0xd8, 0x61, 0x21, 0x0a, 0xfe, 0x17, 0x04, 0x62,
0x8e, 0xfc, 0xbc, 0xf9, 0xfc, 0x79, 0xc6, 0x7b,
0x09, 0x8b, 0x0b, 0xca, 0xf8, 0xfa, 0x82, 0xc0,
0xf8, 0x95, 0xb2, 0x7f, 0x5a, 0x4c, 0x86, 0xb7,
0xd4, 0xb4, 0xfb, 0x57, 0xcf, 0xdb, 0x29, 0xb3,
0x57, 0xb2, 0x8d, 0x71, 0x1c, 0x9c, 0x49, 0x98,
0x21, 0x84, 0xe5, 0x46, 0x46, 0x9c, 0xf0, 0xb6,
0x22, 0xc7, 0xa7, 0xfe, 0xd2, 0x4a, 0x8c, 0x69,
0x94, 0xe0, 0x06, 0xcc, 0x1b, 0x20, 0xb1, 0xe2,
0xfb, 0x44, 0xcf, 0xd7, 0x7f, 0xa3, 0xb9, 0xbc,
0xea, 0xbb, 0xfa, 0x23, 0x6c, 0x7a, 0xc0, 0x28,
};
#endif
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


uint8_t m = 0;
uint8_t melody[48] = { C4,C4,G4,G4,A4,A4,G4,G4, 
					   F4,F4,E4,E4,D4,D4,C4,C4,
					   G4,G4,F4,F4,E4,E4,D4,D4,
					   G4,G4,F4,F4,E4,E4,D4,D4,
					   C4,C4,G4,G4,A4,A4,G4,G4, 
					   F4,F4,E4,E4,D4,D4,C4,C4 };
int	melody_len = 48;


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
uint8_t melody_a1[MELODY_FRAG_LEN] = { C4,C4,G4,G4,A4,A4,G4 };
uint8_t melody_a2[MELODY_FRAG_LEN] = { F4,F4,E4,E4,D4,D4,C4 };
uint8_t melody_b[MELODY_FRAG_LEN] = { G4,G4,F4,F4,E4,E4,D4 };

#define EIGHTH_NOTE 110

#define MOON_LEN 27
uint8_t moon[MOON_LEN] = { A4, F4, C4,
						   A4, F4, C4,
						   Bb4, G4, C4,
						   Bb4, G4, C4,
						   C5, A4, D5,
						   C5, A4, D5,
						   C5, A4, G4,
						   F4, F4, E4, D4, E4, F4 };
uint8_t moon_lens[MOON_LEN] = { 2, 2, 4,
								2, 2, 4,
								2, 2, 4,
								2, 2, 4,
								2, 2, 4,
								2, 2, 4,
								2, 2, 3,
								1, 1, 1, 1, 1, 8 };

#define TWINKLE_LEN 42
uint8_t twinkle[TWINKLE_LEN] = { C4,C4,G4,G4,A4,A4,G4,
								 F4,F4,E4,E4,D4,D4,C4,
								 G4,G4,F4,F4,E4,E4,D4,
								 G4,G4,F4,F4,E4,E4,D4,
								 C4,C4,G4,G4,A4,A4,G4,
								 F4,F4,E4,E4,D4,D4,C4 };
uint8_t twinkle_lens[TWINKLE_LEN] = { 2, 2, 2, 2, 2, 2, 4,
									  2, 2, 2, 2, 2, 2, 4,
									  2, 2, 2, 2, 2, 2, 4,
									  2, 2, 2, 2, 2, 2, 4,
									  2, 2, 2, 2, 2, 2, 4,
									  2, 2, 2, 2, 2, 2, 8};

void play_moon(void) {
	uint8_t i, j;
	for ( i = 0; i < MOON_LEN; i++ ) {
		wav_len = moon[i];
		refresh();
		for ( j = 0; j < moon_lens[i]; j++)
			_delay_ms(EIGHTH_NOTE);
	}
}	
void play_twinkle(void) {
	uint8_t i, j;
	for ( i = 0; i < TWINKLE_LEN; i++ ) {
		wav_len = twinkle[i];
		refresh();
		for ( j = 0; j < twinkle_lens[i]; j++)
			_delay_ms(EIGHTH_NOTE);
	}
}	


					   
#define DELAY 200
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
  OCR0A = 142;        /* divisor */
  TIMSK = 1 << OCIE0A;
  
  /* set_sleep_mode(SLEEP_MODE_PWR_DOWN); */
  /* pinMode(4, OUTPUT); */
  /* pinMode(1, OUTPUT); */
  refresh();
  p = 0;
  last_p = wav_len = melody[0];
#ifdef TWO_SPEAKERS
  p2 = 0;
  last_p2 = wav_len2 = melody[0];

  wav_len2 = 100;
  p2 = 0;
  last_p2 = wav_len2;
#endif
  sei();
  for(;;){
	  play_twinkle();
	  play_moon();
  }
  
  return 0;
}


#define DECAY 4
int decay = 0;

#ifdef TWO__NOTES
#define DECAY2 3
int decay2 = 0;
#endif

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
