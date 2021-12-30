#include <Arduino.h>
#include <ArduinoMqttClient.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <math.h>

#include "Temperature.cpp"
#include "Settings.cpp"
#include "ArduinoSecrets.h"
#include "Arduinodata.h"

const char* broker = "mqtt.flespi.io";

byte willQoS = 0;
const char* willTopic = "kristian";
const char* willMessage = "Frederik er lækker";
boolean willRetain = false;

int interval = 5000;
boolean sipState = false;
double sipAngle;
double distanceToCoffee;
double currentCoffeeVolume;

Adafruit_MPU6050 gyro;
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
      client.subscribe("settings");
    } else {
      Serial.println("\nTrying again");
      delay(5000);
    }
  }
}

void settings_callback(char* topic, byte* payload, unsigned int length) {
  String json = "";
  Serial.print("Topic: ");
  Serial.println(topic);
  for (int i = 0; i < length; i++) {
    json = json + (char)payload[i];
  }
  Serial.print(json);
  Settings settings(json);
  interval = settings.Interval;
  Serial.println();  
}

double getLiquidDistance() {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);  

  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
  double distance = duration * SOUND_SPEED / 2;

  return distance + ULTRASONIC_LENGTH;
}

double calculateVolumeOfCoffee(double coffeeDistance) {
  return PI * (CUP_RADIUS * CUP_RADIUS) * coffeeDistance;
}

void handleSipping() {
  sensors_event_t a, g, temp;
  gyro.getEvent(&a, &g, &temp);

  double roll = 0.00;
  double pitch = 0.00;
  double x_Buff = float(a.acceleration.x);
  double y_Buff = float(a.acceleration.y);
  double z_Buff = float(a.acceleration.z);
  roll = atan2(y_Buff, z_Buff) * RADIAN_TO_DEGREE;
  pitch = atan2((- x_Buff), sqrt(y_Buff * y_Buff + z_Buff * z_Buff)) * RADIAN_TO_DEGREE;

  Serial.println("");
  Serial.print(roll);
  Serial.print(" ");
  Serial.print(pitch);
  Serial.println("");
  Serial.print(STATIC_ROLL);
  Serial.print(" ");
  Serial.print(STATIC_PITCH);
  Serial.println("");

  double absoluteDifferencePitch = abs(abs(STATIC_ROLL) - abs(roll));
  double absoluteDifferenceRoll = abs(abs(STATIC_PITCH) - abs(pitch));

  Serial.print("Sip angle: ");
  Serial.println(sipAngle);
  Serial.print("Absolute difference pitch: ");
  Serial.println(absoluteDifferencePitch);
  Serial.print("Absolute difference roll: ");
  Serial.println(absoluteDifferenceRoll);

  if(((absoluteDifferencePitch >= sipAngle) || (absoluteDifferenceRoll >= sipAngle)) && !sipState) {
    sipState = true;
    Serial.println("User is sipping");
  }

  while(sipState) {
    gyro.getEvent(&a, &g, &temp);
    x_Buff = float(a.acceleration.x);
    y_Buff = float(a.acceleration.y);
    z_Buff = float(a.acceleration.z);

    roll = atan2(y_Buff , z_Buff) * RADIAN_TO_DEGREE;
    pitch = atan2((- x_Buff) , sqrt(y_Buff * y_Buff + z_Buff * z_Buff)) * RADIAN_TO_DEGREE;

    absoluteDifferencePitch = abs(abs(STATIC_ROLL) - abs(roll));
    absoluteDifferenceRoll = abs(abs(STATIC_PITCH) - abs(pitch));

    Serial.print("Sip angle: ");
    Serial.println(sipAngle);
    Serial.print("Absolute difference pitch: ");
    Serial.println(absoluteDifferencePitch);
    Serial.print("Absolute difference roll: ");
    Serial.println(absoluteDifferenceRoll);

    if((absoluteDifferencePitch < STILL_ANGLE) && (absoluteDifferenceRoll < STILL_ANGLE)) {
      delay(500);    
      sipState = false;
      Serial.println("Sip is done");
      double newDistanceToCoffee = getLiquidDistance();
      double newDistance = CUP_HEIGHT - newDistanceToCoffee;
      double newVolume = calculateVolumeOfCoffee(newDistance);
      double volumeSipped = currentCoffeeVolume - newVolume;
      Serial.print("New distance to coffee: ");
      Serial.println(newDistanceToCoffee);
      Serial.print("New distance: ");
      Serial.println(newDistance);
      Serial.print("Old distance: ");
      Serial.println(distanceToCoffee);
      Serial.print("New volume: ");
      Serial.print(newVolume);
      Serial.println("ml");
      Serial.print("Volume sipped: ");
      Serial.print(volumeSipped);
      Serial.println("ml");
      currentCoffeeVolume = newVolume;
      distanceToCoffee = newDistanceToCoffee;
      sipAngle = (atan2(distanceToCoffee, CUP_RADIUS) * RADIAN_TO_DEGREE) - SIP_ANGLE_OFFSET;
    }
    delay(1000);
  }  
}

void setup() {
  Serial.begin(115200);

  if (!gyro.begin()) {
    Serial.println("Could not find sensor");
  } else {
    Serial.println("Sensor found");
  }

  setupWifi();
  delay(3000);
  gyro.setAccelerometerRange(MPU6050_RANGE_8_G);
  gyro.setGyroRange(MPU6050_RANGE_500_DEG);
  gyro.setFilterBandwidth(MPU6050_BAND_5_HZ);

  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  
  distanceToCoffee = getLiquidDistance();
  double coffeeHeight = CUP_HEIGHT - distanceToCoffee;
  currentCoffeeVolume = calculateVolumeOfCoffee(coffeeHeight);  
  Serial.print("distance to coffee: ");
  Serial.println(distanceToCoffee);
  Serial.print("coffee height: ");
  Serial.println(coffeeHeight);
  Serial.print("volume of coffee: ");
  Serial.print(currentCoffeeVolume);
  Serial.println("ml");
  sipAngle = (atan2(distanceToCoffee, CUP_RADIUS) * RADIAN_TO_DEGREE) - SIP_ANGLE_OFFSET;

  Serial.print("Angle for sipping: ");
  Serial.println(sipAngle);
    
  client.setServer(broker, 1883); 
  client.setCallback(settings_callback);
  temp_sensor.begin();
}

void loop() {
  if(!client.connected()){
    reconnect();
  }

  handleSipping();
  Serial.print(getLiquidDistance());
  Serial.println("cm");

  /*temp_sensor.requestTemperatures();
  float temperatureC = temp_sensor.getTempCByIndex(0);
  Temperature temp(MACHINE_ID, temperatureC, "Celsius");
  String output = temp.ToJsonString();

  client.publish(willTopic, output.c_str());
  client.loop();

  delay(interval);*/
  delay(1000);
}