#include <iostream>
#include <ArduinoJson.h>
class FillEvent {
    public:
        double VolumeFilled;

        FillEvent(double volumeFilled) {
            VolumeFilled = volumeFilled;
        };

        String ToJsonString() {
            String output;
            StaticJsonDocument<512> doc;
            doc["volumeFilled"] = VolumeFilled;
            serializeJson(doc, output);
            return output;
        };
};