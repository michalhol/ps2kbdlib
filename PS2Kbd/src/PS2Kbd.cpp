#include <Arduino.h>
#include <cstdint>
#include "PS2Kbd.h"

//http://retired.beyondlogic.org/keyboard/keybrd.htm


PS2Kbd* PS2Kbd::keyboard0;
PS2Kbd* PS2Kbd::keyboard1;
PS2Kbd* PS2Kbd::keyboard2;
PS2Kbd* PS2Kbd::keyboard3;
PS2Kbd* PS2Kbd::keyboard4;
PS2Kbd* PS2Kbd::keyboard5;
PS2Kbd* PS2Kbd::keyboard6;
PS2Kbd* PS2Kbd::keyboard7;
/*
volatile uint16_t PS2Kbd::shift;
volatile uint8_t PS2Kbd::modifs;
bool PS2Kbd::cpslk;
bool PS2Kbd::scrlk;
bool PS2Kbd::numlk;
bool PS2Kbd::dirOUT;
uint8_t PS2Kbd::kstate;
uint8_t PS2Kbd::cnt;
uint8_t PS2Kbd::rc;
const uint8_t PS2Kbd::CHARS;
volatile unsigned char PS2Kbd::buffer;
volatile uint8_t PS2Kbd::from;
volatile uint8_t PS2Kbd::to;
volatile bool PS2Kbd::ACK;
bool PS2Kbd::updLEDs;
*/
const char PS2Kbd::chrsNS[]={
    0,249, 0  ,245,243,241,242,252,0,250 ,248,246,244 ,'\t','`',0,
    0, 0 , 0  , 0 , 0 ,'q','1', 0 ,0, 0 ,'z' ,'s','a','w' ,'2' ,0,
    0,'c','x' ,'d','e','4','3', 0 ,0,' ','v' ,'f','t','r' ,'5' ,0,
    0,'n','b' ,'h','g','y','6', 0 ,0, 0 ,'m' ,'j','u','7' ,'8' ,0,
    0,',','k' ,'i','o','0','9', 0 ,0,'.','/' ,'l',';','p' ,'-' ,0,
    0, 0 ,'\'', 0 ,'[','=', 0 , 0 ,0, 0 ,'\n',']', 0 ,'\\', 0  ,0};
    
const char PS2Kbd::chrsSH[]={
    0,249, 0  ,245,243,241,242,252,0,250,248 ,246,244,'\t','~',0,
    0, 0 , 0  , 0 , 0 ,'Q','!', 0 ,0, 0 ,'Z' ,'S','A','W' ,'@',0,
    0,'C','X' ,'D','E','$','#', 0 ,0,' ','V' ,'F','T','R' ,'%',0,
    0,'N','B' ,'H','G','Y','^', 0 ,0, 0 ,'M' ,'J','U','&' ,'*',0,
    0,'<','K' ,'I','O',')','(', 0 ,0,'>','?' ,'L',':','P' ,'_',0,
    0, 0 ,'\"', 0 ,'{','+', 0 , 0 ,0, 0 ,'\n','}', 0 ,'|' , 0 ,0};



uint8_t PS2Kbd::getModifiers() {
    return modifs;
}

void PS2Kbd::send(uint8_t x) {
    bool d=true;
    dirOUT=true;
    for(uint8_t i=0;i<8;i++) {
        if((x&(1<<i))!=0) {
            d=!d;
        }
    }
    uint16_t shift=x|(0x200)|(0x100*d);
    Serial.println(shift,2);
    digitalWrite(clkPin,LOW);
    delayMicroseconds(60);
    digitalWrite(dataPin,LOW);
    delayMicroseconds(1);
    digitalWrite(clkPin,HIGH);
    for(uint8_t i=0;i<10;i++) {
        while(digitalRead(clkPin));
        while(!digitalRead(clkPin));
        digitalWrite(dataPin,shift&1);
        shift>>=1;
    }
    digitalWrite(dataPin,HIGH);
    while(digitalRead(clkPin));
    while(!digitalRead(clkPin));
    dirOUT=false;
}

uint8_t PS2Kbd::available() {
    return to-from;
}

unsigned char PS2Kbd::read() {
    if(from==to)return '\0';
    return buffer[from++];
}

void PS2Kbd::waitACK() {
    while(!ACK);
    ACK=false;
}


void PS2Kbd::tryUpdateLEDs() {
    if(!updLEDs)return;
    updLEDs=false;
    kstate=0;
    send(0xed);
    delay(100);
    send(scrlk|(numlk<<1)|(cpslk<<2));
}

//buffer write char
void PS2Kbd::bufwchr(char x) {
    if(to+1==from)return;
    buffer[to++]=x;
}

void PS2Kbd::setLeds(uint8_t d) {
    send(0xed);
    send(d&7);
}

