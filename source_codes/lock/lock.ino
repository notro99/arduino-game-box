
#include <SPI.h> //Ez a könyvtár lehetővé teszi az SPI eszközökkel való kommunikációt, ahol az Arduino a master eszköz.
#include <MFRC522.h> //Arduino könyvtár MFRC522 és más RFID RC522 alapú modulokhoz.
 
#define SS 10
#define RST 9
#define BUZZER 2 //Csengő pin
#define RELAY 3 //Relé pin
#define DELAY 1000 //Késleltetés ms-ben megadva
MFRC522 mfrc522(SS, RST);   //MFRC522 RFID scanner páldányosítása
 
void setup() {
  Serial.begin(9600);   // Beállítja a soros adatátvitel adatátviteli sebességét másodpercenkénti bitben (baud). 
  SPI.begin();          //  Az SPI-busz inicializálása az SCK, MOSI és SS kimenetekre állításával, az SCK és MOSI alacsonyra, az SS pedig magasra húzásával. 
  mfrc522.PCD_Init();   // Az MFRC522 chip inicializálása.
  pinMode(RELAY, OUTPUT); //A RELAY pin-t úgy konfigurálja, hogy kimenetként viselkedjen.
  pinMode(BUZZER, OUTPUT); //A BUZZER pin-t úgy konfigurálja, hogy kimenetként viselkedjen.
  noTone(BUZZER); //Leállítja a tone() által kiváltott négyszöghullám generálását. Nincs hatása, ha nem generál hangot.
  digitalWrite(RELAY, LOW); //A pinMode() segítségével OUTPUT-ként konfigurált pin, akkor 0V (föld)-re lesz állítva.
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) {//Igazat ad vissza, ha egy PICC válaszol a PICC_CMD_REQA parancsra. Csak az IDLE állapotú "új" kártyák kapnak meghívást. A HALT állapotban lévő alvó kártyákat figyelmen kívül hagyja
    return; //Addig loopol a függvény, amíg nem érzékel kártyát
  }
  //A program akkor jut ide, ha új kártyát észlelt
  if (!mfrc522.PICC_ReadCardSerial()) {//Igazat ad vissza, ha egy UID beolvasható.  
    return;
  }
  //Debugolási szempontból érdemes kiíratni az érzékelt kártya azonosítóját
  Serial.print("UID :");
  String tagUid= ""; //Változó, amiben az érzékelt RFID tag UID-jét tároljuk
  for (byte i = 0; i < mfrc522.uid.size; i++) {//Végigmegyünk az UID hosszán
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX); //Kiiratjuk az aktuális karaktert
     tagUid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     tagUid.concat(String(mfrc522.uid.uidByte[i], HEX)); //A későbbi ellenőrzés végett elmentjük az UID karaktereit a korábban létrehozozz változónkba
  }
  Serial.println();
  Serial.print("Belepes : ");
  tagUid.toUpperCase(); //Minden betűt nagybetűre állítunk
  if (tagUid.substring(1) == "XX XX XX XX" || tagUid.substring(1) == "XX XX XX XX") {//Itt ellenőrízhetjük, hogy a megadottak között volt-e a próbált RFID tag
    Serial.println("Jovahagyva"); 
    Serial.println();
    delay(500); //A program elején megadott ideig késlelteti a továbbhaladást
    digitalWrite(RELAY, HIGH); //Relére magas feszültség küldése, aminek hatására az kinyitja a szolenoid zárat
    delay(DELAY);   //A program elején megadott ideig késlelteti a továbbhaladást
    digitalWrite(RELAY, LOW); //A relé alacsony fesztültséget kap, a zár a kezdőpozíciójába áll
  } else {
    Serial.println("Megtagadva");
    tone(BUZZER, 300); //Sikertelen kísérlet esetén a megadott frekvenciájú négyszöghullámot generál  
    delay(DELAY); //A program elején megadott ideig késlelteti a továbbhaladást
    noTone(BUZZER); //A piezo hangjelző a noTone() hívásáig fog hangot kibocsájtani
  }
}

//...








