#include <SoftwareSerial.h>
#include "DHT.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<DSM501.h>
/*---------------------------------------*/

#define GPRS_Tx 8
#define GPRS_Rx 7

#define onModulePin 9

#define DOUTpin 6
#define AOUTpin A0
#define DSM501_PM10 3
#define DSM501_PM25 4

/*---------------------------------------*/
SoftwareSerial GPRS(GPRS_Rx, GPRS_Tx);
DSM501 dsm501(DSM501_PM10, DSM501_PM25);
DHT dht;
/*---------------------------------------*/

float humidity = 0.0, temperature = 0.0;
int answer = 0;
char response[100];



#define	uchar	unsigned char
#define	uint	unsigned int
/*---------------------------------------*/
void setup()
{
  dsm501.begin(MIN_WIN_SPAN);
  dht.setup(2);
  pinMode(onModulePin, OUTPUT);
  delay(100);
  GPRS.begin(9600);
  Serial.begin(9600);
  Serial.println("Starting...");
  powerOn();
  while (sendATcommand("AT", "OK", 1000) == 0);
  sendATcommand("", "Call Ready", 5000);
  Serial.println("Done");
  setupGPRS();
}

/*---------------------------------------*/
void loop ()
{
  int value = readDustValue();
  readTH(&temperature, &humidity);
  if(Serial.available())
  {  
    if(Serial.read() == 'T')
      sendTransaction(temperature, humidity, 125, readMQ7());
  }
  delay(100);
}

/*---------------------------------------*/
int readDustValue()
{
  return (dsm501.getAQI());
}

/*---------------------------------------*/
void readTH(float *temp, float *hum)
{
  delay(dht.getMinimumSamplingPeriod());
  *hum = dht.getHumidity();
  *temp = dht.getTemperature();
}

/*---------------------------------------*/
int readMQ7()
{
  int limit;
  int value;
  long valueSum = 0;
  int count = 0;
  for(int i = 0; i<500; i++)
  {
    valueSum += analogRead(AOUTpin);
    limit = digitalRead(DOUTpin);
    count = i;    
  }
  value = valueSum/count;
  return value;
}

/*---------------------------------------*/
void setupGPRS()
{
  Serial.println("setupGPRS.");
  sendATcommand("ATE0", "OK", 1000);
  sendATcommand("AT+IPR=9600", "OK", 1000);
  sendATcommand("AT+CMGF=1", "OK", 100);
  while (sendATcommand("AT+CSTT=\"zongwap\"", "OK", 1000) == 0 );
  while (sendATcommand("AT+CIICR", "OK", 5000) == 0);
  sendATcommand("AT+CIFSR", ".", 2000);
  sendATcommand("AT+CIFSR=?", "OK", 2000);
  sendATcommand("AT+CDNSCFG=\"8.8.8.8\",\"8.8.4.4\"", "OK", 2000);
  sendATcommand("AT+CGATT=1", "OK", 5000);
  sendATcommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", "OK", 2000);
  sendATcommand("AT+SAPBR=3,1,\"APN\",\"zongwap\"", "OK", 2000);
  sendATcommand("AT+SAPBR=1,1", "OK", 5000);
}

/*---------------------------------------*/
void sendTransaction(float t, unsigned int d, unsigned int h, unsigned int mq)
{
  while (sendATcommand("AT+HTTPINIT", "OK", 2000) == 0);
  httpSetParameter(t, d, h, mq);
  sendATcommand("AT+HTTPACTION=0", "+HTTPACTION:0,200,", 30000);
  Serial.print("Response:" );
  Serial.println(response);
  delay(1000);
  sendATcommand("AT+HTTPREAD=0,100", "OK", 5000);
  Serial.print("Response:" );
  Serial.println(response);
  while (sendATcommand("AT+HTTPTERM", "OK", 1000) == 0);
}

/*---------------------------------------*/
void httpSetParameter(float t, unsigned int d, unsigned int h, unsigned int mq)
{
  char t_[10];
  dtostrf( t, 4, 2, t_ );
  //http://damp-crag-29984.herokuapp.com/server?t=14.56&d=99.26&h=123456&mq=22
  /*http://immunization.ipal.itu.edu.pk/index.php/ApiEngine/IPushdata?rfid=4536822837&mcode=255645435345&amount=223&rate=5.5*/
  const char *URL = "damp-crag-29984.herokuapp.com/server?";
  char command[200];
  sprintf(command, "AT+HTTPPARA=\"URL\",\"%st=%s&d=%i&h=%i&mq=%i\"", URL, t_, d, h, mq);
  sendATcommand(command, "OK", 1000);
}

/*---------------------------------------*/
void powerOn()
{
  digitalWrite(onModulePin, LOW);
  delay(1000);
  digitalWrite(onModulePin, HIGH);
  delay(2000);
  digitalWrite(onModulePin, LOW);
  delay(4000);
}

/*---------------------------------------*/
void powerDown()
{
  sendATcommand("AT+CPOWD=1", "NORMAL POWER DOWN", 5000);
}

/*---------------------------------------*/
int sendATcommand(char* ATcommand, char* expected_answer1, unsigned int timeout)
{
  GPRS.begin(9600);
  Serial.println(ATcommand);
  unsigned int x = 0;
  answer = 0;
  unsigned long previous;
  memset(response, '\0', 100);    // Initialize the string
  delay(100);
  while ( GPRS.available() > 0) GPRS.read();   // Clean the input buffer
  GPRS.println(ATcommand);    // Send the AT command
  previous = millis();
  do {
    if (GPRS.available() != 0) {
      while (GPRS.available())
      {
        response[x] = GPRS.read();
        x++;
      }
    }
    if (strstr(response, expected_answer1) != NULL)
    {
      answer = 1;
    }
    else
      answer = 0;
  }
  while ((answer == 0) && ((millis() - previous) < timeout));
  while (GPRS.available())
  {
    response[x] = GPRS.read();
    x++;
  }
  if ((millis() - previous) > timeout)
    Serial.println("TimeOut.");
  Serial.println(response);
  return answer;
}
