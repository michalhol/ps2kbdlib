#include <Arduino.h>
//http://retired.beyondlogic.org/keyboard/keybrd.htm

volatile uint16_t shift=1<<11;
#define L_SHIFT 1
#define R_SHIFT 2
#define L_ALT 4
#define R_ALT 8
#define L_CTRL 16
#define R_CTRL 32
#define SHIFT 3
#define ALT 12
#define CTRL 48
volatile uint8_t modifs=0;
bool cpslk=false;
bool scrlk=false;
bool numlk=false;
bool dirOUT=false;
uint8_t kstate=0;
uint8_t cnt=0;
uint8_t rc=0;
const uint8_t CHARS=0x60;
volatile unsigned char buffer[256];
volatile uint8_t from=0;
volatile uint8_t to=0;
const char chrsNS[]={
    0,249, 0  ,245,243,241,242,252,0,250 ,248,246,244 ,'\t','`',0,
    0, 0 , 0  , 0 , 0 ,'q','1', 0 ,0, 0 ,'z' ,'s','a','w' ,'2' ,0,
    0,'c','x' ,'d','e','4','3', 0 ,0,' ','v' ,'f','t','r' ,'5' ,0,
    0,'n','b' ,'h','g','y','6', 0 ,0, 0 ,'m' ,'j','u','7' ,'8' ,0,
    0,',','k' ,'i','o','0','9', 0 ,0,'.','/' ,'l',';','p' ,'-' ,0,
    0, 0 ,'\'', 0 ,'[','=', 0 , 0 ,0, 0 ,'\n',']', 0 ,'\\', 0  ,0};
    
const char chrsSH[]={
    0,249, 0  ,245,243,241,242,252,0,250,248 ,246,244,'\t','~',0,
    0, 0 , 0  , 0 , 0 ,'Q','!', 0 ,0, 0 ,'Z' ,'S','A','W' ,'@',0,
    0,'C','X' ,'D','E','$','#', 0 ,0,' ','V' ,'F','T','R' ,'%',0,
    0,'N','B' ,'H','G','Y','^', 0 ,0, 0 ,'M' ,'J','U','&' ,'*',0,
    0,'<','K' ,'I','O',')','(', 0 ,0,'>','?' ,'L',':','P' ,'_',0,
    0, 0 ,'\"', 0 ,'{','+', 0 , 0 ,0, 0 ,'\n','}', 0 ,'|' , 0 ,0};
volatile bool ACK=false;
uint8_t kbd_getModifiers(){return modifs;}
void send(uint8_t x){
    bool d=true;
    dirOUT=true;
    for(uint8_t i=0;i<8;i++)
        if((x&(1<<i))!=0)
            d=!d;
    uint16_t shift=x|(0x200)|(0x100*d);
    Serial.println(shift,2);
    digitalWrite(18,LOW);
    delayMicroseconds(60);
    digitalWrite(19,LOW);
    delayMicroseconds(1);
    digitalWrite(18,HIGH);
    for(uint8_t i=0;i<10;i++){
        while(digitalRead(18));
        while(!digitalRead(18));
        digitalWrite(19,shift&1);
        shift>>=1;
    }
    digitalWrite(19,HIGH);
    while(digitalRead(18));
    while(!digitalRead(18));
    dirOUT=false;
}
uint8_t kbd_avail(){
    return to-from;
}
unsigned char kbd_read(){
    if(from==to)return '\0';
    return buffer[from++];
}
void waitACK(){
    while(!ACK);
    ACK=false;
}
bool updLEDs=false;
void kbd_tryUpdateLEDs(){
    if(!updLEDs)return;
    updLEDs=false;
    kstate=0;
    send(0xed);
    delay(100);
    send(scrlk|(numlk<<1)|(cpslk<<2));
}
void bufwchr(char x){
    if(to+1==from)return;
    buffer[to++]=x;
}
void sleds(uint8_t d){
    send(0xed);
    send(d&7);
}
void INT(){
    if(dirOUT)return;
    shift>>=1;
    shift|=(digitalRead(19)<<10);
    if(++rc==11){
        if((shift&0x401)==0x400){
            uint8_t x=(shift>>1)&0xff;
            switch(kstate){
                case 0:
                case 1:
                    if(x==0xfa){
                        ACK=true;
                    }else if(x==0x76){
                        bufwchr('\033');
                    }else if(x==0xf0){
                        kstate=kstate+1;
                        break;
                    }else if(x==0xe0)//EXTENDED
                        kstate=kstate+2;//TEMPORARY
                    else if(x==0x12){//SHIFT
                        if(kstate==0)
                            modifs|=L_SHIFT;
                        else modifs&=~L_SHIFT;
                    }else if(kstate==1)
                        kstate=0;
                    else if(x==0x66){//BCKSPC
                        if(kstate==0)
                            bufwchr('\b');
                    }else if(x==0xe1){//PSBRK
                        kstate=4;
                        cnt=7;
                    }else if(x==0x58){
                        cpslk=!cpslk;
                        updLEDs=true;
                    }else if(x==0x77){
                        numlk=!numlk;
                        updLEDs=true;
                    }else if(x==0x7E){
                        scrlk=!scrlk;
                        updLEDs=true;
                    }else if(x<CHARS){
                        if((modifs&3)==cpslk){
                            if(chrsSH[x]!=0)
                            bufwchr(chrsSH[x]);
                        }else if(chrsNS[x]!=0){
                            bufwchr(chrsNS[x]);
                        }
                    }
                    break;
                case 2:
                case 3:
                    if(x==12){
                        cnt=2;
                        kstate+=2;
                    }else kstate=0;
                    break;
                case 4:
                case 5:
                    cnt--;
                    if(cnt==0)
                        kstate=0;
                    break;
            }
        }
        shift=0;
        rc=0;
    }
}
void startKBD(){
    pinMode(18,OUTPUT_OPEN_DRAIN);
    pinMode(19,OUTPUT_OPEN_DRAIN);
    digitalWrite(18,true);
    digitalWrite(19,true);
    attachInterrupt(digitalPinToInterrupt(18),INT,FALLING);
}
