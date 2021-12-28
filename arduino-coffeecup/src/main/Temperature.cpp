#include <iostream>
#include "TemperatureBody.cpp"
#include <ArduinoJson.h>

class Temperature {
    public:
        String MachineId;  
        float Value;
        String MeasurementUnit;
        String SensorId;

        Temperature(String machineId, float value, String measurementUnit, String sensorId) {
            MachineId = machineId;
            Value = value;
            MeasurementUnit = measurementUnit;
            SensorId = sensorId;
        };

        String ToJsonString() {
            String output;
            StaticJsonDocument<512> doc;
            doc["id"] = MachineId;
            doc["sensorId"] = SensorId;
            doc["value"] = Value;
            doc["measurementUnit"] = MeasurementUnit;
            serializeJson(doc, output);
            return output;
        };
};