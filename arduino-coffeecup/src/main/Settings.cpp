#include <ArduinoJson.h>

class Settings {
    public: 
        int Interval;
        int PrefMinTemp;

    Settings(int interval, int prefMinTemp) {
        Interval = interval;
        PrefMinTemp = prefMinTemp;
    };

    Settings(String jsonString) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, jsonString);
        int interval = doc["interval"];
        int prefMinTemp = doc["prefMinTemp"];

        Interval = interval;
        PrefMinTemp = prefMinTemp;
    };
};