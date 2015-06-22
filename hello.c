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
	DDRB |= 1 << LED;
	/* enable pull-up resistor for BUTTON */
	PORTB |= 1 << BUTTON;
	
	led_off();
	button_init(&b);

	while(1) {
		check_button(&b);
	}


#if 0
		PORTB &= ~(1 << PB0); 
		_delay_ms(1000);
		PORTB |= (1 << PB0);
		_delay_ms(1000);
#endif

	return 0;
}
