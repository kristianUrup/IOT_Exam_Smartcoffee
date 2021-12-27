#include <iostream>
#include "TemperatureBody.cpp"
#include <ArduinoJson.h>

class Temperature {
    public:
        String Id;  
        float Value;
        String MeasurementUnit;
        String TimeStamp;
        String SensorId;

        Temperature(String id, float value, String measurementUnit, String timeStamp, String sensorId) {
            Id = id;
            Value = value;
            MeasurementUnit = measurementUnit;
            TimeStamp = timeStamp;
            SensorId = sensorId;
        };

        String ToJsonString() {
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