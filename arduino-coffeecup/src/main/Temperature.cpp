#include <iostream>
#include "TemperatureBody.cpp"

class Temperature {
    public:
    int Id;  
    TemperatureBody Body;

    Temperature(int id, char value, char measurementUnit, char timeStamp, char sensorId) {
        Id = id;
        Body = TemperatureBody(value, measurementUnit, timeStamp, sensorId);
    };
};