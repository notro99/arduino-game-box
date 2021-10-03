int Led1 = 3;

void setup() {
  // put your setup code here, to run once:
  pinMode(Led1,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(Led1,0);
  delay(1000);
  digitalWrite(Led1,1);
  delay(1000);
}
