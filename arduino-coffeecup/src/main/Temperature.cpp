#include <iostream>
#include <ArduinoJson.h>

class Temperature {
    public:
        String MachineId;  
        float Value;
        String MeasurementUnit;

        Temperature(String machineId, float value, String measurementUnit) {
            MachineId = machineId;
            Value = value;
            MeasurementUnit = measurementUnit;
        };

        String ToJsonString() {
            String output;
            StaticJsonDocument<512> doc;
            doc["id"] = MachineId;
            doc["value"] = Value;
            doc["measurementUnit"] = MeasurementUnit;
            serializeJson(doc, output);
            return output;
        };
};