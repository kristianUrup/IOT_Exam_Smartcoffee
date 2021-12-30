#include <iostream>
#include <ArduinoJson.h>

class CoffeeMessage {
    public:
        String MachineId;
        String MessageBody;

        CoffeeMessage(String machineId, String messageBody) {
            MachineId = machineId;
            MessageBody = messageBody;
        };

        String ToJsonString() {
            String output;
            StaticJsonDocument<512> doc;
            doc["id"] = MachineId;
            doc["messageBody"] = MessageBody;
            serializeJson(doc, output);
            return output;
        };
};

