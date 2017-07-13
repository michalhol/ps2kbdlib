#pragma once

#include <cstdint>

#define L_SHIFT 1
#define R_SHIFT 2
#define L_ALT 4
#define R_ALT 8
#define L_CTRL 16
#define R_CTRL 32
#define SHIFT 3
#define ALT 12
#define CTRL 48

class PS2Kbd {
    private:
        int clkPin;
        int dataPin;
        volatile uint16_t shift;
        volatile uint8_t modifs;
        bool cpslk;
        bool scrlk;
        bool numlk;
        bool dirOUT;
        uint8_t kstate;
        uint8_t cnt;
        uint8_t rc;
        const uint8_t CHARS;
        volatile unsigned char buffer[256];
        volatile uint8_t from;
        volatile uint8_t to;
        volatile bool ACK;
        bool updLEDs;

        static const char chrsNS[];
        static const char chrsSH[];

        void waitACK();
        void tryUpdateLEDs();
        void bufwchr(char);
        void setLeds(uint8_t);
        static PS2Kbd* keyboard0;
        static PS2Kbd* keyboard1;
        static PS2Kbd* keyboard2;
        static PS2Kbd* keyboard3;
        static PS2Kbd* keyboard4;
        static PS2Kbd* keyboard5;
        static PS2Kbd* keyboard6;
        static PS2Kbd* keyboard7;
        void send(uint8_t);
        static void kbdInterrupt0();
        static void kbdInterrupt1();
        static void kbdInterrupt2();
        static void kbdInterrupt3();
        static void kbdInterrupt4();
        static void kbdInterrupt5();
        static void kbdInterrupt6();
        static void kbdInterrupt7();
    public:
        void interruptHandler();
        PS2Kbd(int, int);
        ~PS2Kbd();
        void begin();
        unsigned char read();
        uint8_t available();
        uint8_t getModifiers();
};