#include <Arduino.h>
#include <ArduinoMqttClient.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "Temperature.cpp"
#include "ArduinoSecrets.h"
#include "Arduinodata.h"

const char* broker = "mqtt.flespi.io";

byte willQoS = 0;
const char* willTopic = "kristian";
const char* willMessage = "Frederik er lækker";
boolean willRetain = false;

OneWire oneWire(TEMP_PIN);

DallasTemperature sensors(&oneWire);

WiFiClient espClient;
PubSubClient client(espClient);

void setupWifi() {
  delay(100);
  
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  while (WiFi.status() != WL_CONNECTED){
    delay(100);
    Serial.print("-");
  }

  Serial.print("\nConnected to: ");
  Serial.print(SECRET_SSID);
}

void reconnect() {
  while(!client.connected()){
    Serial.println("\nConnecting to ");
    Serial.println(broker);
    // Flespi doesn't care about the password, it only wants the token.
    // client(<clientId>, <username>, <password>)
    if(client.connect("kristian", FLESPI_TOKEN, "")){
      Serial.print("\nConnected to ");
      Serial.print(broker);
    } else {
      Serial.println("\nTrying again");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setupWifi();
  pinMode(LED_BUILTIN, HIGH);
  delay(3000);
    
  client.setServer(broker, 1883);
  sensors.begin();
}

void loop() {
  if(!client.connected()){
    reconnect();
  }  
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  Temperature temp(MACHINE_ID, temperatureC, "Celsius", TEMP_SENSOR_ID);
  String output = temp.ToJsonString();
  client.publish(willTopic, output.c_str());
  client.loop();
  delay(4000);
}