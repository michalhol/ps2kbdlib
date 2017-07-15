#include <Arduino.h>
#include <PS2Kbd.h>

#define KEYBOARD_DATA 19
#define KEYBOARD_CLK  18

PS2Kbd keyboard(KEYBOARD_DATA, KEYBOARD_CLK);

void setup() {
    Serial.begin(115200);

    keyboard.begin();
}

void loop() {
    if (keyboard.available()) {
        Serial.write(keyboard.read());
    }
}
