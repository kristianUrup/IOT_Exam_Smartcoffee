#include <iostream>
#include "TemperatureBody.cpp"
#include <ArduinoJson.h>

class Temperature {
    public:
        int Id;  
        char Value;
        char MeasurementUnit;
        char TimeStamp;
        char SensorId;

        Temperature(int id, char value, char measurementUnit, char timeStamp, char sensorId) {
            Id = id;
            Value = value;
            MeasurementUnit = measurementUnit;
            TimeStamp = timeStamp;
            SensorId = SensorId;
        };

        String ToJsonObject() {
            String output;
            StaticJsonDocument<512> doc;
            doc["id"] = Id;
            doc["sensorId"] = SensorId;
            doc["value"] = Value;
            doc["measurementUnit"] = MeasurementUnit;
            doc["timeStamp"] = TimeStamp;
            serializeJson(doc, output);
            return output;
        };
};