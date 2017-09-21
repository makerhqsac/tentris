#include <SPI.h>
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>

#include "config.h"
#include "shapes.h"
#include "melody.h"
Adafruit_NeoPixel strip = Adafruit_NeoPixel(BOARD_WIDTH * BOARD_HEIGHT, NEO_PIN, NEO_GRB + NEO_KHZ800);
bool saveScores = true;
byte currentShape = 0;
byte currentRotation = 0;
byte nextShapeIndex = 0;

short yOffset = -4;
short xOffset = 0;
short lastY = -4;
short lastX = 0;

unsigned long toneStamp = millis();
unsigned short currentNote = 0;

unsigned short level = INITIAL_BLOCK_SPEED;
unsigned int score = 0;
unsigned long stamp = 0;
unsigned long lastDown = 0;
unsigned long lastRotate = 0;
unsigned long lastButton[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

COLOR grid[BOARD_WIDTH][BOARD_HEIGHT];
COLOR shapeColors[SHAPE_COUNT];

void fillBlock(byte x, byte y, COLOR color);
bool debounceButton(int pin);
void printBoardToSerial();
void animRandom();
void checkForTetris();

bool hittingBottom() {
  for (int i = 3; i != 0; i--) {
    for (int j = 3; j != 0; j--) {
      if (bitRead(shapes[currentShape][currentRotation][i], j) == 1) {
        return (i + 1 + yOffset) >= BOARD_HEIGHT;
      }
    }
  }
}
void drawNextShape() {
  Serial.print("Next shape: ");
  Serial.println(shapeNames[nextShapeIndex]);
  return;
  /*tft.fillRect(NEXTSHAPE_X, NEXTSHAPE_Y, 30, 30, BACKGROUND_COLOR);
  for (byte i = 0; i < 4; i++) {
    for (short j = 3, x = 0; j != -1; j--, x++) {
      if (bitRead(shapes[nextShapeIndex][0][i], j) == 1) {
        //tft.fillRect(NEXTSHAPE_X + (x * BLOCK_SIZE), NEXTSHAPE_Y + (i * BLOCK_SIZE), BLOCK_SIZE - 1, BLOCK_SIZE - 1, shapeColors[nextShapeIndex]);
      }
    }
  }*/
}

void nextShape() {
  yOffset = -4;
  xOffset = BOARD_WIDTH/2-2;
  currentRotation = 0;
  currentShape = nextShapeIndex;
  nextShapeIndex = random(SHAPE_COUNT);
  drawNextShape();
}

void waitForClick() {

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
#else
  while (true) {
    if (debounceButton(BUTTON_ROTATE)) {
      return;
    }
    animRandom();
  }
#endif
}

void animRandom() {
  static long last = 0;
  if (abs(millis() - last) > 100) {
    last = millis();
    COLOR c;
    c.R = random(255);
    c.G = random(255);
    c.B = random(255);
    fillBlock(random(BOARD_WIDTH), random(BOARD_HEIGHT), c);
    strip.show();
  }
}

void saveScore() {
  if (!saveScores) {
    return;
  }

  // TODO: Do something
}
void gameOver() {
  saveScore();
  Serial.println("Game over man! Game over!!");
  printBoardToSerial();
  waitForClick();

  for (byte i = 0; i < BOARD_WIDTH; i++) {
    for (byte j = 0; j < BOARD_HEIGHT; j++) {
      fillBlock(i, j, BACKGROUND_COLOR);
    }
  }

  score = 0;
  nextShape();
  strip.show();
}

void fillBlock(byte x, byte y, COLOR color) {
  grid[x][y] = color;
#ifdef TOPDOWN
  strip.setPixelColor(BOARD_WIDTH * y + x, color.R, color.G, color.B);
#else
  strip.setPixelColor(BOARD_WIDTH * (BOARD_HEIGHT - y - 1) + x, color.R, color.G, color.B);
  /*if (x >= BOARD_WIDTH || x < 0) {
    Serial.print("X exceeded width ");
    Serial.print(x);
    Serial.print(",");
    Serial.println(y);
  }
  if (y >= BOARD_HEIGHT || y < 0) {
    Serial.print("Y exceeded height ");
    Serial.print(x);
    Serial.print(",");
    Serial.println(y);
  }*/
#endif
}

COLOR getCurrentShapeColor() {
  return shapeColors[currentShape];
}

void gravity(bool apply) {
  static byte lastXoffset = 0;

  for (byte k = 0; k < 2; k++) {
    for (byte i = 0; i < 4; i++) {
      for (short j = 3, x = 0; j != -1; j--, x++) {
        if (bitRead(shapes[currentShape][currentRotation][i], j) == 1) {
          fillBlock((k == 0 ? lastXoffset : xOffset) + x, yOffset + i, k == 0 ? BACKGROUND_COLOR : shapeColors[currentShape]);
        }
      }
    }

    if (k == 0 && apply) {
      yOffset++;
    }
  }

  if (xOffset != lastXoffset) {
    lastXoffset = xOffset;
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
        if (grid[xOffset + x][max(0, yOffset + i + 1)] != BACKGROUND_COLOR) {
          if (yOffset < -1) {
            gameOver();
          }

          Serial.println("Shape collided");
          printBoardToSerial();
          return true;
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
    if (foundScore && (++rowsPastFirstMatching > 4)) {
      return;
    }

    for (byte col = 0; col < BOARD_WIDTH; col++) {
      if (grid[col][row] == BACKGROUND_COLOR) {
        break;
      }

      // If detected full line
      if (col == (BOARD_WIDTH - 1)) {
        score += LINE_SCORE_VALUE;
        foundScore = true;

        for (byte i = 0; i < BOARD_WIDTH; i++) {
          fillBlock(i, row, BACKGROUND_COLOR);
        }

        for (byte r = row; r > 1; r--) {
          for (byte c = 0; c < BOARD_WIDTH; c++) {
            swap(grid[c][r], grid[c][r - 1]);
            fillBlock(c, r, grid[c][r]);
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
        if ((bitRead(predictedShapePositions[i], j) == 1) && (grid[xOffset + x + (left ? -1 : 1)][yOffset + i] != BACKGROUND_COLOR)) {
          return false;
        }
      }
    }
  }

  return true;
}
byte getNextRotation() {
  return (shapeRotations[currentShape] - 1) == currentRotation ? 0 : currentRotation + 1;
}

bool canRotate() {
  byte nextRotation = getNextRotation();

  for (byte k = 0; k < 1; k++) {
    for (byte i = 0; i < 4; i++) {
      for (short j = 3, x = 0; j != -1; j--, x++) {
        if (bitRead(shapes[currentShape][nextRotation][i], j) == 1) {
          if ((xOffset + x) >= BOARD_WIDTH) {
            // will rotate off grid
            return false;
          }


          if (grid[xOffset + x][yOffset + i] != BACKGROUND_COLOR) {
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

void rotate() {
  if (shapeRotations[currentShape] == 1) {
    return;
  }

  if (!canRotate()) {
    return;
  }

  for (byte k = 0; k < 1; k++) {
    for (byte i = 0; i < 4; i++) {
      for (short j = 3, x = 0; j != -1; j--, x++) {
        if (bitRead(shapes[currentShape][currentRotation][i], j) == 1) {
          fillBlock(xOffset + x, yOffset + i, k == 0 ? BACKGROUND_COLOR : getCurrentShapeColor());
        }
      }
    }

    strip.show();
    currentRotation = getNextRotation();
  }
}

void joystickMovement() {
  unsigned long now = millis();
  static bool hasClicked = false;
  static unsigned long lastMove = now;
  static short lastYoffset = yOffset;
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
#else
  // Handle buttons
  if ((now - lastMove) > MOVE_DELAY){
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

  bool rotatePressed = false;
  if (now - lastRotate > ROTATE_DELAY && debounceButton(BUTTON_ROTATE)) {
    stamp -= level;
    lastRotate = now;
    rotatePressed = true;
  }
  
#endif


  hasClicked = hasClicked && rotatePressed;
  if (!hasClicked && rotatePressed) {
    hasClicked = true;
    rotate();
  }
}

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
      if (grid[x][y] == BACKGROUND_COLOR) {
        Serial.print(" ");
      } else {
        Serial.print("X");
      }
    }
    Serial.println();
  }
#endif
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
#else
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_ROTATE, INPUT_PULLUP);
#endif

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

  for (byte i = 0; i < BOARD_WIDTH; i++) {
    for (byte j = 0; j < BOARD_HEIGHT; j++) {
      fillBlock(i, j, BACKGROUND_COLOR);
    }
  }
  strip.show();
  // TODO: Attach a button
  waitForClick();
  currentShape = random(SHAPE_COUNT);
  nextShapeIndex = random(SHAPE_COUNT);
  nextShape();
  Serial.print("First shape: ");
  Serial.println(shapeNames[currentShape]);
  stamp = millis();

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

  if ((now - stamp) > level - score) {
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
}

