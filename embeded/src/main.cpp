#include <Arduino.h>
#include <Wifi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "env.h" 

#define endpoint "project-2024-he7v.onrender.com"

#define fanPin 23
#define lightPin 22
#define pirPin 15
#define tempPin 4

OneWire oneWire(tempPin);
DallasTemperature sensors(&oneWire);


/*
//testing
float getTemp(){

  return random(21.1,33.1);
}

bool getpresence(){

  return random(0,1);
}
*/
float getTemp(){
  sensors.requestTemperatures(); 
  
  Serial.print("Celsius temperature: ");
  Serial.print(sensors.getTempCByIndex(0)); 
  Serial.print(" - Fahrenheit temperature: ");
  Serial.println(sensors.getTempFByIndex(0));
  return sensors.getTempCByIndex(0);
}

bool getpresence(){

  int pirState = digitalRead(pirPin); 
  if (pirState == HIGH) {
    // Motion detected
    Serial.println("Motion detected!");
  } else {
    Serial.println("No motion detected.");
  }
  return pirState;
}

void setup() {

  Serial.begin(9600);

	pinMode(fanPin,OUTPUT);
  pinMode(lightPin,OUTPUT);  
  pinMode(pirPin, INPUT);  
  
  sensors.begin();


	// WiFi_SSID and WIFI_PASS should be stored in the env.h
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("");
	// Connect to wifi
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  //Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){
    Serial.println("");
    Serial.println("");

    HTTPClient http;
  
    // Establish a connection to the server
    String url = "https://" + String(endpoint) + "/info";
    http.begin(url);
    http.addHeader("Content-type", "application/json");

    JsonDocument docput;
    String httpRequestData;

    // Serialise JSON object into a string to be sent to the API
  

    docput["temperature"] = getTemp();
    docput["presence"] = getpresence();
  

    // convert JSON document, doc, to string and copies it into httpRequestData
    serializeJson(docput, httpRequestData);

    // Send HTTP PUT request
    int httpResponseCode = http.POST(httpRequestData);
    String http_response;

    // check reuslt of PUT request. negative response code means server wasn't reached
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      Serial.print("HTTP Response from server: ");
      http_response = http.getString();
      Serial.println(http_response);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();


    url = "https://" + String(endpoint) + "/state";    
    http.begin(url);
    httpResponseCode = http.GET();

    Serial.println("");
    Serial.println("");

    if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);

        Serial.print("Response from server: ");
        http_response = http.getString();
        Serial.println(http_response);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
 
    JsonDocument docget;

    DeserializationError error = deserializeJson(docget, http_response);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }
    
    bool temp = docget["fan"]; 
    bool light= docget["light"]; 


  Serial.print("temperature ");

    digitalWrite(fanPin,temp);
    digitalWrite(lightPin,light);
    
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}






