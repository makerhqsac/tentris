#include <SPI.h>
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>
#include <skywriter.h>

#include "config.h"
#include "shapes.h"
#include "melody.h"
Adafruit_NeoPixel strip = Adafruit_NeoPixel(BOARD_WIDTH * BOARD_HEIGHT, NEO_PIN, NEO_GRB + NEO_KHZ800);
bool saveScores = true;
byte currentShape = 0;
byte currentRotation = 0;
byte nextShapeIndex = 0;
bool runrun = true;

short yOffset = -4;
short xOffset = 0;
short lastY = -4;
short lastX = 0;

unsigned long toneStamp = millis();
unsigned short currentNote = 0;

unsigned short level = INITIAL_BLOCK_DELAY;
unsigned int score = 0;
unsigned long stamp = 0;
unsigned long timeCollided = 0;
unsigned long lastDown = 0;
unsigned long lastRotate = 0;
unsigned long lastButton[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

COLOR shapeColors[SHAPE_COUNT];

void fillBlock(byte x, byte y, COLOR color);
uint16_t xyToCell(byte x, byte y);
bool debounceButton(int pin);
void printBoardToSerial();
void animateRandom(bool firstRun);
void animateChase(bool firstRun);
void animateRain(bool firstRun);
void checkForTetris();
void handleGesture(unsigned char type);
void handleAirwheel(int speed);
void anyGesture(unsigned char dontcare);
void anyWheel(int dontcare);
void anyXYZ(unsigned int x, unsigned int y, unsigned int z);

void clearBoard();

bool hittingBottom() {
  for (int i = 3; i != 0; i--) {
    for (int j = 3; j != 0; j--) {
      if (bitRead(shapes[currentShape][currentRotation][i], j) == 1) {
        return (i + 1 + yOffset) >= BOARD_HEIGHT;
      }
    }
  }

  return false;
}
void drawNextShape() {
  Serial.print("Next shape: ");
  Serial.println(shapeNames[nextShapeIndex]);
#ifdef SHOWNEXTSHAPE
    if (bitRead(shapes[nextShapeIndex][0][y], x) == 1) {
      fillBlock((k == 0 ? lastXOffset : xOffset) + x, yOffset + i, k == 0 ? BACKGROUND_COLOR : shapeColors[nextShapeIndex]);
    }
#endif
}

void nextShape() {
  yOffset = -4;
  xOffset = BOARD_WIDTH / 2 - 1;
  currentRotation = 0;
  currentShape = nextShapeIndex;
  nextShapeIndex = (byte)random(SHAPE_COUNT);
  drawNextShape();
}

void waitForClick() {
  int choice = random(3);
#ifdef USE_ANALOG_JOY
  while (true) {
    while (digitalRead(JOY_BTN) != LOW) {
      delay(100);
    }

    delay(50);
    if (digitalRead(JOY_BTN) == LOW) {
      return;
    }
  }
#endif
#ifdef USE_SKYWRITER
  runrun = true;
  long delayGesture = millis();
  bool firstRun = true;
  while (runrun) {
    switch (choice) {
      case 0: animateRandom(firstRun); break;
      case 1: animateChase(firstRun); break;
      case 2: animateRain(firstRun); break;
      default: animateChase(firstRun); break;
    }
    Skywriter.poll();
    if (debounceButton(BUTTON_ROTATE)) {
      runrun = false;
    }

    firstRun = false;
    if (delayGesture > 0 && millis() - 5000 > delayGesture) {
      Skywriter.onGesture(anyGesture);
      Skywriter.onAirwheel(anyWheel);
      Skywriter.onXYZ(anyXYZ);
      delayGesture = 0;
    }
  }
  Skywriter.onGesture(handleGesture);
  Skywriter.onAirwheel(handleAirwheel);
#endif
#ifdef USE_BUTTONSasdf
  while (true) {
    if (debounceButton(BUTTON_ROTATE)) {
      return;
    }
    switch (choice) {
      case 0: animateRandom(); break;
      case 1: animateChase(); break;
      case 2: animateRain(); break;
      default: animateRain(); break;
    }
  }
#endif
}

COLOR randomColor() {
  COLOR c;
  c.R = (byte)random(ANIMATE_MAX_BRIGHT);
  c.G = (byte)random(ANIMATE_MAX_BRIGHT);
  c.B = (byte)random(ANIMATE_MAX_BRIGHT);
  return c;
}
void animateRandom(bool firstRun) {
  static long last = 0;
  if (abs(millis() - last) > ANIM_DELAY) {
    last = millis();
    COLOR c = randomColor();
    fillBlock(random(BOARD_WIDTH), random(BOARD_HEIGHT), c);
    strip.show();
  }
}

void animateChase(bool firstRun) {
  static long last = 0;
  static int i = 0;
  if (firstRun)
    i = 0;
  if (abs(millis() - last) > ANIM_CHASE_DELAY) {
    last = millis();
    COLOR c = randomColor();
    COLOR b = BACKGROUND_COLOR;
    strip.setPixelColor(i, c.R, c.G, c.B); // Draw new pixel
    strip.setPixelColor(i - ANIM_CHASE_LENGTH, b.R, b.G, b.B); // Erase pixel a few steps back
    strip.show();
    i++;
    if (i > BOARD_WIDTH * BOARD_HEIGHT + ANIM_CHASE_LENGTH)
      i = 0;
  }
}

void animateRain(bool firstRun) {
  static long last = 0;
  if (abs(millis() - last) > ANIM_CHASE_DELAY) {
    int chance;
    COLOR c = randomColor();
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
      for (int x = 0; x < BOARD_WIDTH; x++) {
        fillBlock(x, y, getPixel(x, y - 1));
      }
    }

    for (int i = 0; i < BOARD_WIDTH; i++) {
      chance = random(100);
      if (chance < 10) {
        fillBlock(i, 0, c);
      }
    }

    strip.show();
    last = millis();
  }
}
void saveScore() {
  if (saveScores) {
    unsigned int oldHigh;
        EEPROM.get(sizeof(int), oldHigh);
        if (score > oldHigh) {
          EEPROM.put(sizeof(int), score);
          Serial.print("New high score!! ");
          Serial.println(score);
        } else {
          Serial.print("High score remains at ");
          Serial.println(score);
        }
  }
}

