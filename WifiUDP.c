/*

 Udp NTP Client

 Get the time from a Network Time Protocol (NTP) time server
 Demonstrates use of UDP sendPacket and ReceivePacket
 For more on NTP time servers and the messages needed to communicate with them,
 see http://en.wikipedia.org/wiki/Network_Time_Protocol

 created 4 Sep 2010
 by Michael Margolis
 modified 9 Apr 2012
 by Tom Igoe

 This code is in the public domain.

 */

#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>

int status = WL_IDLE_STATUS;
#include "arduino_secrets.h" 
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

unsigned int localPort = 2390;      // local port to listen for UDP packets
IPAddress timeServer(34, 234, 210, 135); // AWS Server
uint8_t AWS_Port = 80;

const int NTP_PACKET_SIZE = 17; 
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
boolean stringComplete = false;
uint8_t val=0;

void setup()
{
  WiFi.setPins(8,7,4);
  Serial.begin(115200, SERIAL_8N1);
  pinMode(3,OUTPUT);
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
   // Serial.println("WiFi shield not present");
    while (true);
  }

  while ( status != WL_CONNECTED) {
    // Connect to WPA network
    status = WiFi.begin(ssid, pass);
    delay(5000);
  }
  
  //Starting connection to server
  Udp.begin(localPort);
}

void loop()
{
  if (stringComplete) {
    /*delay(1000);
    if(packetBuffer[15] == 0x6B){
      digitalWrite(3, HIGH);
    }
    Serial.write('A');
    if(packetBuffer[15] == 0x6B){
      delay(3000);
      digitalWrite(3, LOW);
    }
    stringComplete = false;*/
    sendNTPpacket(timeServer);
    delay(1000);
    if(Udp.parsePacket()){
      Udp.read(packetBuffer, NTP_PACKET_SIZE);
      if(packetBuffer[0] > 0x00){
        Serial.write('A');  //Send Ack to G55
        stringComplete = false;
      } 
    }    
  }
}


unsigned long sendNTPpacket(IPAddress& address)
{
  // set all bytes in the buffer to 0
 // memset(packetBuffer, 0, NTP_PACKET_SIZE);

  packetBuffer[0] = 0x00;  
  packetBuffer[1] = 0x00;    
  packetBuffer[2] = 0x00;     
  packetBuffer[3] = 0x77;  
  packetBuffer[4] = 0x00;
  packetBuffer[5] = 0x0A;
  packetBuffer[16] = 0x00;

  /*packetBuffer[6] = 0xAA;  
  packetBuffer[7] = 0x32;    
  packetBuffer[8] = 0x33;     
  packetBuffer[9] = 0x31;  
  packetBuffer[10] = 0x32;
  packetBuffer[11] = 0x33;
  packetBuffer[12] = 0x41;  
  packetBuffer[13] = 0x42;    
  packetBuffer[14] = 0x43;     
  packetBuffer[15] = 0x6B;  */
  
  Udp.beginPacket(address, AWS_Port); 
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}



void serialEvent() {
  if(stringComplete == false){
    while (Serial.available()) {    
      byte inChar = Serial.read();
        if(inChar == 0xAA){
          val = 1;
          packetBuffer[6] = 0xAA;
          //Serial.print(inChar, HEX);
        }
        else if (val > 0){
          if(inChar > 0x30){
            packetBuffer[val+6] = inChar;
            val++;
            if(val==10){
              stringComplete = true;
              val = 0;
            }
          }
        }
    }
  }
}









