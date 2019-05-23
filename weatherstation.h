#include <string>

using namespace std;

class WeatherStation {
    public:
        string Id;
        float Battery;
        string BatteryUnit;
        float WindSpeedMin; //Sn
        string WindSpeedMinUnit;
        float WindSpeedAvg; //Sm
        string WindSpeedAvgUnit;
        float WindSpeedMax; //Sx
        string WindSpeedMaxUnit;
        float WindDirMin; //Dn
        string WindDirMinUnit;
        float WindDirAvg; //Dm
        string WindDirAvgUnit;
        float WindDirMax; //Dx
        string WindDirMaxUnit;
        float AirPressure; //Pa
        string AirPressureUnit;
        float AirTemperature; //Ta
        string AirTemperatureUnit;
        float InternalTemperature; //Tp
        string InternalTemperatureUnit;
        float RelativeHumidity; //Ua
        string RelativeHumidityUnit;
        float RainAcc; //Rc
        string RainAccUnit;
        float RainDuration; //Rd
        string RainDurationUnit;
        float RainIntensity; //Ri
        string RainIntensityUnit;
        float RainPeak; //Rp
        string RainPeakUnit;
        float HailAcc; //Hc
        string HailAccUnit;
        float HailDuration; //Hd
        string HailDurationUnit;
        float HailIntensity; //Hi
        string HailIntensityUnit;
        float HailPeak; //Hp
        string HailPeakUnit;
        float HeatingTemp; //Th
        string HeatingTempUnit;
        float HeatingVolt; //Vh
        string HeatingVoltUnit;
        float ReferenceVolt; //Vr
        string ReferenceVoltUnit;
        string NonDecoded;
        
};
