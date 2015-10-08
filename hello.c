#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#include <util/delay.h>


#define LED PB4
#define LED2 PB2
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
static struct button b;


void button_init(struct button *b) {
	b->raw1 = 1;
	b->raw2 = 1;
	b->val1 = 1;
	b->val2 = 1;
	b->debounce = 0;
}

int check_button(struct button *b) {
	int ret = 0;

	b->raw1 = PINB & (1 << BUTTON);

	if ( b->raw1 != b->raw2 )
		b->debounce = 0;
		
	if ( b->debounce++ > DEBOUNCE ) {
		b->val2 = b->val1;
		b->val1 = b->raw1;

		/* the leading edge toggles state */
		if ( !b->val1 && b->val2 )
			ret = 1;

		b->debounce = 0;
	}
	b->raw2 = b->raw1;
	
	return ret;
}

int read_button(struct button *b) {
	b->raw1 = PINB & (1 << BUTTON);

	if ( b->raw1 != b->raw2 )
		b->debounce = 0;
		
	if ( b->debounce++ > DEBOUNCE ) {
		b->val2 = b->val1;
		b->val1 = b->raw1;

		/* the leading edge toggles state */
		//if ( !b->val1 && b->val2 )
		//	ret = 1;

		b->debounce = 0;
	}
	b->raw2 = b->raw1;
	
	return b->val1;
}


static int try_sleep(void) {
	int ret = 0;

    ADCSRA &= ~_BV(ADEN);       // ADC off
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);   

	cli();
	if ( !state ) {
		ret = 1;
		sleep_enable();
		sleep_bod_disable();
		sei();  
		sleep_cpu();    

		cli();                     // Disable interrupts
		PCMSK &= ~_BV(PCINT3);     // Turn off PB3 as interrupt pin
		sleep_disable();                        // Clear SE bit
		ADCSRA |= _BV(ADEN);                    // ADC on
		sei();                                  // Enable interrupts
    } else
		sei();
	return ret;
}

/* int try_sleep(void) { */
/* 	int ret = 0; */

/* 	set_sleep_mode(SLEEP_MODE_PWR_DOWN); */
/* 	cli(); */
/* 	if ( !state ) { */
/* 		ret = 1; */
/* 		sleep_enable(); */
/* 		sleep_bod_disable(); */
/* 		sei(); */
/* 		sleep_cpu(); */
/* 		sleep_disable(); */
/* 	} */
/* 	sei(); */
	
/* 	return ret; */
/* } */

int num_interrupts = 0;

void sleep() {

    GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
    PCMSK |= _BV(PCINT3);                   // Use PB3 as interrupt pin
    ADCSRA &= ~_BV(ADEN);                   // ADC off
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement

    sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
    sei();                                  // Enable interrupts
    sleep_cpu();                            // sleep

    cli();                                  // Disable interrupts
    //PCMSK &= ~_BV(PCINT3);                  // Turn off PB3 as interrupt pin
    sleep_disable();                        // Clear SE bit
    ADCSRA |= _BV(ADEN);                    // ADC on

    sei();                                  // Enable interrupts
} 

int main(void) {

    //GIMSK |= _BV(PCIE);         // Enable Pin Change Interrupts
    //PCMSK |= _BV(PCINT3);       // Use PB3 as interrupt pin


	/* make LED an output */
	//DDRB |= (1 << LED) | (1 << SPEAKER) | (1 << PB0) | (1 << LED2);
	/* enable pull-up resistor for BUTTON */
	//PORTB |= 1 << BUTTON;
	DDRB &= ~(1 << BUTTON);
	DDRB |= (1 << LED2) | (1 << LED);
	PORTB |= 1 << BUTTON;

	/* initialize led and button */
	//led_off();
	button_init(&b);


	while (1) {
#if 0	
		if ( check_button(&b) ) {
			state = state ? 0 : 1;
			if ( state ) {
				led_on();
			} else {
				led_off();
				try_sleep();
			}
		}
#endif	
		if ( !state ) {
			sleep();
			
			//while( read_button(&b) );
			PCMSK |= _BV(PCINT3); // ready for next interrupt now
			
		}
		
		PORTB |= (1 << LED2);
		_delay_ms(100);		
		PORTB &= ~(1 << LED2);
		_delay_ms(100);
	}

#if 0
		while ( !check_button(&b) );
		state = state ? 0 : 1;
		if ( state ) {
			led_on();
		} else {
			led_off();
			_delay_ms(1000);
			try_sleep();
		}
#endif



	_delay_ms(1000);

	return 0;
}

// Called on pin interrupt
ISR(PCINT0_vect) {
	int i;

	// We don't want other interrupts from this pin yet
    PCMSK &= ~_BV(PCINT3);  

	//while ( !check_button(&b) );
	state = state ? 0 : 1;
	if ( state ) {
		led_on();
	} else {
		led_off();
	}

#if 0
	num_interrupts++;

	for ( i = 0; i < 3; i++ ){
		PORTB |= (1 << LED2);
		_delay_ms(100);		
		PORTB &= ~(1 << LED2);
		_delay_ms(100);		
	}
	_delay_ms(1000);		
	for ( i = 0; i < num_interrupts; i++ ){
		PORTB |= (1 << LED2);
		_delay_ms(1000);		
		PORTB &= ~(1 << LED2);
		_delay_ms(1000);		
	}
	_delay_ms(1000);		
	for ( i = 0; i < 3; i++ ){
		PORTB |= (1 << LED2);
		_delay_ms(100);		
		PORTB &= ~(1 << LED2);
		_delay_ms(100);		
	}
#endif



}

