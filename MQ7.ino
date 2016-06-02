/* MQ-7 Carbon Monoxide Sensor Circuit with Arduino */

const int ledPin=13;//the anode of the LED connects to digital pin D13 of the arduino



void setup() {
  Serial.begin(9600);//sets the baud rate
  pinMode(DOUTpin, INPUT);//sets the pin as an input to the arduino
  pinMode(ledPin, OUTPUT);//sets the pin as an output of the arduino
}

void loop()
{
  Serial.println(readMQ7());
  delay(100);
}
