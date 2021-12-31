#include <iostream>
#include <ArduinoJson.h>

class PourEvent {
    public:
        double VolumePoured;
        double CurrentVolume;

        PourEvent(double volumePoured, double currentVolume) {
            VolumePoured = volumePoured;
            CurrentVolume = currentVolume;
        };

        String ToJsonString() {
            String output;
            StaticJsonDocument<512> doc;
            doc["volumePoured"] = VolumePoured;
            doc["currentVolume"] = CurrentVolume;
            serializeJson(doc, output);
            return output;
        };
};