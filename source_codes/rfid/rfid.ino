
#include <SPI.h>
#include <RFID.h>
#include <FastLED.h>

#define SDA_DIO 9
#define RESET_DIO 8

#define BUZZER 2      //Csengő pin
#define LED_PIN 3     //Szalag DIN pin
#define NUM_LEDS 30   //LED-ek száma a szalagon
#define LEDS_FIRST 15 //A LED-ek első felének száma

#define RELAY 12      //Relé pin
#define DELAY_1S 1000 //Késleltetés ms-ben megadva
#define DELAY_5S 5000 //Késleltetés ms-ben megadva

RFID RC522(SDA_DIO, RESET_DIO); //RFID olvasó példányosítás

int R = 255;                      //Piros
int G = 255;                      //Zöld
int B = 255;                      //Kék
bool isPermissionAllowed = false; //Jogosultság ellenőrzésére szolgáló segégváltozó
CRGB leds[NUM_LEDS];

void setup()
{
  Serial.begin(9600);                                    // Beállítja a soros adatátvitel adatátviteli sebességét másodpercenkénti bitben (baud).
  SPI.begin();                                           //  Az SPI-busz inicializálása az SCK, MOSI és SS kimenetekre állításával, az SCK és MOSI alacsonyra, az SS pedig magasra húzásával.
  RC522.init();                                          // Az MFRC522 chip inicializálása
  pinMode(RELAY, OUTPUT);                                //A RELAY pin-t úgy konfigurálja, hogy kimenetként viselkedjen.
  pinMode(BUZZER, OUTPUT);                               //A BUZZER pin-t úgy konfigurálja, hogy kimenetként viselkedjen.
  noTone(BUZZER);                                        //Leállítja a tone() által kiváltott négyszöghullám generálását. Nincs hatása, ha nem generál hangot.
  digitalWrite(RELAY, LOW);                              //A pinMode() segítségével OUTPUT-ként konfigurált pin, akkor 0V (föld)-re lesz állítva.
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS); //WS2812 LED szalag példányosítása
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);        //Beállítja a maximálisan felhasznált áramot milliamperben egy adott feszültséghez.
  FastLED.clear();                                       //Törli a LED-ek értékét
  FastLED.show();                                        //Frissíti a vezérlőket az aktuális színekkel
  R = 255;                                               //Színátmenet esetén ezeknek a változóknak a módosításával
  G = 255;                                               //lehet különböző effektusokat elérni
  B = 255;
}

void loop()
{
  String tagUid = "";
  if (RC522.isCard()) //Igazat ad vissza, ha egy kártyát érzékel a közelben
  {
    RC522.readCardSerial(); //Igazat ad vissza, ha egy sorszám kiolvasható
    Serial.println("Kartya beolvasva:");
    for (int i = 0; i < 5; i++)
    {
      Serial.print(RC522.serNum[i], DEC);          //Soros kimenetre az azonosító kiíratása
      tagUid.concat(String(RC522.serNum[i], DEC)); //Azonosító kimentése segédváltozóba
    }
    Serial.println();
    Serial.println(tagUid);

    if (tagUid == "2810916423194") //Itt ellenőrízhetjük, hogy a megadottak között volt-e a próbált RFID tag
    {
      Serial.println("Jovahagyva");
      Serial.println();
      delay(500);                //A program elején megadott ideig késlelteti a továbbhaladást
      digitalWrite(RELAY, HIGH); //Relére magas feszültség küldése, aminek hatására az kinyitja a szolenoid zárat
      playAcceptSequence();      //Beengedést jelző, zöld animáció
      resetLeds();               //LED-ek kikapcsolása
      delay(DELAY_5S);           //A program elején megadott ideig késlelteti a továbbhaladást
      digitalWrite(RELAY, LOW);  //A relé alacsony fesztültséget kap, a zár a kezdőpozíciójába áll
    }
    else
    {
      Serial.println("Megtagadva");
      tone(BUZZER, 300);     //Sikertelen kísérlet esetén a megadott frekvenciájú négyszöghullámot generál
      playDeclineSequence(); //Elutasítást jelző piros animáció
      resetLeds();           //LED-ek kiakpcsolása
      noTone(BUZZER);        //A piezo hangjelző a noTone() hívásáig fog hangot kibocsájtani
    }
  }
  delay(300);
}

void playAcceptSequence() //Zöld animáció, balról jobbra
{
  for (int j = 0; j < 5; j++)
  {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CRGB(0, G, 0);
      FastLED.show();
      delay(15);
    }
    FastLED.clear();
  }
  FastLED.clear();
}

void playDeclineSequence() //Középről kifelé pulzáló, piros animáció
{
  int k = LEDS_FIRST;
  for (int j = 0; j < 3; j++)
  {
    for (int i = LEDS_FIRST; i >= 0; i--)
    {
      leds[k] = CRGB(R, 0, 0);
      leds[i] = CRGB(R, 0, 0);
      FastLED.show();
      delay(20);
      k++;
    }
    k = LEDS_FIRST + 1;
    delay(20);
    for (int i = LEDS_FIRST; i >= 0; i--)
    {
      leds[k] = CRGB(0, 0, 0);
      leds[i] = CRGB(0, 0, 0);
      FastLED.show();
      delay(20);
      k++;
    }
    delay(30);
  }
  FastLED.clear();
  for (int j = 0; j < 4; j++)
  {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CRGB(R, 0, 0);
      FastLED.show();
    }
    delay(100);
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CRGB(0, 0, 0);
      FastLED.show();
    }
  }
  FastLED.clear();
}

void resetLeds() //Bekapcsolt LED-ek kikapcsolására szolgáló függvény
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(0, 0, 0);
    FastLED.show();
  }
}