void drawScore(int s) {
  int offset = 0;
  if (s > 9) {
    writeDigit(1,0);
    s = s % 10;
    offset = 5;
  }
  writeDigit(s, offset);
  strip.show();
}

void writeDigit(int digit, int offset) {
  for (int x = 0; x <= 6; x++) {
    for (int y = 0; y < 7; y++) {
      if (bitRead(numbers[digit][y], 6 - x))
        fillBlock(offset + x, y, COLOR_WHITE);
    }
  }
}

void gameOver() {
  saveScore();
  drawScore(score/LINE_SCORE_VALUE);
  Serial.println("Game over man! Game over!!");
  Serial.println(score);
  delay(2000);
  printBoardToSerial();
  waitForClick();
  clearBoard();
  score = 0;
  nextShape();
  strip.show();
}

void fillBlock(byte x, byte y, COLOR color) {
#ifdef SHOWNEXTBLOCK
#else
  if (x < BOARD_WIDTH && y < BOARD_HEIGHT) {
    strip.setPixelColor(xyToCell(x, y), color.R, color.G, color.B);
  }
#endif
}

uint16_t xyToCell(byte x, byte y) {
#ifdef WIRING_SNAKE
#ifdef TOPDOWN
  if (y % 2 == 0) {
    return y * BOARD_WIDTH + x;
  } else {
    return (y + 1) * BOARD_WIDTH - (x + 1);
  }
#else
#endif
  if (y % 2 == 0) {
    return BOARD_WIDTH * (BOARD_HEIGHT - y - 1) + x;
  } else {
    return BOARD_WIDTH * (BOARD_HEIGHT - y - 1) + BOARD_WIDTH - x - 1;
  }
#else
#ifdef TOPDOWN
  return BOARD_WIDTH * y + x;
#else
  return BOARD_WIDTH * (BOARD_HEIGHT - y - 1) + x;
#endif
#endif
}

