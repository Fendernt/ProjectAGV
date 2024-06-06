
#include <avr/io.h>
#include <avr/interrupt.h>

ISR(TIMER0_OVF_vect)
{
    PORTB &= ~(1<<PB6);
    int x = 1;

    if(x == 1)
    {
        OCR0A++;
        if(OCR0A > 255)
        {
            x = 0;
        }
    }
    else if(x == 0)
    {
        OCR0A--;
        if(OCR0A < 1)
        {
            x = 1;
        }
    }

}

ISR(TIMER0_COMPA_vect)
{
    PORTB |= (1<<PB7);
}


void init_led(void)
{
    DDRB |= (1<<PB6);
    PORTB &= ~(1<<PB6);
}

void init_timer(void)
{
    TCCR0A = 0;
    TCCR0B |= (1<<CS02) | (0<<CS01) | (1<<CS00);

}

void init_interrupt(void)
{
    TIMSK0 |= (1<<TOIE0) | (1<<OCIE0A);
    OCR0A = 1;
}

int main(void)
{

    init_led();
    init_timer();
    init_interrupt();
    sei();

    while(1)
    ;

    return 0;
}
