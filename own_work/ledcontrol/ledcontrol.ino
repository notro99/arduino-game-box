//Viral Science  www.viralsciencecreativity.com  www.youtube.com/c/viralscience
//LED 8x8 Matrix Display DEMO

#include <LedControl.h>
int DIN = 10;
int CS =  9;
int CLK = 8;

LedControl lc=LedControl(DIN,CLK,CS,0);

void setup(){
 lc.shutdown(0,false);       
 lc.setIntensity(0,4);      //Adjust the brightness maximum is 15
 lc.clearDisplay(0);    
}

void loop(){ 
    
    //Facial Expression
    byte smile[8]=   {0x3C,0x42,0xA5,0x81,0xA5,0x99,0x42,0x3C};

    byte oo[8]=   {0x3C,0x42,0xA5,0x81,0x99,0xA5,0x5A,0x3C};

   byte heart1[8] = {0x00,0x00,0x6C,0x92,0x82,0x44,0x28,0x10};
    
   byte heart2[8] = {0x00,0x00,0x36,0x49,0x41,0x22,0x14,0x08};



//Facial Expression   
    printByte(heart1);
     
    delay(200);

    printByte(heart2);

    delay(200);
     
 
}

void printByte(byte character [])
{
  int i = 0;
  for(i=0;i<8;i++)
  {
    lc.setRow(0,i,character[i]);
  }
}
