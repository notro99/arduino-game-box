#include <FastLED.h>
#define LED_PIN 2
#define NUM_LEDS 30
#define LEDS_FIRST 15
int R = 255;
int G = 255;
int B = 255;
bool isPermissionAllowed = false;
bool isLedSequenceOver = false;
CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.clear();
  
  FastLED.show();
  R = 255;
  G = 255;
  B = 255;
  Serial.begin(9600);
}

void loop() {
  int k = 15;
  if(isPermissionAllowed && !isLedSequenceOver) {
    for (int j = 0; j < 100; j++){
      for(int i = 0; i<NUM_LEDS; i++) {
        leds[i] = CRGB(0, G, 0);
        FastLED.show();
        delay(15);
      }
      FastLED.clear();
      }
    isLedSequenceOver=true;
    FastLED.clear();
  } else if(!isPermissionAllowed && !isLedSequenceOver){ 
    for (int j = 0; j < 3; j++){
        for(int i = 15; i>=0; i--) {  
          leds[k]= CRGB(R, 0, 0);
          leds[i] = CRGB(R, 0, 0);
          FastLED.show();
          delay(20);
          k++;
        }
        k=16;
        delay(20);
        for(int i = 15; i>=0; i--) {     
          leds[k]= CRGB(0, 0, 0);
          leds[i] = CRGB(0, 0, 0);
          FastLED.show();
          delay(20);
          k++;
        }
        delay(30);
      }
      FastLED.clear();
      for(int j=0; j<4; j++){
        for(int i = 0; i<NUM_LEDS; i++) {
          leds[i] = CRGB(R, 0, 0);
          FastLED.show();
        }
        delay(100);
        for(int i = 0; i<NUM_LEDS; i++) {
          leds[i] = CRGB(0, 0, 0);
            FastLED.show();
        }
    }
    isLedSequenceOver=true;
    FastLED.clear();
  } else {
    for(int i = 0; i<NUM_LEDS; i++) {
        leds[i] = CRGB(0, 0, 0);
          FastLED.show();
      }
  }


  
  
}
