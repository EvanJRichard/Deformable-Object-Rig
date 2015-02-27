int blinkPin1 = 1;
int blinkPin2 = 4;

void setup()
{
  pinMode(blinkPin1, OUTPUT);
  pinMode(blinkPin2, OUTPUT);
}

void loop()
{
  digitalWrite(blinkPin1, HIGH);
  digitalWrite(blinkPin2, LOW);
  delay(500);
  digitalWrite(blinkPin1, LOW);
  digitalWrite(blinkPin2, HIGH);
  delay(500);
}

