#include <Adafruit_NeoPixel.h>
#include <Wire.h>

#define NEO_PIN 7
#define CLEAR_PIN 2
#define RESET_PIN 4

Adafruit_NeoPixel Neopixel = Adafruit_NeoPixel(200, NEO_PIN, NEO_GRB + NEO_KHZ800);

unsigned long brightness = 150;
unsigned char color = 0;

// Include string names of gestures/touches for testing
//#define SKYWRITER_INC_DEBUG_STRINGS
#include "skywriter.h"

unsigned int max_x, max_y, max_z;
unsigned int min_x, min_y, min_z;

void setup() {

  pinMode(CLEAR_PIN, INPUT_PULLUP);
  pinMode(RESET_PIN, INPUT_PULLUP);

  delay(1000);

  Neopixel.begin();
  Neopixel.setBrightness(brightness);
  Neopixel.show(); // Initialize all pixels to 'off'

  delay(1000);

  Skywriter.begin(6, 5);
  Skywriter.onXYZ(xyz);
  delay(1000);
  Skywriter.begin(6, 5);
  Skywriter.onXYZ(xyz);
}

void loop() {
  Skywriter.poll();

  if (digitalRead(CLEAR_PIN) == LOW) {
    Neopixel.clear();
  }

  if (digitalRead(RESET_PIN) == LOW) {
    Skywriter.begin(6, 5);
    Skywriter.onXYZ(xyz);
  }

  Neopixel.show();
}

void xyz(unsigned int x, unsigned int y, unsigned int z){
  if (x < min_x) min_x = x;
  if (y < min_y) min_y = y;
  if (z < min_z) min_z = z;
  if (x > max_x) max_x = x;
  if (y > max_y) max_y = y;
  if (z > max_z) max_z = z;

  unsigned char pixel_x = map(x, min_x, max_x, 0, 9);
  unsigned char pixel_y = map(y, min_y, max_y, 0, 19);
  uint32_t color        = Wheel(map(z, min_z, max_z, 0, 255));

  Neopixel.setPixelColor(pixel_x + (pixel_y*10), color);
}

void airwheel(int delta){
  Serial.println("Got airwheel ");
  Serial.print(delta);
  Serial.print('\n');

  brightness += (delta/100.0);
  Neopixel.setBrightness(brightness % 255);
}

uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return Neopixel.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
    return Neopixel.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return Neopixel.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

