#include <Adafruit_NeoPixel.h>

#define PIN            6
#define NUMPIXELS      2
#define BRIGHTNESS     30
#define DELAY_MS       200

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    pixels.begin();
}

void loop() {

    // all red
    for(int i=0;i<NUMPIXELS;i++) {
        pixels.setPixelColor(i, pixels.Color(255, 0, 0));
    }
    pixels.setBrightness(BRIGHTNESS);
    pixels.show();
    delay(DELAY_MS);

    // all green
    for(int i=0;i<NUMPIXELS;i++) {
        pixels.setPixelColor(i, pixels.Color(0, 255, 0));
    }
    pixels.setBrightness(BRIGHTNESS);
    pixels.show();
    delay(DELAY_MS);

    // all blue
    for(int i=0;i<NUMPIXELS;i++) {
        pixels.setPixelColor(i, pixels.Color(0, 0, 255));
    }
    pixels.setBrightness(BRIGHTNESS);
    pixels.show();
    delay(DELAY_MS);

}