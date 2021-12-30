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
#include "CoffeeMessage.cpp"
#include "SipEvent.cpp"
#include "FillEvent.cpp"
#include "ArduinoSecrets.h"
#include "Arduinodata.h"

const char* broker = "mqtt.flespi.io";

double minTemp = 25;
int interval = 5000;
unsigned long previousMillis = 0;
boolean sipState = false;
double sipAngle;
double pouringAngle;
double distanceToCoffee;
double currentCoffeeVolume;
double prevCoffeeDistance = 0;
boolean pourState = false;
long pouringStarted = 0;

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
      client.subscribe(MACHINE_ID + "/settings-change");
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
  minTemp = settings.PrefMinTemp;
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

double getNormalizedLiquidDistance() {
  double totalDistanceSum = 0;
  for (int i = 0; i < 10; i++) {
    totalDistanceSum += getLiquidDistance();
    delay(10);
  }

  return totalDistanceSum / 10;
}

double calculateVolumeOfCoffee(double coffeeDistance) {
  return PI * (CUP_RADIUS * CUP_RADIUS) * coffeeDistance;
}

double calculateSippingAngle() {
  double sippingAngle = (atan2(distanceToCoffee, CUP_RADIUS) * RADIAN_TO_DEGREE) - SIP_ANGLE_OFFSET;
  if (sippingAngle >= MAX_SIP_ANGLE) {
    return MAX_SIP_ANGLE;
  } else {
    return sippingAngle;
  }
}

double calculatePouringAngle(double sippingAngle) {
  double pouringAngle = sippingAngle + 20;
  if (pouringAngle >= MAX_POURING_ANGLE) {
    return MAX_POURING_ANGLE;
  } else {
    return pouringAngle;
  }
}

void setSipAndPouringAngles() {
  double sippingAngle = calculateSippingAngle();
  sipAngle = sippingAngle;
  double pourgAngle = calculatePouringAngle(sippingAngle);
  pouringAngle = pourgAngle;

  Serial.print("Angle for sipping: ");
  Serial.println(sipAngle);
  Serial.print("Angle for pouring: ");
  Serial.println(pouringAngle);
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

  /*Serial.println("");
  Serial.print(roll);
  Serial.print(" ");
  Serial.print(pitch);
  Serial.println("");
  Serial.print(STATIC_ROLL);
  Serial.print(" ");
  Serial.print(STATIC_PITCH);
  Serial.println("");*/

  double absoluteDifferencePitch = abs(abs(STATIC_ROLL) - abs(roll));
  double absoluteDifferenceRoll = abs(abs(STATIC_PITCH) - abs(pitch));

  /*Serial.print("Sip angle: ");
  Serial.println(sipAngle);
  Serial.print("Absolute difference pitch: ");
  Serial.println(absoluteDifferencePitch);
  Serial.print("Absolute difference roll: ");
  Serial.println(absoluteDifferenceRoll);*/

  if(((absoluteDifferencePitch >= sipAngle) || (absoluteDifferenceRoll >= sipAngle)) && !sipState) {
    sipState = true;
    pourState = false;
    Serial.println("User is sipping");
  }
  if (!sipState) {
    return;
  }
  long sipStart = millis();
  double maxDrinkingAnkle = 0;

  while(sipState) {
    gyro.getEvent(&a, &g, &temp);
    x_Buff = float(a.acceleration.x);
    y_Buff = float(a.acceleration.y);
    z_Buff = float(a.acceleration.z);

    roll = atan2(y_Buff , z_Buff) * RADIAN_TO_DEGREE;
    pitch = atan2((- x_Buff) , sqrt(y_Buff * y_Buff + z_Buff * z_Buff)) * RADIAN_TO_DEGREE;

    absoluteDifferencePitch = abs(abs(STATIC_ROLL) - abs(roll));
    absoluteDifferenceRoll = abs(abs(STATIC_PITCH) - abs(pitch));

    if (absoluteDifferencePitch > maxDrinkingAnkle) {
      maxDrinkingAnkle = absoluteDifferencePitch;
    }

    if (absoluteDifferenceRoll > maxDrinkingAnkle) {
      maxDrinkingAnkle = absoluteDifferenceRoll;
    }

    /*Serial.print("Sip angle: ");
    Serial.println(sipAngle);
    Serial.print("Absolute difference pitch: ");
    Serial.println(absoluteDifferencePitch);
    Serial.print("Absolute difference roll: ");
    Serial.println(absoluteDifferenceRoll);*/

    if((absoluteDifferencePitch < STILL_ANGLE) && (absoluteDifferenceRoll < STILL_ANGLE)) {
      boolean isPourOut = false;
      long sipDuration = millis() - sipStart;
      if(maxDrinkingAnkle >= pouringAngle && sipDuration < MAX_POURING_TIME) {
        isPourOut = true;
      }
      delay(500);    
      sipState = false;
      double newDistanceToCoffee = getNormalizedLiquidDistance();
      double newDistance = CUP_HEIGHT - newDistanceToCoffee;
      double newVolume = calculateVolumeOfCoffee(newDistance);
      double volumeSipped = currentCoffeeVolume - newVolume;
      /*Serial.print("New distance to coffee: ");
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
      Serial.println("ml");*/

      currentCoffeeVolume = newVolume;
      distanceToCoffee = newDistanceToCoffee;
      setSipAndPouringAngles();
      SipEvent evt(volumeSipped, newVolume);
      String evtString = evt.ToJsonString();
      CoffeeMessage msg(MACHINE_ID, evtString);
      String msgString = msg.ToJsonString();

      if (isPourOut) {
        Serial.println("Has been poured");
        client.publish("smart-cup/event/coffee.poured", msgString.c_str()); 
      } else {        
        Serial.println("Coffee has been sipped");
        client.publish("smart-cup/event/coffee.sipped", msgString.c_str()); 
      }
    }
    delay(1000);
  }  
}