void PS2Kbd::interruptHandler() {
    if(dirOUT)return;
    shift>>=1;
    shift|=(digitalRead(dataPin)<<10);
    if(++rc==11) {
        if((shift&0x401)==0x400) {
            uint8_t x=(shift>>1)&0xff;
            switch(kstate) {
                case 0:
                case 1:
                    if(x==0xfa) {
                        ACK=true;
                    }else if(x==0x76) {
                        bufwchr('\033');
                    }else if(x==0xf0) {
                        kstate=kstate+1;
                        break;
                    }else if(x==0xe0)//EXTENDED
                        kstate=kstate+2;//TEMPORARY
                    else if(x==0x12) {//SHIFT
                        if(kstate==0)
                            modifs|=L_SHIFT;
                        else modifs&=~L_SHIFT;
                    }else if(kstate==1)
                        kstate=0;
                    else if(x==0x66) {//BCKSPC
                        if(kstate==0)
                            bufwchr('\b');
                    }else if(x==0xe1) {//PSBRK
                        kstate=4;
                        cnt=7;
                    }else if(x==0x58) {
                        cpslk=!cpslk;
                        updLEDs=true;
                    }else if(x==0x77) {
                        numlk=!numlk;
                        updLEDs=true;
                    }else if(x==0x7E) {
                        scrlk=!scrlk;
                        updLEDs=true;
                    }else if(x<CHARS) {
                        if((modifs&3)!=cpslk) {
                            if(chrsSH[x]!=0)
                            bufwchr(chrsSH[x]);
                        }else if(chrsNS[x]!=0) {
                            bufwchr(chrsNS[x]);
                        }
                    }
                    break;
                case 2:
                case 3:
                    if(x==12) {
                        cnt=2;
                        kstate+=2;
                    }else kstate=0;
                    break;
                case 4:
                case 5:
                    cnt--;
                    if(cnt==0) {
                        kstate=0;
                    }
                    break;
            }
        }
        shift=0;
        rc=0;
    }
}

void PS2Kbd::begin() {
    Serial.println("begin");
    pinMode(dataPin,OUTPUT_OPEN_DRAIN);
    pinMode(clkPin,OUTPUT_OPEN_DRAIN);
    digitalWrite(dataPin,true);
    digitalWrite(clkPin,true);
    if (keyboard0==nullptr) {
        keyboard0 = this;
        attachInterrupt(digitalPinToInterrupt(clkPin), kbdInterrupt0, FALLING);
        Serial.println("intset");
    }
    else if (keyboard1==nullptr) {
        keyboard1 = this;
        attachInterrupt(digitalPinToInterrupt(clkPin), kbdInterrupt1, FALLING);
    }
    else if (keyboard2==nullptr) {
        keyboard2 = this;
        attachInterrupt(digitalPinToInterrupt(clkPin), kbdInterrupt2, FALLING);
    }
    else if (keyboard3==nullptr) {
        keyboard3 = this;
        attachInterrupt(digitalPinToInterrupt(clkPin), kbdInterrupt3, FALLING);
    }
    else if (keyboard4==nullptr) {
        keyboard4 = this;
        attachInterrupt(digitalPinToInterrupt(clkPin), kbdInterrupt4, FALLING);
    }
    else if (keyboard5==nullptr) {
        keyboard5 = this;
        attachInterrupt(digitalPinToInterrupt(clkPin), kbdInterrupt5, FALLING);
    }
    else if (keyboard6==nullptr) {
        keyboard6 = this;
        attachInterrupt(digitalPinToInterrupt(clkPin), kbdInterrupt6, FALLING);
    }
    else if (keyboard7==nullptr) {
        keyboard7 = this;
        attachInterrupt(digitalPinToInterrupt(clkPin), kbdInterrupt7, FALLING);
    }
}

PS2Kbd::PS2Kbd(int dataPin, int clkPin)
    :dataPin(dataPin),
    clkPin(clkPin),
    shift(0),
    modifs(0),
    cpslk(false),
    scrlk(false),
    numlk(false),
    dirOUT(false),
    kstate(0),
    cnt(0),
    rc(0),
    CHARS(0x60),
    from(0),
    to(0),
    ACK(false),
    updLEDs(false)

{}

void PS2Kbd::kbdInterrupt0() {
    keyboard0->interruptHandler();
}
void PS2Kbd::kbdInterrupt1() {
    keyboard1->interruptHandler();
}
void PS2Kbd::kbdInterrupt2() {
    keyboard2->interruptHandler();
}
void PS2Kbd::kbdInterrupt3() {
    keyboard3->interruptHandler();
}
void PS2Kbd::kbdInterrupt4() {
   keyboard4->interruptHandler();
}
void PS2Kbd::kbdInterrupt5() {
    keyboard5->interruptHandler();
}
void PS2Kbd::kbdInterrupt6() {
    keyboard6->interruptHandler();
}
void PS2Kbd::kbdInterrupt7() {
    keyboard7->interruptHandler();
}

PS2Kbd::~PS2Kbd() {
    detachInterrupt(clkPin);
}
