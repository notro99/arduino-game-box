#define DELAY 2000 // A játékosnak ennyi ideje van gondolkozni két gombnyomás között  

byte solution[100];           // A megoldást tároló tömb. Mérete növelhető, viszont nem szükséges
byte currentLength = 0;              // A megoldás jelenlegi hossza
byte inputCount = 0;          // Adott körben lenyomott gombok száma
byte lastInput = 0;           // A játékos utolsó megnyomott gombja
byte expectedInput = 0;               // Elvárt gombnyomás
bool isButtonPressed = false;          // Segítségével megállapítható, hogy van-e benyomva gomb. Ha van - true
bool isPlayersTurn = false;            // Jelzi, hogy a játékos köre van-e éppen. Ha igen - true
bool isGameOver = false;       // Játék végét jelzi. Ha a játéknak vége - true
byte numberOfPins = 5;              // A felhasznált LED-ek száma, mely 2-től lehet akár 13 is. Jelen esetben 5 LED-ből fog állni a játék.
byte pins[] = {2, 13, 10, 8, 5}; //LED-ek és gombok pin-jei                        
long inputTime = 0;           // Segédváltozó a gombnyomások közti idő mérésére

void setup() {
    Serial.begin(9600);         // Start Serial monitor. This can be removed too as long as you remove all references to Serial below
    ResetGame();
}

///
/// Sets all the pins as either INPUT or OUTPUT based on the value of 'dir'
///
void setPinMode(byte mode){
  for(byte i = 0; i < numberOfPins; i++){
    pinMode(pins[i], mode); 
  }
}

//send the same value to all the LED pins
void writeAllPins(byte val){
  for(byte i = 0; i < numberOfPins; i++){
    digitalWrite(pins[i], val); 
  }
}

///
/// Flashes all the LEDs together
/// freq is the blink speed - small number -> fast | big number -> slow
///
void flash(short freq){
  setPinDirection(OUTPUT); /// We're activating the LEDS now
  for(int i = 0; i < 5; i++){
    writeAllPins(HIGH);
    delay(freq);
    writeAllPins(LOW);
    delay(freq);
  }
}

///
///This function resets all the game variables to their default values
///
void ResetGame(){
  flash(500);
  currentLength = 0;
  inputCount = 0;
  lastInput = 0;
  expectedInput = 0;
  isButtonPressed = false;
  isPlayersTurn = false;
  isGameOver = false;
}

///
/// User lost
///
void Lose(){
  flash(50);  
}

///
/// The arduino shows the user what must be memorized
/// Also called after losing to show you what you last solution was
///
void playsolution(){
  //Loop through the stored solution and light the appropriate LEDs in turn
  for(int i = 0; i < currentLength; i++){
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

///
/// The events that occur upon a loss
///
void DoLoseProcess(){
  Lose();             // Flash all the LEDS quickly (see Lose function)
  delay(1000);
  playsolution();     // Shows the user the last solution - So you can count remember your best score - Mine's 22 by the way :)
  delay(1000);
  ResetGame();            // Reset everything for a new game
}

///
/// Where the magic happens
///
void loop() {  
  if(!isPlayersTurn){      
                            //****************//
                            // Arduino's turn //
                            //****************//
    setPinDirection(OUTPUT);                      // We're using the LEDs
    
    randomSeed(analogRead(A0));                   // https://www.arduino.cc/en/Reference/RandomSeed
    solution[currentLength] = pins[random(0,numberOfPins)];    // Put a new random value in the next position in the solution -  https://www.arduino.cc/en/Reference/random
    currentLength++;                                     // Set the new Current length of the solution
    
    playsolution();                               // Show the solution to the player
    
    isPlayersTurn = true;                                  // Set Wait to true as it's now going to be the turn of the player
    inputTime = millis();                         // Store the time to measure the player's response time
  }else{ 
                            //***************//
                            // Player's turn //
                            //***************//
    setPinDirection(INPUT);                       // We're using the buttons

    if(millis() - inputTime > DELAY){  // If the player takes more than the allowed time,
      DoLoseProcess();                            // All is lost :(
      return;
    }      
        
    if(!isButtonPressed){                                  // 
      expectedInput = solution[inputCount];               // Find the value we expect from the player
      Serial.print("Expected: ");                 // Serial Monitor Output - Should be removed if you removed the Serial.begin above
      Serial.println(expectedInput);                      // Serial Monitor Output - Should be removed if you removed the Serial.begin above
      
      for(int i = 0; i < numberOfPins; i++){           // Loop through the all the pins
        if(pins[i]==expectedInput)                        
          continue;                               // Ignore the correct pin
        if(digitalRead(pins[i]) == HIGH){         // Is the buttong pressed
          lastInput = pins[i];
          isGameOver = true;                       // Set the isGameOver - this means you lost
          isButtonPressed = true;                          // This will prevent the program from doing the same thing over and over again
          Serial.print("Read: ");                 // Serial Monitor Output - Should be removed if you removed the Serial.begin above
          Serial.println(lastInput);              // Serial Monitor Output - Should be removed if you removed the Serial.begin above
        }
      }      
    }

    if(digitalRead(expectedInput) == 1 && !isButtonPressed)        // The player pressed the right button
    {
      inputTime = millis();                       // 
      lastInput = expectedInput;
      inputCount++;                               // The user pressed a (correct) button again
      isButtonPressed = true;                              // This will prevent the program from doing the same thing over and over again
      Serial.print("Read: ");                     // Serial Monitor Output - Should be removed if you removed the Serial.begin above
      Serial.println(lastInput);                  // Serial Monitor Output - Should be removed if you removed the Serial.begin above
    }else{
      if(isButtonPressed && digitalRead(lastInput) == LOW){  // Check if the player released the button
        isButtonPressed = false;
        delay(20);
        if(isGameOver){                              // If this was set to true up above, you lost
          DoLoseProcess();                          // So we do the losing solution of events
        }
        else{
          if(inputCount == currentLength){                 // Has the player finished repeating the solution
            isPlayersTurn = false;                           // If so, this will make the next turn the program's turn
            inputCount = 0;                         // Reset the number of times that the player has pressed a button
            delay(1500);
          }
        }
      }
    }    
  }
}