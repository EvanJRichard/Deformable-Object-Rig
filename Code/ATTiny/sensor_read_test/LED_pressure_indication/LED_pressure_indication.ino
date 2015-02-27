#define P_LED_PIN PB4
#define PRESS_PIN PB3
#define UP_LIM 710
#define LOW_LIM 300

void setup()
{
}

void loop()
{
  int senseVal = analogRead(PRESS_PIN);
  int ledBright = (int)(((float)abs(senseVal - LOW_LIM)/(UP_LIM - LOW_LIM)) * 255);
  analogWrite(P_LED_PIN, ledBright);
}