COLOR getCurrentShapeColor() {
  return shapeColors[currentShape];
}

void gravity(bool apply) {
  static byte lastXOffset = 0;

  for (byte k = 0; k < 2; k++) {
    for (byte i = 0; i < 4; i++) {
      for (short j = 3, x = 0; j != -1; j--, x++) {
        if (bitRead(shapes[currentShape][currentRotation][i], j) == 1) {
          fillBlock((k == 0 ? lastXOffset : xOffset) + x, yOffset + i, k == 0 ? BACKGROUND_COLOR : shapeColors[currentShape]);
        }
      }
    }

    if (k == 0 && apply) {
      yOffset++;
    }
  }

  if (xOffset != lastXOffset) {
    lastXOffset = xOffset;
  }
  strip.show();
}

bool isShapeColliding() {
  short p[4];
  short shiftedUp[4];

  for (byte i = 0; i < 3; i++) {
    shiftedUp[i] = shapes[currentShape][currentRotation][i + 1];
  }
  shiftedUp[3] = 0;

  for (byte i = 0; i < 4; i++) {
    p[i] = shapes[currentShape][currentRotation][i] - (shapes[currentShape][currentRotation][i] & shiftedUp[i]);
  }

  for (byte i = 0; i < 4; i++) {
    byte x = 0;
    for (short j = 3; j != -1; j--) {
      if (bitRead(p[i], j) == 1) {
        if (getPixel(xOffset + x, max(0, yOffset + i + 1)) != BACKGROUND_COLOR) {
          if (yOffset < -1) {
            gameOver();
          }

          if (timeCollided + COLLISION_DELAY < millis()) {
            Serial.println("Shape collided");
            printBoardToSerial();
            timeCollided = 0;
            return true;

          } else {
            if (timeCollided == 0) {
              timeCollided = millis();
            }
            return false;
          }

        }
      }

      x++;
    }
  }

  return false;
}

void detectCurrentShapeCollision() {
  if (hittingBottom() || isShapeColliding()) {
    checkForTetris();
    nextShape();
  }
}

void checkForTetris() {
  bool foundScore = false;
  byte rowsPastFirstMatching = 0;
  for (byte row = 0; row < BOARD_HEIGHT; row++) {
    // Tiny optimization: no need to scan more than four rows for line clears
    // This optimization sounds wrong.
    if (foundScore && (++rowsPastFirstMatching > 4)) {
      return;
    }

    for (byte col = 0; col < BOARD_WIDTH; col++) {
      if (getPixel(col, row) == BACKGROUND_COLOR) {
        break;
      }

      // If detected full line
      if (col == (BOARD_WIDTH - 1)) {
        score += LINE_SCORE_VALUE;
        Serial.print("Score: ");
        Serial.println(score);
        foundScore = true;

        for (byte i = 0; i < BOARD_WIDTH; i++) {
          fillBlock(i, row, BACKGROUND_COLOR);
        }

        for (byte r = row; r > 1; r--) {
          for (byte c = 0; c < BOARD_WIDTH; c++) {
            swap2(c, r, c, r - 1);
            fillBlock(c, r, getPixel(c, r));
          }
        }
      }
    }
  }

  strip.show();
}

