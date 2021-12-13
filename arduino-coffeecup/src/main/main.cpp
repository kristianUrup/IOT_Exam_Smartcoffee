#include <Arduino.h>
#include <ArduinoMqttClient.h>
#include <WiFi.h>
#include <ArduinoSecrets.h>
#include <PubSubClient.h>
#include "Temperature.cpp"

const char* broker = "mqtt.flespi.io";

byte willQoS = 0;
const char* willTopic = "kristian";
const char* willMessage = "Frederik er lækker";
boolean willRetain = false;

WiFiClient espClient;
PubSubClient client(espClient);

void setupWifi() {
  delay(100);
  
  Serial.print("\nConnecting to ");
  Serial.println(SECRET_SSID);
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  while (WiFi.status() != WL_CONNECTED){
    delay(100);
    Serial.print("-");
  }

  Serial.print("\nConnected to ");
  Serial.println(SECRET_SSID);

}

void setup() {
  Serial.begin(115200);
  setupWifi();
  pinMode(LED_BUILTIN, HIGH);
    delay(300);
    
  client.setServer(broker, 1883);
}

void reconnect() {
  while(!client.connected()){
    Serial.println("\nConnecting to ");
    Serial.println(broker);
    Serial.println(client.connected());
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

void loop() {
  if(!client.connected()){
    reconnect();
  }
  delay(4000);
  client.connect("kristian", BROKER_USER, "");
  Temperature temp(1, '2','1','3','4');
  String output = temp.ToJsonObject();
  client.publish(willTopic, output.c_str());
  client.loop();
}
