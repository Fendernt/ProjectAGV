void init_Leds(void){
    PORTL = 0xff;
    PORTB |= (1 << PB2) | (1 << PB3);
}

void LedTreeIndictorLeft(int x){
    if(x){
        PORTB |= (1 << TreeIndicatedLEDLeft);
    }
    else{
        PORTB &= ~ (1 << TreeIndicatedLEDLeft);
    }
}
void LedTreeIndictorLeft(int x){
    if(x){
        PORTB |= (1 << TreeIndicatedLEDRight);
    }
    else{
        PORTB &= ~ (1 << TreeIndicatedLEDRight);
    }
}

void LedTurnSignalLeft(int x){
    if(x){
        PORTL |= (1 << TurnSignalLEDLeft);
    }
    else{
        PORTL &= ~ (1 << TurnSignalLEDLeft);
    }
}
void LedTurnSignalRight(int x){
    if(x){
        PORTL |= (1 << TurnSignalLEDRight);
    }
    else{
        PORTL &= ~(1 << TurnSignalLEDRight);
    }
}

void LedBreakLightLeft(int x){
    if(x){
        PORTL |= (1 << BreaklightsLeft);
    }
    else{
        PORTL &= ~(1 << BreaklightsLeft);
    }
}
void LedBreakLightRight(int x){
    if(x){
        PORTL |= (1 << BreaklightsRight);
    }
    else{
        PORTL &= ~(1 << BreaklightsRight);
    }
}

void LedHeadlightLeft(int x){
    if(x){
        PORTL |= (1 << HeadlightsLeft);
    }
    else{
        PORTL &= ~(1 << HeadlightsLeft);
    }
}
void LedHeadlightRight(int x){
    if(x){
        PORTL |= (1 << HeadlightsRight);
    }
    else{
        PORTL &= ~(1 << HeadlightsRight);
    }
}

void LedNoodstopFront(int x){
    if(x){
        PORTL |= (1 << NoodstopLEDFront);
    }
    else{
        PORTL &= ~(1 << NoodstopLEDFront);
    }
}
void LedNoodstopBack(int x){
    if(x){
        PORTL |= (1 << NoodstopLEDBack);
    }
    else{
        PORTL &= ~(1 << NoodstopLEDBack);
    }
}