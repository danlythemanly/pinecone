#include <avr/io.h>
#include <util/delay.h>


#define LED PB0
#define BUTTON PB1
#define DEBOUNCE 100

#define led_on() do {							\
		PORTB |= (1 << PB0);					\
	} while (0)

#define led_off() do {							\
		PORTB &= ~(1 << PB0);					\
	} while (0)

struct button {
	int raw1;  /* raw input value */
	int raw2;  /* old raw value */
	int val1; /* input value after debounce */
	int val2; /* old value */
	int debounce; /* debounce "timer" */
};

int main(void) {
	int state = 0;
	int raw1 = 0;
	int raw2 = 0;
	int debounce = 0;
	int val1 = 1; 
	int val2 = 1;


	/* make LED an output */
	DDRB |= 1 << LED;
	/* enable pull-up resistor for BUTTON */
	PORTB |= 1 << BUTTON;
	
	led_off();

	while(1) {
		raw1 = PINB & (1 << BUTTON);

		if ( raw1 != raw2 )
			debounce = 0;
		
		if ( debounce++ > DEBOUNCE ) {
			val2 = val1;
			val1 = raw1;

			/* the leading edge toggles state */
			if ( !val1 && val2 ) {
				state = state ? 0 : 1;
				if ( state ) 
					led_on();
				else 
					led_off();
			} 

			debounce = 0;
		}
		raw2 = raw1;
	}


#if 0
		PORTB &= ~(1 << PB0); 
		_delay_ms(1000);
		PORTB |= (1 << PB0);
		_delay_ms(1000);
#endif

	return 0;
}
