
#include <string>

using namespace std;

class WeatherStation {
    public:
        string Id;
        float Battery;
        char BatteryUnit;
        float WindSpeedMin; //Sn
        char WindSpeedMinUnit;
        float WindSpeedAvg; //Sm
        char WindSpeedAvgUnit;
        float WindSpeedMax; //Sx
        char WindSpeedMaxUnit;
        float WindDirMin; //Dn
        char WindDirMinUnit;
        float WindDirAvg; //Dm
        char WindDirAvgUnit;
        float WindDirMax; //Dx
        char WindDirMaxUnit;
        float AirPressure; //Pa
        char AirPressureUnit;
        float AirTemperature; //Ta
        char AirTemperatureUnit;
        float InternalTemperature; //Tp
        char InternalTemperatureUnit;
        float RelativeHumidity; //Ua
        char RelativeHumidityUnit;
        float RainAcc; //Rc
        char RainAccUnit;
        float RainDuration; //Rd
        char RainDurationUnit;
        float RainIntensity; //Ri
        char RainIntensityUnit;
        float RainPeak; //Rp
        char RainPeakUnit;
        float HailAcc; //Hc
        char HailAccUnit;
        float HailDuration; //Hd
        char HailDurationUnit;
        float HailIntensity; //Hi
        char HailIntensityUnit;
        float HailPeak; //Hp
        char HailPeakUnit;
        float HeatingTemp; //Th
        char HeatingTempUnit;
        float HeatingVolt; //Vh
        char HeatingVoltUnit;
        float ReferenceVolt; //Vr
        char ReferenceVoltUnit;
        string NonDecoded;
        
};