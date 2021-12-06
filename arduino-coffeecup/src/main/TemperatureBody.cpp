#include <iostream>;

class TemperatureBody {
    public: 
    char Value;
    char MeasurementUnit;
    char TimeStamp;
    char SensorId;

    TemperatureBody() = default;

    TemperatureBody(char value, char measurementUnit, char timeStamp, char sensorId){
        Value = value;
        MeasurementUnit = measurementUnit;
        TimeStamp = timeStamp;
        SensorId = sensorId;
    }

};