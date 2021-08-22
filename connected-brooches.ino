#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>

#include <ArduinoJson.h>
#include <WiFiManager.h>
//#include <FirebaseArduino.h>

#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#include <Adafruit_NeoPixel.h>

#define API_KEY "..."

#define DATABASE_URL "..."

#define USER_EMAIL "..."
#define USER_PASSWORD "..."

#define BROOCH_ID "..."
#define PAIR_BROOCH_ID "..."

#define LED_PIN 14
#define BRIGHTNESS 50
#define BUTTON_PIN 12

Adafruit_NeoPixel strip(1, LED_PIN, NEO_GRBW + NEO_KHZ800);
WiFiManager wifiManager;

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

int currentState = 0;

bool sentState = false;

const int BROOCH_OPEN = 1;
const int BROOCH_CLOSED = 2;
const int PAIR_BROOCH_CLOSED = 3;
const int PAIR_BROOCH_OPEN = 4;

void setup() {

   Serial.begin(9600);
   
   pinMode(BUTTON_PIN, INPUT_PULLUP);
 
  if(!wifiManager.autoConnect("Connected Brooches")) {
    delay(1000);
    Serial.println("failed to connect and hit timeout");
    ESP.reset();
    delay(1000);
  };

  Serial.println("Connected");


    /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = "...";

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);
  
  strip.begin();
  turnOffLED();

  if(isBroochBeingWorn()) {
     Serial.println("SETUP - BROOCH IS CLOSED");
    currentState = BROOCH_CLOSED;
    setBroochState(BROOCH_ID, 1);
  } else {
    Serial.println("SETUP - BROOCH IS OPEN");
    currentState = BROOCH_OPEN;
    setBroochState(BROOCH_ID, 0);
  }
}

void loop() {
  if(currentState == BROOCH_OPEN) {
    //Check if pin is closed
    if(isBroochBeingWorn()) {
      Serial.println("BROOCH WAS CLOSED");
      setBroochState(BROOCH_ID, 1);
      currentState = BROOCH_CLOSED;
    } else {
      Serial.println("BROOCH WAS OPEN");
      delay(2000);
    }
  } else if(currentState == BROOCH_CLOSED) {
    Serial.println("BROOCH CLOSED");

    if(!isBroochBeingWorn()) {
      setBroochState(BROOCH_ID, 0);
      currentState = BROOCH_OPEN;
    } else {
      bool pairState = getBroochState(PAIR_BROOCH_ID);
      if(pairState == 1) {
        turnOnLED();
        currentState = PAIR_BROOCH_CLOSED;
      } else {
        currentState = PAIR_BROOCH_OPEN;
      }
    }
  } else if(currentState == PAIR_BROOCH_CLOSED) {
    Serial.println("PAIR BROOCH CLOSED");

    if(!isBroochBeingWorn()) {
      turnOffLED();
      setBroochState(BROOCH_ID, 0);
      currentState = BROOCH_OPEN;
    } else {
    
    if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
      Serial.println("A");
      sendDataPrevMillis = millis();
      bool pairState = getBroochState(PAIR_BROOCH_ID);
      Serial.println(pairState);
      if(pairState == 0) {
        Serial.println(pairState);
        turnOffLED();
        currentState = PAIR_BROOCH_OPEN;
      }
    } else {
      Serial.println("B");
      delay(15000);
    }
    }
  } else if(currentState == PAIR_BROOCH_OPEN) {
    Serial.println("PAIR BROOCH OPEN");

    if(!isBroochBeingWorn()) {
      setBroochState(BROOCH_ID, 0);
      currentState = BROOCH_OPEN;
    } else {
    
      if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
        sendDataPrevMillis = millis();
        bool pairState = getBroochState(PAIR_BROOCH_ID);
        if(pairState == 1) {
          turnOnLED();
          currentState = PAIR_BROOCH_CLOSED;
        }
      } else {
         delay(15000);
      }
    }
  }
  
}

int getBroochState(char *broochID) {
  if(!Firebase.ready())
    return -1;

  if(!Firebase.RTDB.getInt(&fbdo, broochID) || fbdo.dataType() != "int")
    return -1;
    
  return fbdo.intData();
}

bool setBroochState(char *broochID, int state) {
  if(!Firebase.ready())
    return false;

   Firebase.RTDB.setInt(&fbdo, broochID, state);
}


void turnOnLED() {
  strip.setPixelColor(0, strip.Color(125, 125, 125, 125));
 strip.show();
}

void turnOffLED() {
  strip.setPixelColor(0, strip.Color(1, 1, 1, 1));
  strip.show();
}

void checkIfPairIsTurnedOn() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    bool pairState = getBroochState(PAIR_BROOCH_ID);
    if(currentState == PAIR_BROOCH_CLOSED) {
      
    } else if (currentState == PAIR_BROOCH_OPEN) {
      
    }
  }
}

bool isBroochBeingWorn() {
  bool state = digitalRead(BUTTON_PIN);
  return !state;
}
