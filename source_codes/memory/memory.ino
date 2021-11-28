#define DELAY 2000               // A játékosnak ennyi ideje van gondolkozni két gombnyomás között  

byte solution[100];              // A megoldást tároló tömb. Mérete növelhető, viszont nem szükséges
byte currentLength = 0;          // A megoldás jelenlegi hossza
byte inputCount = 0;             // Adott körben lenyomott gombok száma
byte lastInput = 0;              // A játékos utolsó megnyomott gombja
byte expectedInput = 0;          // Elvárt gombnyomás
bool isButtonPressed = false;    // Segítségével megállapítható, hogy van-e benyomva gomb.
// Ha van - true
bool isPlayersTurn = false;      // Jelzi, hogy a játékos köre van-e éppen. Ha igen - true
bool isGameOver = false;         // Játék végét jelzi. Ha a játéknak vége - true
byte numberOfPins = 5;           // A felhasznált LED-ek száma, mely 2-től lehet akár 13 is
//. Jelen esetben 5 LED-ből fog állni a játék.
byte pins[] = {2, 4, 6, 8, 10}; //LED-ek és gombok pin-jei
long inputTime = 0;              // Segédváltozó a gombnyomások közti idő mérésére

void setup() {
  Serial.begin(9600);         // Beállítja a soros adatátvitel adatátviteli sebességét másodpercenkénti bitben (baud).
  ResetGame();
}

void setPinMode(byte mode) { //Ennek a függvénynek a segítségével egyszerre állíthatjuk be, hogy INPUT, vagy OUTPUT módban szeretnénk használni a pin-eket, amik a pin[] tömbben lettek felsorolva
  for (byte i = 0; i < numberOfPins; i++) {
    pinMode(pins[i], mode);
  }
}

void writeAllPins(byte val) { //Ennek a függvénynek a segítségével az összes LED egyszerre lesz írható
  for (byte i = 0; i < numberOfPins; i++) {
    digitalWrite(pins[i], val);
  }
}

void flashLeds(short frequency) { //frequency paraméterrel megadható a villogás frekvenciája. Minél nagyobb, annál lassabb
  setPinMode(OUTPUT); // OUTPUT-ra állítjuk a pineket, mivel a LED-eket használjuk
  for (int i = 0; i < 5; i++) { //5x be-, majd kikapcsoljuk a LED-eket
    writeAllPins(HIGH);
    delay(frequency);
    writeAllPins(LOW);
    delay(frequency);
  }
}

void ResetGame() { //Változók visszaállítása az alapértelmezett értékekre
  flashLeds(500);
  currentLength = 0;
  inputCount = 0;
  lastInput = 0;
  expectedInput = 0;
  isButtonPressed = false;
  isPlayersTurn = false;
  isGameOver = false;
}

void playSolution() { //Végigmegy a jelenleg tárolt feladványon kiírja, valamint felvillantja a megfelelő LED-eket
  for (int i = 0; i < currentLength; i++) {
    Serial.print("Seq: ");
    Serial.print(i);
    Serial.print("Pin: ");
    Serial.println(solution[i]);
    digitalWrite(solution[i], HIGH);
    delay(500);
    digitalWrite(solution[i], LOW);
    delay(250);
  }
}

void endGame() {
  flashLeds(50);            // Gyorsan villogtatja az összes LED-et
  delay(1000);
  ResetGame();            // Alaphelyzetbe állítja a játékot
}

void loop() {
  if (!isPlayersTurn) {    //Arduino köre
    setPinMode(OUTPUT);                      //A LED-eket használjuk, így OUTPUT-ra kell állítani a pineket

    randomSeed(analogRead(A0));                   // https://www.arduino.cc/en/Reference/RandomSeed
    solution[currentLength] = pins[random(0, numberOfPins)];   // Egy random számmal kiegészül a feladvány https://www.arduino.cc/en/Reference/random
    currentLength++;                                     // A feladvány hossza növekedett

    playSolution();                               // Feladvány megjelenítése a LED-eken

    isPlayersTurn = true;                                  // Amennyiben lejátszódott a feladvány bemutatása, az isPlayersTurn igazra vált, jelezve, hogy a játékos következik
    inputTime = millis();                         // Eltárolja a kör befejezésének idejét
  }
  else { // Játékos következik
    setPinMode(INPUT);                       // A gombokat használjuk, így INPUT-ra kell állítani a pineket

    if (millis() - inputTime > DELAY) { // Amennyiben a játékos több ideig gondolkozott, mint a megengedett, vesztett
      endGame();
      return;
    }

    if (!isButtonPressed) {                                 // Meg kell várni, míg nincs benyomva gomb
      expectedInput = solution[inputCount];               // Beállítódik az elvárt
      for (int i = 0; i < numberOfPins; i++) {          // Végig kell nézni, az összes pint
        if (pins[i] != expectedInput && digitalRead(pins[i]) == HIGH) {
          isGameOver = true;                       // Mivel nem az elvárt gomb lett nyomva, ezért vége a játéknak
          isButtonPressed = true;                 // Beragadás ellen true-ra kell állítani a változót, különben kéretlenül rossz ágra fog futni
        }
      }
    }

    if (digitalRead(expectedInput) == 1 && !isButtonPressed) {      // Amennyiben meg lett nyomva a helyes gomb, megy tovább a program
      inputTime = millis();                                       // Újraindul az időzítő
      lastInput = expectedInput;                                  //Ebben az esetben a legutoljára lenyomott gomb az elvárt
      inputCount++;                                               // Jól bevitt nyomások száma
      isButtonPressed = true;                                     // Beragadás ellen true-ra kell állítani a változót, különben kéretlenül rossz ágra fog futni
    }
    else {
      if (isButtonPressed && digitalRead(lastInput) == LOW) { //Ha megnyomtuk a gombot, de az már nincs nyomva
        isButtonPressed = false;
        delay(20);
        if (isGameOver) {                            // Ellenőrízzük, hogy elrontottuk-e a feladványt
          endGame();                          // Ha igen, akkor játék vége
        }
        else {
          if (inputCount == currentLength) {               // Különben felkészítjük a játékot a következő körre
            isPlayersTurn = false;                           // AZ Arduino következik
            inputCount = 0;                         // A nyomott gombok számlálója nullázódik
            delay(1500);
          }
        }
      }
    }
  }
}
