#include <Arduino.h>
#include <PS2Kbd.h>

#define KEYBOARD_DATA 19
#define KEYBOARD_CLK  18

PS2Kbd keyboard(KEYBOARD_DATA, KEYBOARD_CLK);
/*
* You can find codes for some "special" keys in keyCodes.txt.
*
* And be aware of buffer overflow - when it does, all keys you haven't read yet will be lost
* Current size of the buffer is 256 keys.
* If you do want to change it's size, open PS2Kbd.h (it is located in the <path-to-library>/PS2Kbd/src/)
* and on line 44 replace 256 with whatever you want (it just has to be bigger number than 0).
*/

void setup() {
    Serial.begin(115200);

    keyboard.begin();
}

void loop() {
    if (keyboard.available()) {
        Serial.write(keyboard.read());
    }
}
