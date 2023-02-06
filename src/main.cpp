/*
  Rui Santos
  Complete project details at our blog.
    - ESP32: https://RandomNerdTutorials.com/esp32-firebase-realtime-database/
    - ESP8266: https://RandomNerdTutorials.com/esp8266-nodemcu-firebase-realtime-database/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  Based in the RTDB Basic Example by Firebase-ESP-Client library by mobizt
  https://github.com/mobizt/Firebase-ESP-Client/blob/main/examples/RTDB/Basic/Basic.ino
*/

#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <SPI.h>
#include "Adafruit_SHT31.h"

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
/*
#define WIFI_SSID "Student"
#define WIFI_PASSWORD "Kristiania1914"
*/
#define WIFI_SSID "MaxChillOutCrib"
#define WIFI_PASSWORD "Ch1ll3rn!"

// Insert Firebase project API Key
#define API_KEY "AIzaSyCy3-DtGtHhTE3ouTFzL_Lp9ku-epRHGnI"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://greenhouse-monitor-cdc89-default-rtdb.europe-west1.firebasedatabase.app/" 

#define USER_EMAIL "segb1337@gmail.com"
#define USER_PASSWORD "test123"

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String uid;

unsigned long sendDataPrevMillis = 0; //Handles interval of sending data to Firebase
int count = 0;

Adafruit_SHT31 sht31 = Adafruit_SHT31();

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void setup(){
  Serial.begin(115200);
  initWiFi();

  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up for anonumous user
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  */

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.print(uid);
}
// Handle data recieved from ESP32S3, together with interval for reading data
float t, h;
unsigned long recieveDataPrevMillis = 0;

void loop(){
  // Renew token for authetification when it expires
  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Serial.println("Refresh token");
  }
  // Control how often we read temperature from ESP32S3 and if it reads temperature or not
  if((millis() - recieveDataPrevMillis > 14500 || recieveDataPrevMillis == 0)){
    recieveDataPrevMillis = millis();
    t = sht31.readTemperature();
    h = sht31.readHumidity();

    if (! isnan(t)) {  // check if 'is not a number'
      Serial.print("Reading from ESP32S3, temp *C = "); Serial.println(t);
    } else { 
      Serial.println("Failed to read temperature");
    }
    /* Humidity 
    if (! isnan(h)) {  // check if 'is not a number'
      Serial.print("Hum. % = "); Serial.println(h);
    } else { 
      Serial.println("Failed to read humidity");
    }
    */
  }

  // Post data from ESP32S3 to Firebase realtime database as long as user is authenticated
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    /* Write an Int number on the database path test/int
    if (Firebase.RTDB.setInt(&fbdo, "test/int", count)){
      Serial.println("PASSED");
      Serial.print("PATH: ");
      Serial.println(fbdo.dataPath());
      Serial.print("TYPE: ");
      Serial.println(fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.print("REASON:");
      Serial.println(fbdo.errorReason());
    }
    count++;
    */

    // Write an Float number on the database path test/float, 0.01 + random(0,100)
    if (Firebase.RTDB.setFloat(&fbdo, "test/temperature", t)){
      Serial.println("PASSED uploading to RTDB");
      Serial.print("PATH: "); Serial.print(fbdo.dataPath()); 
      Serial.print(" - TYPE: "); Serial.println(fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.print("REASON:"); Serial.println(fbdo.errorReason());
    }
  }
}