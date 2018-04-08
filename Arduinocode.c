/**************************************************************************/

#include <Wire.h>
#include <Adafruit_PN532.h>
#include <string.h>

#define PN532_IRQ   (2)
#define PN532_RESET (3)  

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

#if defined(ARDUINO_ARCH_SAMD)
   #define Serial SerialUSB
#endif

uint8_t pw;


void setup(void) {
  #ifndef ESP8266
    while (!Serial); 
  #endif
  Serial.begin(115200);
  //Serial.println("Hello!");

  nfc.begin();
  pw = 0;
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    //Serial.print("Didn't find PN53x board");
    while (1); 
  }

  //Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  //Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  //Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  nfc.SAMConfig();
  
  //Serial.println("Waiting for an ISO14443A Card ...");
}


void loop(void) {
  uint8_t writemode = 0;
  uint8_t data_SN[16] = {1, 2, 3, 4, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t data_ZN[16] = {6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t data_ZT[16] = {9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  uint8_t t_ZN[2] = {0, 0};
  uint8_t t_ZT[2] = {0, 0};
  String in_data = Serial.readString();
  if(in_data[0] == 'S'){
    
    writemode = 1;
    
    uint8_t p = 1;
    uint8_t in_data2;

    uint8_t k;
    for(k=0;k<6;k++){
      data_SN[k] = in_data[p];
      p++;
    }
       
    p++;

    t_ZN[1] = in_data[p] - 48;
    p++;

    if(in_data[p] != '+'){
      t_ZN[0] = in_data[p] - 48;
      data_ZN[0] = t_ZN[1] * 10 + t_ZN[0];
    }
    else{
      data_ZN[0] = t_ZN[1];
    }

    p++;

    t_ZT[1] = in_data[p] - 48;
    p++;
    
    if(in_data[p] != '+'){
      t_ZT[0] = in_data[p] - 48;
      data_ZT[0] = t_ZT[1]*10 + t_ZT[0];
    }
    else{
      data_ZT[0] = t_ZT[1];
    }

 }


  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  
  uint8_t uidLength;                        
    
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
           
    if (uidLength == 4)
    {
          /* Serial.print("UID = ");
           Serial.print(uid[0]); 
           Serial.print(" ");
           Serial.print(uid[1]);
           Serial.print(" ");
           Serial.print(uid[2]);
           Serial.print(" ");
           Serial.print(uid[3]);
           Serial.println("");*/
 
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };    
      uint8_t newkey[6] = {0, 0, 0, 0, 0, 0};
      int u;
      uint8_t key = 0xBC;
      newkey[0] = uid[0];
      newkey[1] = uid[1];
      newkey[2] = uid[2];
      newkey[3] = uid[3];
      newkey[4] = 0xAC;
      newkey[5] = 0x08;
      for(u=0;u<6;u++){
        newkey[u] = newkey[u] ^ key;
      }

      if( pw == 0){
        success = nfc.AuthenticateBlock(uid, uidLength, 4, 0, newkey);
        if(success){
           //Serial.println("New password worked");
        }
        else if (success == 0){
          //Serial.println("New password did not work");
          pw = 1;
        }
      }
      
      else if(pw == 1){
        pw = 0;
        success = nfc.AuthenticateBlock(uid, uidLength, 4, 0, keya);
        if(success){
          //Serial.println("old password worked");
        
        //write in new password
        uint8_t data[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
         data[0] = newkey[0];
         data[1] = newkey[1];
         data[2] = newkey[2];
         data[3] = newkey[3];
         data[4] = newkey[4];
         data[5] = newkey[5];
         data[6] = 0x7F;
         data[7] = 0x07;
         data[8] = 0x88;
         data[9] = 0x69;
         data[10] = newkey[0];
         data[11] = newkey[1];
         data[12] = newkey[2];
         data[13] = newkey[3];
         data[14] = newkey[4];
         data[15] = newkey[5];
         success = nfc.WriteDataBlock (7, data);
        }
        else{
         // Serial.println("Old password did not work");
        }
        success = 0;
      }
    
      if (success)
      {
        uint8_t data[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        //memcpy(data, (const uint8_t[]){ 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0x7F, 0x07, 0x88, 0x69, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB }, sizeof data);

           if(writemode == 1){
              success = nfc.WriteDataBlock (5, data_ZN);
              if(success){
                  success = nfc.WriteDataBlock (6, data_ZT);
              }
              if(success){
                  success = nfc.WriteDataBlock (4, data_SN);
              }
           }


        /*if(success){
          success = nfc.ReadDataBlock(5, data_ZN);
          Serial.print("ZN ");
          Serial.println(data_ZN[0]);
        }
        if(success){
          success = nfc.ReadDataBlock(6, data_ZT);
         Serial.print("ZT ");
         Serial.println(data_ZT[0]);
        }
        if(success){
          success = nfc.ReadDataBlock(4, data_SN);
          Serial.print("SN ");
          for(u=0;u<6;u++){
           Serial.print(data_SN[u]);
            Serial.print(" ");
          }
        }
        Serial.println("");*/
        if ((success==1)&&(writemode==1))
        {       
            //send ack back to gui
            Serial.println("G");
        }
        else if (writemode==1)
        {
          Serial.println("B");
       //   unable to read the block
        }
      }
      else if (writemode==1)
      {
       Serial.println("B");
      }
    }
  }
  
}
