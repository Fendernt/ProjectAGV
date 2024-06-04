#include <avr/io.h>
#include <util/delay.h>
#include "7_segment_display_library_ruben.h"

#define Shiftclk PH4
#define Latchclk PG5
#define SDI PH5

void init(void) {
    // Initialiseer de pinnen voor datain, shiftclk en latchclk als output
    DDRG |= (1 << Latchclk);
    DDRH |= (1 << Shiftclk);
    DDRH |= (1 << SDI);

    // Maak shiftclk en latchclk laag
    PORTH &= ~(1 << Shiftclk);
    PORTG &= ~(1 << Latchclk);
}

void send_data(int data) {
    // data van karakter
    for (int i = 7; i >= 0; i--) {
        if ((data & (1 << i)) == 0) {
            PORTH &= ~(1 << SDI);
        } else {
            PORTH |= (1 << SDI);
        }

        PORTH ^= (1 << Shiftclk);
        PORTH ^= (1 << Shiftclk);
    }
}

void send_enable(int data) {
    send_data(data);
}

void display(int data, int disp) {
    send_data(data);
    send_enable(disp);

    PORTG ^= (1 << Latchclk);
    PORTG ^= (1 << Latchclk);
}

int main(void) {
    init();
    while (1) {
        for (int digit = 0; digit <= 3; digit++) {
            int pattern;
            int place;
            switch (digit) {
            case 0:
                pattern = disp_A;
                place = 1; //Links
                break;
            case 1:
                pattern = disp_B;
                place = 2; //Midden-links
                break;
            case 2:
                pattern = disp_C;
                place = 4; //Midden-rechts
                break;
            case 3:
                pattern = disp_D;
                place = 8; //Rechts
                break;
            default:
                pattern = disp_clear;
            }
            display(pattern, place);
            //_delay_ms(100);
        }
    }
}
