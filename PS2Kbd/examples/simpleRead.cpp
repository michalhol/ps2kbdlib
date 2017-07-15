#include <Arduino.h>
#include <PS2Kbd.h>

/*
*   clk connected to pin 18
*   dat pin connected to pin 19
*/


void setup() {
    Serial.begin(115200);

    startKBD();
}

void loop() {
    if (kbd_avail()) {
        Serial.write(kbd_read());
    }
}