byte getShapeWidth() {
  static byte lastShape = currentShape;
  static byte lastRotation = currentRotation;
  static byte lastWidth = 0;

  if (currentShape == lastShape && lastRotation == currentRotation && lastWidth != 0) {
    // If shape hasn't changed, return cached value.
    return lastWidth;
  } else {
    lastWidth = 0;
    lastShape = currentShape;
    lastRotation = currentRotation;
  }

  for (byte i = 0; i < 4; i++) {
    for (short j = 3, x = 0; j != -1; j--, x++) {
      if (bitRead(shapes[currentShape][currentRotation][i], j) == 1) {
        if (j == 0) {
          // Found largest possible value.
          lastWidth = 4;
          return 4;
        }

        if ((x + 1) > lastWidth) {
          lastWidth = x + 1;
        }
      }
    }
  }

  return lastWidth;
}

bool canMove(bool left) {
  byte predictedShapePositions[4];
  for (byte i = 0; i < 4; i++) {
    byte shiftedPiece = (left ? shapes[currentShape][currentRotation][i] >> 1 : shapes[currentShape][currentRotation][i] << 1) & B00001111;
    predictedShapePositions[i] = shapes[currentShape][currentRotation][i] - (shiftedPiece & shapes[currentShape][currentRotation][i]);
  }

  for (byte i = 0; i < 4; i++) {
    for (short j = 3, x = 0; j != -1; j--, x++) {
      if ((yOffset + i) >= 0) {
        if ((bitRead(predictedShapePositions[i], j) == 1) && (getPixel(xOffset + x + (left ? -1 : 1), yOffset + i) != BACKGROUND_COLOR)) {
          return false;
        }
      }
    }
  }

  return true;
}
byte getNextRotation(bool reverse) {
  byte returnValue;
  if (reverse)
    returnValue = currentRotation == 0 ? shapeRotations[currentShape] - 1 : currentRotation - 1;
  else
    returnValue = (shapeRotations[currentShape] - 1) == currentRotation ? 0 : currentRotation + 1;
  Serial.print("Rotation: ");
  Serial.println(returnValue);
  return returnValue;
}

COLOR getPixel(uint16_t x, uint16_t y) {
  if (x >= 0 && x < BOARD_WIDTH && y >= 0 && y < BOARD_HEIGHT) {
  uint16_t pixel = xyToCell(x, y);
  uint32_t c = strip.getPixelColor(pixel);
  COLOR retval;
  retval.R = (uint8_t)(c >> 16);
  retval.G = (uint8_t)(c >>  8);
  retval.B = (uint8_t)c;
  return retval;
  } else {
    return COLOR {1,2,3};
    //return BACKGROUND_COLOR;
  }
}

bool canRotate(bool reverse) {
  byte nextRotation = getNextRotation(reverse);

  for (byte k = 0; k < 1; k++) {
    for (byte i = 0; i < 4; i++) {
      for (short j = 3, x = 0; j != -1; j--, x++) {
        if (bitRead(shapes[currentShape][nextRotation][i], j) == 1) {
          if ((xOffset + x) >= BOARD_WIDTH) {
            // will rotate off grid
            return false;
          }

          if (getPixel(xOffset + x, yOffset + i) != BACKGROUND_COLOR) {
            if (bitRead(shapes[currentShape][currentRotation][i], j) != 1) {
              return false;
            }
          }
        }
      }
    }
  }

  return true;
}

void rotate(bool reverse) {
  // Optimization for square
  if (shapeRotations[currentShape] == 1) {
    return;
  }

  if (canRotate(reverse)) {
    for (byte k = 0; k <= 1; k++) {
      for (byte i = 0; i < 4; i++) {
        for (short j = 3, x = 0; j != -1; j--, x++) {
          if (bitRead(shapes[currentShape][currentRotation][i], j) == 1) {
            fillBlock(xOffset + x, yOffset + i, k == 0 ? BACKGROUND_COLOR : getCurrentShapeColor());
          }
        }
      }
      if (k == 0)
        currentRotation = getNextRotation(reverse);
    }

    strip.show();
  }
}

