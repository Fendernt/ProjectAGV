/*
 */

#include <avr/io.h>

#define MAGNET PB7
#define Z_AS_BOTTOM PF1

int MAGNET_PRESSED = 0;

int main(void)
{
     DDRB |= (1 << MAGNET);
     DDRF |= (1 << Z_AS_BOTTOM);
     PORTF |= (1 << Z_AS_BOTTOM);
     PORTB |= (1 << MAGNET);

   while(1){
        if(!(PINF & (1 << Z_AS_BOTTOM)))
        {
            if(MAGNET_PRESSED == 0)
            {
                MAGNET_PRESSED = 1;
                PORTB &= ~(1 << MAGNET);
            }
        } else {
            MAGNET_PRESSED = 2;
        }
   }

    return 0;
}
