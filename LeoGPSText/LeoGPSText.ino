/*

This example is meant to work with the Adafruit FONA 808v2,
utilizing the Arduino Leonardo. Hardware changes to the serial
pins/traces on the 808 are required for this to work.
Otherwise, a standard arduino can be used by changing the
pin numbers below.

Requires Adafruit_FONA libraries.

THIS IS A WORK IN PROGRESS. USE AT YOUR OWN RISK.

*/
#include "Adafruit_FONA.h"

// standard pins for the shield
//  #define FONA_RX 2
//  #define FONA_TX 3
//  #define FONA_RST 4

// pins for the Leonardo hardware mod
#define FONA_RX 8
#define FONA_TX 9
#define FONA_RST 4

// We default to using software serial. If you want to use hardware serial
// (because softserial isnt supported) comment out the following three lines 
// and uncomment the HardwareSerial line
#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

// Hardware serial is also possible!
//  HardwareSerial *fonaSerial = &Serial1;

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

// Have a FONA 3G? use this object type instead
//Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);

// Buffers for receiving and sending SMS messages
char replymsg[160];
char recmsgbuff[255];

void setup() {

// Reboot the arduino until the FONA processor is available
 fonaSerial->begin(4800);
 if (! fona.begin(*fonaSerial)) {
   Serial.println(F("Couldn't find FONA"));
   while(1);
 }

delay(5000);
 
  // turn GPS on by default
 if (!fona.enableGPS(true))
   Serial.println(F("Failed to turn on GPS"));

 delay(2000);

}

void loop() {

 delay(1500);

 // Only read the first text.
 // The program will later delete this text, so only the the first stored SMS should be relevant.
    uint8_t readsmsnum = 1;
    
 // Change smskeyword to define the "reply" keyword.
 // As is, if someone sends an SMS that says "loc" to the unit, it will
 // send a reply, otherwise it will simply delete the text.
    char smskeyword[10] = "loc";

    char sendernum[13]; // Variable to store the sender's number, obtained from SMS #1

 // read the number of SMS's
    int8_t smscount = fona.getNumSMS();
    if (smscount < 1) {
      Serial.println(F("Could not read # SMS"));
      return;
    } else {
      Serial.print(smscount);
      Serial.println(F(" SMS's on SIM card!"));
    }
    
 // read incoming SMS
    Serial.print(F("Read #"));
    Serial.print(F("\n\rReading SMS #"));
    Serial.println(readsmsnum);
    // Retrieve SMS sender address/phone number.
    if (!fona.getSMSSender(readsmsnum, sendernum, 250)) {
      Serial.println("Failed!");
      return;
    }
    Serial.print(F("FROM: ")); Serial.println(sendernum);
    // Retrieve SMS value.
    uint16_t readsmslen;
    if (!fona.readSMS(readsmsnum, recmsgbuff, 250, &readsmslen)) { // pass in buffer and max len!
      Serial.println("Failed to retrieve text.");
      return;
    }
    Serial.print(F("***** SMS #")); Serial.print(readsmsnum);
    Serial.print(" ("); Serial.print(readsmslen); Serial.println(F(") bytes *****"));
    Serial.println(recmsgbuff);
    Serial.println(F("*****"));

 // delete SMS after being read, regardless of the contents
      Serial.println(F("Deleting SMS..."));
    if (!deleteSMSRoutine(smscount)) {
      Serial.println(F("Something went wrong"));
   } else {
      Serial.println(F("All SMS deleted!"));
   }

    
 // Look for a keyword match from the sender. If the sender text is the keyword, reply with battery percent and location.
 // Change this to a != statement, use guard if statements and returns (instead of breaks) from now on
    if (strcasecmp(recmsgbuff, smskeyword) != 0) {
      return;
    }
    
    // read the battery percentage, place into string for SMS reply
       uint16_t vbat;
       char vbattc[4];
       if (!fona.getBattPercent(&vbat)) {
         strcpy(replymsg, "??% battery\n");
       } else {
         dtostrf(vbat, 3, 0, vbattc);
         strcpy(replymsg, vbattc); strcat(replymsg, "% battery\n");
         Serial.print(replymsg);
       }
       
    // check for GPS location, place into string for SMS reply
       float latitude, longitude;
       char latc[11];
       char longc[11];
       boolean gps_success = fona.getGPS(&latitude, &longitude);
       if (gps_success) {
         strcat(replymsg, "Coordinates:\n");
         dtostrf(latitude, 10, 6, latc);
         strcat(replymsg, latc);
         strcat(replymsg, ",");
         dtostrf(longitude, 10, 6, longc);
         strcat(replymsg, longc);
       } else {
         strcat(replymsg, "Waiting for FONA GPS 3D fix...");
       }
       
     //send an SMS reply
       Serial.print(F("Sending ")); Serial.print(replymsg); Serial.print(F(" to ")); Serial.print(sendernum);
       if (!fona.sendSMS(sendernum, replymsg)) {
         Serial.println(F("SMS send Failed"));
       } else {
         Serial.println(F("SMS Sent!"));
       }
    
    return;
}


void flushSerial() {
  while (Serial.available())
    Serial.read();
}

bool deleteSMSRoutine(uint16_t deletecount) {
  for (uint16_t i = 1; i <= deletecount; i++) {
    if (!fona.deleteSMS(i)) {
     Serial.print(F("Couldn't delete SMS ")); Serial.println(i);
     return 0;
    } else {
     Serial.print(F("Deleted SMS ")); Serial.println(i);
    }
  }
  return 1;
}
