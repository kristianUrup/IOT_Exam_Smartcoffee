#include <iostream>
#include <ArduinoJson.h>

class CoffeeEvent {
    public:
        String MachineId;
        double VolumeDiff;
        double CurrentVolume;


    CoffeeEvent(String machineId, double volumeDiff, double currentVolume) {
        MachineId = machineId;
        VolumeDiff = volumeDiff;
        CurrentVolume = currentVolume;
    }

    String ToJsonString() {
        String output;
        StaticJsonDocument<512> doc;
        doc["id"] = MachineId;
        doc["valueDiff"] = VolumeDiff;
        doc["currentVolume"] = CurrentVolume;
        serializeJson(doc, output);
        return output;
    }
}

