#include <avr/io.h>
#include "7_segment_display.h"


// 0 - 9 in binary
char numberCodes[] = {0b00000011, 0b10011111, 0b00100101,
                      0b00001101, 0b10011001, 0b01001001,
                      0b01000001, 0b00011111, 0b00000001,
                      0b00001001, 0b11111101};

void initDisplay (void)
{
    DDRH |= BV(SFTCLK) | BV(LCHCLK);
    DDRB |= BV(DATAIN);

    disablePin(SFTPORT, SFTCLK);
    disablePin(LTCHPORT, LCHCLK);
}

#define DATAIN PB4 //pin 10 (detain)
#define SFTCLK PH6 //pin 9 (shiftclk)
#define LCHCLK PH5 //pin 8 (latchclk)


void send_data(char data)
{
    for(int i = 0; i < 8; i++)
    {
        if((data & (1<<i))){
            setPin(DataPORT, DATAIN);
        } else {
            disablePin(DataPORT, DATAIN);
        }

        setPin(SFTPORT, SFTCLK);
        disablePin(SFTPORT, SFTCLK);
    }
}

void send_enable(int num)
{
    send_data(1<<(3+num));
}

void setLatch(void){
    setPin(LTCHPORT, LCHCLK);
    disablePin(LTCHPORT, LCHCLK);
}

void display(int data)
{
    int pos = 0;
    int negative = 0;

    if(data == 0){
        send_data(numberCodes[0]);
        send_enable(1);
        setLatch();
        return;
    }

    if(data < 0 && data > -1000){
        data *= -1;
        negative = 1;
    }

    while(data > 0){
        send_data(numberCodes[data%10]);
        data /= 10;
        send_enable(++pos);
        setLatch();
    }

    if(negative){
        send_data(numberCodes[10]);
        send_enable(++pos);
        setLatch();
    }
}
