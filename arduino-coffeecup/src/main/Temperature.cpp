#include <iostream>
#include <ArduinoJson.h>

class Temperature {
    public:
        String MachineId;  
        float Value;
        String MeasurementUnit;
        boolean TooCold;

        Temperature(String machineId, float value, String measurementUnit, boolean tooCold) {
            MachineId = machineId;
            Value = value;
            MeasurementUnit = measurementUnit;
            TooCold = tooCold;
        };

        String ToJsonString() {
            String output;
            StaticJsonDocument<512> doc;
            doc["id"] = MachineId;
            doc["value"] = Value;
            doc["measurementUnit"] = MeasurementUnit;
            doc["tooCold"] = TooCold;
            serializeJson(doc, output);
            return output;
        };
};