#include <ArduinoJson.h>

class Settings {
    public: 
        int Interval;

    Settings(int interval) {
        Interval = interval;
    };

    Settings(String jsonString) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, jsonString);
        int interval = doc["interval"];

        Interval = interval;
    };
};