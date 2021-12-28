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
DallasTemperature temp_sensor(&oneWire);

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
  delay(3000);

  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
    
  client.setServer(broker, 1883);
  temp_sensor.begin();
}

float getLiquidAmount() {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
  float distance = duration * SOUND_SPEED / 2;

  return distance;
}

void loop() {
  if(!client.connected()){
    reconnect();
  }

  float distance = getLiquidAmount();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println("cm");

  temp_sensor.requestTemperatures();
  float temperatureC = temp_sensor.getTempCByIndex(0);
  Temperature temp(MACHINE_ID, temperatureC, "Celsius", TEMP_SENSOR_ID);
  String output = temp.ToJsonString();

  client.publish(willTopic, output.c_str());
  client.loop();

  delay(4000);
}