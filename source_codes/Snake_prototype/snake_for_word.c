#include "LedControl.h" //A LedControl egy Arduino könyvtár MAX7219 és MAX7221 LED kijelző vezérlőkhöz. 

struct Pin {
  static const short joyX = A2; //Joystick X tengely pin
  static const short joyY = A3; //joystick Y tengely pin
  static const short joyVCC = 15; //Virtuális VCC a joystickhoz (Analog 1) (hogy a joystick közvetlenül az arduino nano mellé csatlakoztatható legyen)
  static const short joyGND = 14; //Virtuális GND a joystick számára (Analog 0) (hogy a joystick közvetlenül az arduino nano mellé csatlakoztatható legyen).
  static const short potMeter = A7; //Potenciométer a kígyó sebességének beállításához
  static const short CLK = 8;  //LED mátrix órajel
  static const short CS  = 9;  //LED mátrix chip-select 
  static const short DIN = 10; //LED mátrix adatbemenet 
};

const short brightness = 8;   //Fényerőt szabályzó változó
const short initialSnakeLength = 3; //Kezdeti kígyóhossz 
int snakeLength = initialSnakeLength; //Aktuális kígyóhossz (A játék elején beállítódik a konstans értékére)
int snakeSpeed = 1;  //Kígyó aktuális sebességét tároló változó
int snakeDirection = 0; //Kígyó menetirányát adja meg (Amígy 0, addig nem mozdul)
const short up     = 1; //Menetirány: FEL
const short right  = 2; //Menetirány: JOBBRA
const short down   = 3; //Menetirány: LE
const short left   = 4; //Menetirány: BALRA


const int joystickThreshold = 160;  //Küszöbérték, ahol a joystick mozgása elfogadható lesz

int gameboard[8][8] = {}; //8x8-as pálya tárolása
bool win = false;   //Megynert játék, amennyiben igaz - true
bool gameOver = false; //Elvesztett játék, amennyiben igaz - true


void setup() {
  Serial.begin(115200); // Beállítja a soros adatátvitel adatátviteli sebességét másodpercenkénti bitben (baud). 
  initGame();           //Kezdőállapotot beállító függvény meghívása
  calibrateJoystick();  //Joystick kalibrálását végző függvény meghívása
}

void loop() {
  generateFood();       //"Gyümölcs" generáló függvény meghívása
  scanJoystick();       //Joystick állapotát felmérő függvény meghívása
  calculateSnake();     //Snake helyzetét kiszámoló függvény meghívása
  setState();           //Állapot beéllító függvény nyerés/vesztés esetén
}

LedControl matrix(Pin::DIN, Pin::CLK, Pin::CS, 1); //8x8 mátrix pédányosítása

struct Point { //Pont struktúra létrehozása, ami rendelkezik egy sor és oszlop változóval
  int row = 0, col = 0; //Default értéke 0,0
  Point(int row = 0, int col = 0): row(row), col(col) {}
};

struct Coordinate { //Koordináta struktúra létrehozása, ami rendelkezik egy x és y változóval
  int x = 0, y = 0; //Default értéke 0,0
  Coordinate(int x = 0, int y = 0): x(x), y(y) {}
};



Point snake; //Snake létrehozása (Default értéke 0,0)

Point food(-1, -1); //"Gyümölcs" létrehozása, kezdetben pályán kívül

Coordinate joystickHome(500, 500); //Joystick inicializálása, default értékkel

void calibrateJoystick() { //Joystick kalibrálására szolgáló függvény
  Coordinate values; //Pozíciót tároló változó

  for (int i = 0; i < 10; i++) { //Tízszer fut le az ellenőrzés 
    values.x += analogRead(Pin::joyX); //Joystick X pozícióját beolvassa
    values.y += analogRead(Pin::joyY); //Joystick Y pozícióját beolvassa
  }

  joystickHome.x = values.x / 10; //Joystick X pozíciójának kalibrált beállítása
  joystickHome.y = values.y / 10; //Joystick Y pozíciójának kalibrált beállítása
}

void initGame() { //Játék kezdőállapotának beállítása
  pinMode(Pin::joyVCC, OUTPUT);  //joyVCC kimenetre állítás
  digitalWrite(Pin::joyVCC, HIGH); //joyVCC-re magas jel továbbítás

  pinMode(Pin::joyGND, OUTPUT); //joyGNG kimenetre állítás
  digitalWrite(Pin::joyGND, LOW); //joyGND-re digitális alacsony állítás

  matrix.shutdown(0, false); //Mátrix "felébresztése" alvó módból. Alvó módban az összes LED kikapcsolt állapotban van, viszont adatokat továbbra is lehet a mátrixnak küldeni
  matrix.setIntensity(0, brightness); //Mátrix fényerejének beállítása
  matrix.clearDisplay(0); //Az összes led kikapcsolása (nem azonos a shutdown-nal)

  randomSeed(analogRead(A5)); //Random seed
  snake.row = random(8); //Random sor pozíxió
  snake.col = random(8); //Random oszlop pozíxió
}