void joystickMovement() {
  unsigned long now = millis();
  static bool hasClicked = false;
  static unsigned long lastMove = now;
#ifdef USE_ANALOG_JOY
  int joyX = analogRead(JOY_X);
  int joyY = analogRead(JOY_Y);

  // left
  if (joyX < 490 && xOffset > 0 && (now - lastMove) > (MOVE_DELAY + (joyX < 10 ? 0 : MOVE_DELAY * 5))) {
    if (canMove(true)) {
      lastMove = now;
      xOffset--;
    }
  }

  // right
  if (joyX > 520 && xOffset < (BOARD_WIDTH - getShapeWidth()) && (now - lastMove) > (MOVE_DELAY + (joyX > 1015 ? 0 : MOVE_DELAY * 5))) {
    if (canMove(false)) {
      lastMove = now;
      xOffset++;
    }
  }

  // down
  if (yOffset < lastYoffset && !(joyY > 490 && joyY < 520)) {
    return;
  }

  lastYoffset = yOffset;
  if (joyY > 1000 && lastDown - now > DOWN_DELAY) {
    stamp -= level;
    lastDown = now;
  }
#endif
#ifdef USE_BUTTONS
  // Handle buttons
  if ((now - lastMove) > MOVE_DELAY) {
    // Left
    bool leftPressed = debounceButton(BUTTON_LEFT);
    if (leftPressed && canMove(true)) {
      lastMove = now;
      xOffset--;
    }

    // Right
    bool rightPressed = debounceButton(BUTTON_RIGHT);
    if (rightPressed && canMove(false)) {
      lastMove = now;
      xOffset++;
    }
  }

  if (now - lastDown > DOWN_DELAY && debounceButton(BUTTON_DOWN)) {
    stamp -= level;
    lastDown = now;
  }

  if (now - lastRotate > ROTATE_DELAY && debounceButton(BUTTON_ROTATE)) {
    lastRotate = now;
    rotate(false);
  }

  //if (now - lastRotate > ROTATE_DELAY && debounceButton(BUTTON_ROTATE_REVERSE)) {
  //lastRotate = now;
  //rotate(true);
  //}
#endif
}

#ifdef USE_SKYWRITER
void anyGesture(unsigned char dontcare) {
  runrun = false;
}

void anyWheel(int dontcare) {
  runrun = false;
}

void anyXYZ(unsigned int x, unsigned int y, unsigned int z) {
  runrun = false;
  Skywriter.onXYZ(NULL);
}

void handleGesture(unsigned char type) {
  unsigned long now = millis();

  if (type == SW_FLICK_EAST_WEST) {
    if (canMove(true)) {
      xOffset--;
    }
  }

  if (type == SW_FLICK_WEST_EAST) {
    if (canMove(false)) {
      xOffset++;
    }
  }

  if (type == SW_FLICK_NORTH_SOUTH) {
    stamp -= level;
    lastDown = now;
  }

}

void handleAirwheel(int speed) {
  unsigned long now = millis();

  if (speed > 5 && now - lastRotate > ROTATE_DELAY) {
    lastRotate = now;
    rotate(false);

  } else if (speed < -5 && now - lastRotate > ROTATE_DELAY) {
    lastRotate = now;
    rotate(true);
  }
}
#endif

bool debounceButton(int pin) {
  unsigned long now = millis();
  if (digitalRead(pin) == LOW && abs(now - lastButton[pin]) > DEBOUNCE) {
    lastButton[pin] = now;
    Serial.print("Button: ");
    Serial.println(pin);
    return true;
  } else {
    return false;
  }
}

void printBoardToSerial() {
#ifdef DEBUG
  for (int y = 0; y < BOARD_HEIGHT; y++)  {
    for (int x = 0; x < BOARD_WIDTH; x++)  {
      if (getPixel(x,y) == BACKGROUND_COLOR) {
        Serial.print(" ");
      } else {
        Serial.print("X");
      }
    }
    Serial.println();
  }
#endif
}

