#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <HCSR04.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "ZONG MBB-E5573-F0E4"
#define WIFI_PASSWORD "01909151"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDuyz7hXh38YlfYktuhxDv4rUfuDIE2KUo"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "nimraghazal04@gmail.com"
#define USER_PASSWORD "1234567"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "aqui-f94c5-default-rtdb.asia-southeast1.firebasedatabase.app"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

// Variables to save database paths
String WaterLevelPath;
//DATABASE CHILD NODES
String OverheadTankPath;
String distup_cmPath;
String distup_inchPath;


const int trigpin= 15;
const int echopin= 13;
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701
long duration;
float distanceupCm;
float distanceupInch;

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 10000;

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Write float values to the database
void sendFloat(String path, float value){
  if (Firebase.RTDB.setFloat(&fbdo, path.c_str(), value)){
    Serial.print("Writing value: ");
    Serial.print (value);
    Serial.print(" on the following path: ");
    Serial.println(path);
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}
void setup() {
  Serial.begin(115200);
  pinMode(trigpin, OUTPUT);
  pinMode(echopin, INPUT);
 
 initWiFi();
  // Assign the api key (required)
  config.api_key = API_KEY;

// Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
// Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
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
  Serial.println(uid);

   // Update database path
 WaterLevelPath = "/UsersData/" + uid;

  // Update database path for sensor readings
  distup_cmPath = WaterLevelPath + "/distanceupCm"; 
  distup_inchPath = WaterLevelPath + "/distanceupInch"; 
  }

void loop() {
  // Send new readings to database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

digitalWrite(trigpin,LOW);
delayMicroseconds(2);

digitalWrite(trigpin,HIGH);
delayMicroseconds(10);
digitalWrite(trigpin,LOW);

duration= pulseIn(echopin,HIGH);
distanceupCm =duration*SOUND_SPEED/2;

distanceupInch = distanceupCm *CM_TO_INCH;

Serial.print("Distance (cm): ");
Serial.println(distanceupCm);
Serial.print("Distance (inch): " );
Serial.println(distanceupInch);
delay(1000);

// Send readings to database:
 sendFloat(distup_cmPath, distanceupCm);
 sendFloat(distup_inchPath, distanceupInch);
  }
}
