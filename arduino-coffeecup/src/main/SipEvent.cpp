#include <iostream>
#include <ArduinoJson.h>

class SipEvent {
    public:
        double VolumeSipped;
        double CurrentVolume;

        SipEvent(double volumeSipped, double currentVolume) {
            VolumeSipped = volumeSipped;
            CurrentVolume = currentVolume;
        };

        String ToJsonString() {
            String output;
            StaticJsonDocument<512> doc;
            doc["volumeSipped"] = VolumeSipped;
            doc["currentVolume"] = CurrentVolume;
            serializeJson(doc, output);
            return output;
        };
};