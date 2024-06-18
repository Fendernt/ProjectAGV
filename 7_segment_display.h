#define DATAIN PB4 //pin 10 (detain)
#define SFTCLK PH6 //pin 9 (shiftclk)
#define LCHCLK PH5 //pin 8 (latchclk)

#define DataDDR DDRB
#define DataPORT PORTB

#define SFTDDR DDRH
#define SFTPORT PORTH

#define LTCHDDR DDRH
#define LTCHPORT PORTH

//{ Functions
#define BV(pin) (1 << pin)
#define setPin(port, pin) (port |= BV(pin))
#define disablePin(port, pin) (port &= ~BV(pin))
#define togglePin(port, pin) (port ^= BV(pin))

void initDisplay (void);
void display(int data);