boolean isPouring() {
  double currentDistance = getNormalizedLiquidDistance();
  /*Serial.print("Current distance: ");
  Serial.println(currentDistance);
  Serial.print("Previous coffee distance: ");
  Serial.println(prevCoffeeDistance);*/
  double offset = 0.5;
  return currentDistance + offset < prevCoffeeDistance;
}

void handlePouring() {
  double offset = 0.5;
  if (millis() - pouringStarted >= 4000) {
    double currentDistance = getNormalizedLiquidDistance();
    if (currentDistance + offset < distanceToCoffee) {
      double currentVolume = calculateVolumeOfCoffee(CUP_HEIGHT - currentDistance);
      double oldVolume = calculateVolumeOfCoffee(CUP_HEIGHT - distanceToCoffee);

      double volumePoured = currentVolume - oldVolume;
      Serial.print("Pouring occured: ");
      Serial.print(volumePoured);
      Serial.println("ml");
      FillEvent evt(volumePoured);
      String evtStr = evt.ToJsonString();
      CoffeeMessage msg(MACHINE_ID, evtStr);
      String msgStr = msg.ToJsonString();
      client.publish("smart-cup/event/coffee.filled", msgStr.c_str());  
      distanceToCoffee = currentDistance;
      setSipAndPouringAngles();      
    }
    pourState = false;
  }
}

void handleTemperature() {
  temp_sensor.requestTemperatures();
  float temperatureC = temp_sensor.getTempCByIndex(0);
  Temperature temp(MACHINE_ID, temperatureC, "Celsius", temperatureC <= minTemp);
  String output = temp.ToJsonString();
  if (temp.Value > UNDEFINED_TEMP_VALUE) {    
    client.publish("smart-cup/temp", output.c_str());
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
  /*Serial.print("distance to coffee: ");
  Serial.println(distanceToCoffee);
  Serial.print("coffee height: ");
  Serial.println(coffeeHeight);
  Serial.print("volume of coffee: ");
  Serial.print(currentCoffeeVolume);
  Serial.println("ml");*/
  setSipAndPouringAngles();
    
  client.setServer(broker, 1883); 
  client.setCallback(settings_callback);
  temp_sensor.begin();

  previousMillis = millis();
}

void loop() {
  if(!client.connected()){
    reconnect();
  }
  long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    
    previousMillis = millis();
  }
  handleSipping();

  if (isPouring()) {
    Serial.println("Pouring started");
    pourState = true;
    pouringStarted = millis();
  }
  
  prevCoffeeDistance = getNormalizedLiquidDistance();

  if (pourState) {
    handlePouring();
  }
  /*Serial.print("Distance to coffee: ");
  Serial.print(getNormalizedLiquidDistance());
  Serial.println("cm");
  
  Serial.print("Coffee height: ");
  Serial.print(CUP_HEIGHT - getNormalizedLiquidDistance());
  Serial.println("cm");*/
  client.loop();
  delay(10);
}