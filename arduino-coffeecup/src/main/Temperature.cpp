#include <iostream>
#include "TemperatureBody.cpp"
#include <ArduinoJson.h>

class Temperature {
    public:
        String Id;  
        float Value;
        String MeasurementUnit;
        String SensorId;

        Temperature(String id, float value, String measurementUnit, String sensorId) {
            Id = id;
            Value = value;
            MeasurementUnit = measurementUnit;
            SensorId = sensorId;
        };

        String ToJsonString() {
            String output;
            StaticJsonDocument<512> doc;
            doc["id"] = Id;
            doc["sensorId"] = SensorId;
            doc["value"] = Value;
            doc["measurementUnit"] = MeasurementUnit;
            serializeJson(doc, output);
            return output;
        };
};