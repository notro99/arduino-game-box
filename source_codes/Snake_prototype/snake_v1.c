#include "LedControl.h" 

struct Pin {
  static const short joyX = A2; 
  static const short joyY = A3; 
  static const short joyVCC = 15;
  static const short joyGND = 14; 
  static const short potMeter = A7; 
  static const short CLK = 8;  
  static const short CS  = 9;  
  static const short DIN = 10;
};

const short brightness = 8;
const short initialSnakeLength = 3;
int snakeLength = initialSnakeLength;
int snakeSpeed = 1; 
int snakeDirection = 0;
const short up     = 1;
const short right  = 2;
const short down   = 3; 
const short left   = 4; 

//controller parameters
const int joystickThreshold = 160;

int gameboard[8][8] = {};
bool win = false;
bool gameOver = false;


void setup() {
  Serial.begin(115200);
  initGame();       
  calibrateJoystick(); 
}

void loop() {
  generateFood();
  scanJoystick();    
  calculateSnake();  
  setState();
}

LedControl matrix(Pin::DIN, Pin::CLK, Pin::CS, 1);

struct Point {
  int row = 0, col = 0;
  Point(int row = 0, int col = 0): row(row), col(col) {}
};

struct Coordinate {
  int x = 0, y = 0;
  Coordinate(int x = 0, int y = 0): x(x), y(y) {}
};



Point snake;

Point food(-1, -1);

Coordinate joystickHome(500, 500);

void calibrateJoystick() {
  Coordinate values;

  for (int i = 0; i < 10; i++) {
    values.x += analogRead(Pin::joyX);
    values.y += analogRead(Pin::joyY);
  }

  joystickHome.x = values.x / 10;
  joystickHome.y = values.y / 10;
}

void initGame() {
  pinMode(Pin::joyVCC, OUTPUT);
  digitalWrite(Pin::joyVCC, HIGH);

  pinMode(Pin::joyGND, OUTPUT);
  digitalWrite(Pin::joyGND, LOW);

  matrix.shutdown(0, false);
  matrix.setIntensity(0, brightness);
  matrix.clearDisplay(0);

  randomSeed(analogRead(A5));
  snake.row = random(8);
  snake.col = random(8);
}

void generateFood() {
  if (food.row == -1 || food.col == -1) {
    if (snakeLength >= 64) {
      win = true;
      return;
    }
    food.col = random(8);
    food.row = random(8);   
  }
}

void scanJoystick() {
  int previousDirection = snakeDirection;
  long timestamp = millis();

  while (millis() < timestamp + snakeSpeed) {
    float raw = mapf(analogRead(Pin::potMeter), 0, 1023, 0, 1);
    snakeSpeed = mapf(pow(raw, 3.5), 0, 1, 10, 1000);
    if (snakeSpeed == 0) snakeSpeed = 1;

    analogRead(Pin::joyY) < joystickHome.y - joystickThreshold ? snakeDirection = up    : 0;
    analogRead(Pin::joyY) > joystickHome.y + joystickThreshold ? snakeDirection = down  : 0;
    analogRead(Pin::joyX) < joystickHome.x - joystickThreshold ? snakeDirection = left  : 0;
    analogRead(Pin::joyX) > joystickHome.x + joystickThreshold ? snakeDirection = right : 0;

    snakeDirection + 2 == previousDirection && previousDirection != 0 ? snakeDirection = previousDirection : 0;
    snakeDirection - 2 == previousDirection && previousDirection != 0 ? snakeDirection = previousDirection : 0;
    matrix.setLed(0, food.row, food.col, millis() % 100 < 50 ? 1 : 0);
  }
}

void calculateSnake() {
  switch (snakeDirection) {
    case up:
      snake.row--;
      wallDetection();
      matrix.setLed(0, snake.row, snake.col, 1);
      break;

    case right:
      snake.col++;
      wallDetection();
      matrix.setLed(0, snake.row, snake.col, 1);
      break;

    case down:
      snake.row++;
      wallDetection();
      matrix.setLed(0, snake.row, snake.col, 1);
      break;

    case left:
      snake.col--;
      wallDetection();
      matrix.setLed(0, snake.row, snake.col, 1);
      break;

    default:
      return;
  }

  if (gameboard[snake.row][snake.col] > 0 && snakeDirection != 0) {
    gameOver = true;
    return;
  }

  if (snake.row == food.row && snake.col == food.col) {
    food.row = -1; 
    food.col = -1;

    snakeLength++;

    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        if (gameboard[row][col] > 0 ) {
          gameboard[row][col]++;
        }
      }
    }
  }

  gameboard[snake.row][snake.col] = snakeLength + 1; 

  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      
      if (gameboard[row][col] > 0 ) {
        gameboard[row][col]--;
      }
      matrix.setLed(0, row, col, gameboard[row][col] == 0 ? 0 : 1);
    }
  }
}

void wallDetection() {
  snake.col < 0 ? snake.col += 8 : 0;
  snake.col > 7 ? snake.col -= 8 : 0;
  snake.row < 0 ? snake.row += 8 : 0;
  snake.row > 7 ? snake.row -= 8 : 0;
}


void setState() {
  if (gameOver || win) {
    //TODO win animation
    //Score
    //Game Over message(?) 
    win = false;
    gameOver = false;
    snake.row = random(8);
    snake.col = random(8);
    food.row = -1;
    food.col = -1;
    snakeLength = initialSnakeLength;
    snakeDirection = 0;
    memset(gameboard, 0, sizeof(gameboard[0][0]) * 8 * 8);
    matrix.clearDisplay(0);
  }
}


// standard map function, but with floats
float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