void generateFood() { //"Gyümölcs" véletlenszerű generálása
  if (food.row == -1 || food.col == -1) { //Ha a gyümölcs a pályán kívül jönne létre
    if (snakeLength >= 64) { //Ellenőríz, hogy ez azért van-e, mert betelt a pálya (a kígyóval)
      win = true; //Ha igen, nyertünk
      return;
}                             //Ellenkező esetben egy random helyre pakoljuk át a pályán
    food.col = random(8);     //Random vízszinte pozíció beállítása
    food.row = random(8);     //Random függőleges pozíció beállítása
  }
}

void scanJoystick() {         //Joystick elmozdítását figyelő függvény
  int previousDirection = snakeDirection; //Előző irányt tároló lokális változó
  long timestamp = millis();      

  while (millis() < timestamp + snakeSpeed) { //Sebességtől függő ellenőrzés
    float raw = mapFunction(analogRead(Pin::potMeter), 0, 1023, 0, 1); 
    snakeSpeed = mapFunction(pow(raw, 3.5), 0, 1, 10, 1000); //sebesség számolása a potenciométer elforgatásának függvényében
    if (snakeSpeed == 0) snakeSpeed = 1; //Ha le van tekerve a potenciométer, 1 a sebessége

    analogRead(Pin::joyY) < joystickHome.y - joystickThreshold ? snakeDirection = up    : 0;    //Ha felfele van elmozdítva
    analogRead(Pin::joyY) > joystickHome.y + joystickThreshold ? snakeDirection = down  : 0;    //Ha lefele van elmozdítva
    analogRead(Pin::joyX) < joystickHome.x - joystickThreshold ? snakeDirection = left  : 0;    //Ha balra van elmozdítva
    analogRead(Pin::joyX) > joystickHome.x + joystickThreshold ? snakeDirection = right : 0;    //Ha jobbra van elmozdítva

    snakeDirection + 2 == previousDirection && previousDirection != 0 ? snakeDirection = previousDirection : 0; //Teljes elfordulás nem lehetséges, amennyiben mozgásban van
    snakeDirection - 2 == previousDirection && previousDirection != 0 ? snakeDirection = previousDirection : 0; //Teljes elfordulás nem lehetséges, amennyiben mozgásban van
    matrix.setLed(0, food.row, food.col, millis() % 100 < 50 ? 1 : 0); //"Gyümölcs" megvillantása
  }
}

void calculateSnake() { //Kígyó mozgásának kiszámolása
  switch (snakeDirection) { //Mozgásirány alapján
    case up:
      snake.row--;
      wallDetection(); //Fal ellenőrző függvény meghívása
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

    default: //Default érték, amennyiben a kígyó nem mozog
      return;
  }

  if (gameboard[snake.row][snake.col] > 0 && snakeDirection != 0) { //Ha a cellában, amibe menne már van egy kígyó szegments
    gameOver = true; //Akkor Game Over
    return;
  }

  if (snake.row == food.row && snake.col == food.col) { //Ha a cellában, amibe menne van egy "gyümölcs"
    food.row = -1;  //Gyümölcs pályán kívül helyezése
    food.col = -1;  //Ez alapján tudja a program, hogy új helyre kell rakni

    snakeLength++;  //Kígyó hossza növekedett

    for (int row = 0; row < 8; row++) { 
      for (int col = 0; col < 8; col++) {
        if (gameboard[row][col] > 0 ) {
          gameboard[row][col]++; //Az egész kígyó hosszának növelése
        }
      }
    }
  }

  gameboard[snake.row][snake.col] = snakeLength + 1; //A kígyó fejéhez adjuk a szegmenst

  for (int row = 0; row < 8; row++) {  //A végén ki kell kapcsolni a ledet, hogy ne nyúljon a végtelenségig
    for (int col = 0; col < 8; col++) {
      if (gameboard[row][col] > 0 ) {
        gameboard[row][col]--;
      }
      matrix.setLed(0, row, col, gameboard[row][col] == 0 ? 0 : 1); //setLed: ha true a LED bekapcsol, ha false akkor kikapcsol
    }
  }
}

void wallDetection() { //Faldetektáló függvény. Amennyiben falat ér a kígyó, a másik oldalon folytatja útját
  snake.col < 0 ? snake.col += 8 : 0;
  snake.col > 7 ? snake.col -= 8 : 0;
  snake.row < 0 ? snake.row += 8 : 0;
  snake.row > 7 ? snake.row -= 8 : 0;
}


void setState() { //Állapotot beállító függvény
  if (gameOver || win) {
    resetGame();
  }
}

void resetGame() { //Alapállapot visszaállítása
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

float mapFunction(float x, float in_min, float in_max, float out_min, float out_max) { //
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