void clearBoard() {
  for (byte i = 0; i < BOARD_WIDTH; i++) {
    for (byte j = 0; j < BOARD_HEIGHT; j++) {
      fillBlock(i, j, BACKGROUND_COLOR);
    }
  }
  strip.show();
}

void setup() {

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Setup start");

#ifdef USE_SD
  if (!SD.begin(SD_CS)) {
    Serial.println("SD init failed!");
    return;
  }


  if (!SD.exists(FILE_NAME)) {
    File f = SD.open(FILE_NAME, FILE_WRITE);
    f.close();
  }

  if (!SD.exists(FILE_NAME)) {
    Serial.print("Couldn't access file on SD card: ");
    Serial.println(FILE_NAME);
    saveScores = false;
  }
#endif

#ifdef USE_ANALOG_JOY
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);
  pinMode(JOY_BTN, INPUT_PULLUP);
#endif
#ifdef USE_BUTTONS
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_ROTATE, INPUT_PULLUP);
  pinMode(BUTTON_ROTATE_REVERSE, INPUT_PULLUP);
#endif

  // Clear out the EEPROM if this is our first time through. Good for keeping a realistic high score.
  unsigned int testValue;
    EEPROM.get(sizeof(int)*2, testValue);
    if (testValue != 0){
      unsigned int test = 0;
      EEPROM.put(0, test);
      EEPROM.put(sizeof(int), test);
      EEPROM.put(sizeof(int) * 2, test);
    }

    unsigned int seed = 0;
    EEPROM.get(0, seed);
    randomSeed(seed);
    seed = random(65535);
    EEPROM.put(0, seed);
  shapeColors[SHAPE_I] = SHAPE_I_COLOR;
  shapeColors[SHAPE_J] = SHAPE_J_COLOR;
  shapeColors[SHAPE_L] = SHAPE_L_COLOR;
  shapeColors[SHAPE_O] = SHAPE_O_COLOR;
  shapeColors[SHAPE_S] = SHAPE_S_COLOR;
  shapeColors[SHAPE_T] = SHAPE_T_COLOR;
  shapeColors[SHAPE_Z] = SHAPE_Z_COLOR;

  clearBoard();
  //waitForClick();
  clearBoard();
  delay(500);
  currentShape = random(SHAPE_COUNT);
  nextShapeIndex = random(SHAPE_COUNT);
  nextShape();
  Serial.print("First shape: ");
  Serial.println(shapeNames[currentShape]);
  stamp = millis();

#ifdef USE_SKYWRITER
  Skywriter.begin(SK_PIN_TRFR, SK_PIN_RESET);
  // Hack to ensure that it connects
  delay(1000);
  Skywriter.begin(SK_PIN_TRFR, SK_PIN_RESET);
  Skywriter.onGesture(handleGesture);
  Skywriter.onAirwheel(handleAirwheel);
#endif

  Serial.println("Setup end");
}

void loop() {
  unsigned long now = millis();
  unsigned int noteDuration = 1000 / pgm_read_byte_near(noteDurations + currentNote);

    if ((now - toneStamp) > noteDuration) {
      noTone(BUZZER);
    }

    if ((now - toneStamp) > (noteDuration * 1.3)) {
      toneStamp = now;
      if (++currentNote > (sizeof(melody) / 2)) {
        currentNote = 0;
      }

      tone(BUZZER, pgm_read_word_near(melody + currentNote), noteDuration);
    }
  
  if ((now - stamp) > level - min(score, 1000)) {
    stamp = millis();
    gravity(true);
  }

  if ((lastX != xOffset) || (lastY != yOffset)) {
    gravity(false);
    detectCurrentShapeCollision();
    lastX = xOffset;
    lastY = yOffset;
  }

  joystickMovement();

#ifdef USE_SKYWRITER
  Skywriter.poll();
#endif

}
