/* WSplug - Weather Station Ultra-ligth connector
*
* Author: Alexandre Heideker - UFABC
* 2019-04-25
* In context of SWAMP Project
*/
#include <unistd.h>
#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <thread>
#include <curl/curl.h>
#include <mutex>

#define SEM_WAIT mt.lock();
#define SEM_POST mt.unlock();

#include "swissknife.h"
#include "weatherstation.h"

using namespace std;


// Globals
std::mutex mt;
string SerialPort;
string SerialSpeed;
unsigned int SerialBits;
unsigned int SerialStopBits;
char SerialParity;
bool _debugMode = false;
bool OrionMode = false;
string OrionHost;
uint16_t OrionPort;
bool EventDriven = false;
uint32_t PublishInterval;
string NodeID;
bool IoTAgentMode = false;
WeatherStation WS;



//functions
bool readSetup();
bool readSetupFromCL(int argc, char *argv[]);
bool parseVar(string token, string value);
void dumpVar();
bool setInterfaceAttributes(int fd);
void updateData(string line);
void thrOrionPublisher();
bool ckEntity();
bool createEntity();
bool updateEntity();
string getRestFiware(string url, curl_slist *chunk, string data);
static size_t curlCallback(char *contents, size_t size, size_t nmemb, void *userp);
const string getJSON();

int main(int argc, char *argv[]) {
    int fd;
    if (!readSetup()) return -1;
    if (argc>1) {
        if (!readSetupFromCL(argc, argv)) return -1;
    }
    if (_debugMode) dumpVar();

    fd = open(SerialPort.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        cout << "Error opening " << SerialPort << "(" << strerror(errno) << endl;
        return -1;
    }
    if (_debugMode) cout << "Serial Port Opened." << endl;

    if (!setInterfaceAttributes(fd)) {
        close(fd);
        return -1;
    }
    if (_debugMode) cout << "Serial Port Configurated." << endl;

    thread tOrion (thrOrionPublisher);

    do {
        if (_debugMode) cout << "Getting Serial Data..." << endl;
        char buf[100];
        int rdlen;
        rdlen = read(fd, buf, sizeof(buf) - 1);
        if (rdlen > 0) {
            buf[rdlen] = 0;
            string line(buf);
            if (_debugMode) cout << line << endl;
            updateData(line);
        } else if (rdlen < 0) {
            cout << "Error reading port (" << strerror(errno) << ")" << endl;
        }
    } while (1);
    tOrion.join();

}




void updateData(string line){
    // wind: Dn Dm Dx Sn Sm Sx
    // PTU: Ta Tp Ua Pa
    // Rain: Rc Rd Ri Hc Hd Hi Rp Hp
    // Supv: Th Vh Vs Vr Id
    // Units: 

    std::vector <std::string> tokens;
    line = line.substr(0, line.length()-2);
    tokens = splitString(line, ',');
    string token, value, unit;
    //Decode token loop
    SEM_WAIT
    if (_debugMode) cout << "|" << line << "|" << endl;
    for (auto tk: tokens) {
	unsigned int n = tk.find("=");
	if (n!=string::npos) {
        token = trim(tk.substr(0, n));
        if (token=="Id") {
            value = trim(tk.substr(n+1, tk.length()-n-1));
            unit = " ";
        } else {
            value = trim(tk.substr(n+1, tk.length()-n-2));
            unit = tk.substr(tk.length()-1,1);
        }
	if (_debugMode) cout << "| Token:" << token << "\tValue:" << value << "\tUnit:" << unit << " |" << endl;
        if (token=="Id") {
            WS.Id = value;
        } else
        if (token=="Vs") {
            WS.Battery = stof(value);
            WS.BatteryUnit = unit;
        } else
        if (token=="Sn") {
            WS.WindSpeedMin = stof(value);
            WS.WindSpeedMinUnit = unit;
        } else
        if (token=="Sm") {
            WS.WindSpeedAvg = stof(value);
            WS.WindSpeedAvgUnit = unit;
        } else
        if (token=="Sx") {
            WS.WindSpeedMax = stof(value);
            WS.WindSpeedMaxUnit = unit;
        } else
        if (token=="Dn") {
            WS.WindDirMin = stof(value);
            WS.WindDirMinUnit = unit;
        } else
        if (token=="Dm") {
            WS.WindDirAvg = stof(value);
            WS.WindDirAvgUnit = unit;
        } else
        if (token=="Dx") {
            WS.WindDirMax = stof(value);
            WS.WindDirMaxUnit = unit;
        } else
        if (token=="Pa") {
            WS.AirPressure = stof(value);
            WS.AirPressureUnit = unit;
        } else
        if (token=="Ta") {
            WS.AirTemperature = stof(value);
            WS.AirTemperatureUnit = unit;
        } else
        if (token=="Tp") {
            WS.InternalTemperature = stof(value);
            WS.InternalTemperatureUnit = unit;
        } else
        if (token=="Ua") {
            WS.RelativeHumidity = stof(value);
            WS.RelativeHumidityUnit = unit;
        } else
        if (token=="Rc") {
            WS.RainAcc = stof(value);
            WS.RainAccUnit = unit;
        } else
        if (token=="Rd") {
            WS.RainDuration = stof(value);
            WS.RainDurationUnit = unit;
        } else
        if (token=="Ri") {
            WS.RainIntensity = stof(value);
            WS.RainIntensityUnit = unit;
        } else
        if (token=="Rp") {
            WS.RainPeak = stof(value);
            WS.RainPeakUnit = unit;
        } else
        if (token=="Hc") {
            WS.HailAcc = stof(value);
            WS.HailAccUnit = unit;
        } else
        if (token=="Hd") {
            WS.HailDuration = stof(value);
            WS.HailDurationUnit = unit;
        } else
        if (token=="Hi") {
            WS.HailIntensity = stof(value);
            WS.HailIntensityUnit = unit;
        } else
        if (token=="Hp") {
            WS.HailPeak = stof(value);
            WS.HailPeakUnit = unit;
        } else
        if (token=="Th") {
            WS.HeatingTemp = stof(value);
            WS.HeatingTempUnit = unit;
        } else
        if (token=="Vh") {
            WS.HeatingVolt = stof(value);
            WS.HeatingVoltUnit = unit;
        } else
        if (token=="Vr") {
            WS.ReferenceVolt = stof(value);
            WS.ReferenceVoltUnit = unit;
        } else
            WS.NonDecoded = tk;
      }
    } 
    SEM_POST
}




bool setInterfaceAttributes(int fd) {
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        cout << "Error from tcgetattr: " << strerror(errno) << endl;
        return false;
    }
    speed_t SerSp;
    SerSp = (unsigned long) SerialSpeed.c_str();


    cfsetospeed(&tty, SerSp);
    cfsetispeed(&tty, SerSp);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    switch (SerialBits) {
        case 5:
            tty.c_cflag |= CS5;
            break;
        case 6:
            tty.c_cflag |= CS6;
            break;
        case 7:
            tty.c_cflag |= CS7;
            break;
        default:
            tty.c_cflag |= CS8;
            break;    
    };
    
    if (SerialParity=='N') {
        tty.c_cflag &= ~PARENB;     /* no parity bit */
    } else {
        if (SerialParity=='O') {
            tty.c_cflag &= PARENB;     /* parity odd */
        }
    }
    if (SerialStopBits==1) {
        tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    } else {
        tty.c_cflag &= CSTOPB;     /* 2 stop bit */
    }

    //default configurations 
    tty.c_cflag &= ~CRTSCTS; 
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        cout << "Error from tcsetattr: " << strerror(errno) << endl;
        return false;
    }
    return true;
}

bool parseVar(string token, string value){
    if (token == "debugMode") {
        if (value == "1")
            _debugMode = true;
        else
            _debugMode = false; 
    } else
    if (token == "NodeID") {
        NodeID = value;
    } else
    if (token == "SerialPort") {
        SerialPort = value;
    } else
    if (token == "SerialSpeed") {
        SerialSpeed = value;
    } else
    if (token == "SerialBits") {
        SerialBits = stoi(value);
    } else
    if (token == "SerialStopBits") {
        SerialStopBits = stoi(value);
    } else
    if (token == "SerialParity") {
        SerialParity = value.c_str()[0];
    } else
    if (token == "OrionMode") {
        if (value == "1")
            OrionMode = true;
        else
            OrionMode = false; 
    } else
    if (token == "OrionHost") {
        OrionHost = value;
    } else
    if (token == "OrionPort") {
        OrionPort = stoi(value);
    } else
    if (token == "EventDriven") {
        if (value == "1")
            EventDriven = true;
        else
            EventDriven = false; 
    } else
    if (token == "PublishInterval") {
        PublishInterval = stoi(value);
    } else
    if (token == "IoTAgentMode") {
        if (value == "1")
            IoTAgentMode = true;
        else
            IoTAgentMode = false; 
    } else
    {
        cout << "Invalid argument: Token=" << token << "  Value=" << value << endl;
        return false;
    }
    return true;
}

bool readSetup(){
    bool error = false;
    ifstream File;
    string line;
    File.open ("wsplug.conf");
    if (File.is_open()) {
        string token;
        string value;
        while (!File.eof() && !error){
            getline(File, line);
            if (line[0] != '#' && line!="") {
                token = trim(line.substr(0, line.find("=")));
                value = trim(line.substr(line.find("=")+1, line.length()-1));
                error = !parseVar(token, value);
            }
        }
        File.close();
        return !error;
    } else {
        cout << "Error reading wsplug.conf\n" << endl;
        return false;
    }
}

bool readSetupFromCL(int argc, char *argv[]){
    int i;
    bool error = false;
    string token;
    string value;
    for (i=1; i<argc; i++) {
        string line(argv[i]);
        if (line[0] == '-' && line[1] == '-') {
            token = trim(line.substr(2, line.find("=")-2));
            value = trim(line.substr(line.find("=")+1, line.length()-1));
            error = !parseVar(token, value);
        } else {
            error = true;
        }
        if (error) break;
    }
    if (error) {
        cout << "Error: Invalid command line argument" << endl;
        cout << "Usage: FILL USAGE VARS " << endl;
        return false;
    } else {
        return true;
    }
}

void dumpVar() {
    cout << "*** DEBUG Mode *** Dumping variables:" << endl;
    cout << "SerialPort:\t" << SerialPort << endl;
    cout << "SerialSpeed:\t" <<  SerialSpeed << endl;
    cout << "SerialBits:\t" <<  SerialBits << endl;
    cout << "SerialStopBits:\t" <<  SerialStopBits << endl;
    cout << "SerialParity:\t" <<  SerialParity << endl;
    cout << "OrionMode:\t" <<  OrionMode << endl;
    cout << "OrionHost:\t" <<  OrionHost << endl;
    cout << "OrionPort:\t" <<  OrionPort << endl;
    cout << "EventDriven:\t" <<  EventDriven << endl;
    cout << "PublishInterval:\t" <<  PublishInterval << endl;
    cout << "NodeID:\t" <<  NodeID << endl;
    cout << "IoTAgentMode:\t" <<  IoTAgentMode << endl;
}


void thrOrionPublisher(){
    if (!OrionMode) return;
	sleep(30);
    if (_debugMode) cout << "Start Orion Publisher Thread." << endl;
    bool connected = false;
    while (true) {
        if (!connected) {
            while (!ckEntity()) {
                if (_debugMode) cout << "Can't find entity..." << endl;
                if (!createEntity()) {
                    if (_debugMode) cout << "Waiting for three seconds and I'll try to connect again..." << endl;
                    sleep(3); //if can't create entity, wait 3 seconds and try again...
                } else {
                    if (_debugMode) cout << "Entity created..." << endl;
                    break;
                }
            }
            connected = true;
        }
        while (connected) {
            std::time_t tsIni = std::time(0);
            if (_debugMode) cout << "Updating Orion Context-broker..." << endl;
            if (!updateEntity()) {
                connected = false;
                if (_debugMode) cout << "Lost connection with Orion..." << endl;
                break;
            }
            unsigned int taskTime = (int) (std::time(0) - tsIni);
            int sleepTime = PublishInterval - taskTime;
            if (taskTime <= PublishInterval) 
                sleep(sleepTime);
        }
    }
}

bool ckEntity(){
    ostringstream url;

    url << OrionHost << ":" << OrionPort << "/v2/entities?type=WeatherStation&id=" << NodeID;
    if (_debugMode) cout << "URL:\t" << url.str() << endl;
    struct curl_slist *chunk = NULL;
//    chunk = curl_slist_append(chunk, "Content-Type: application/json");
    chunk = curl_slist_append(chunk, "fiware-service: openiot");
    chunk = curl_slist_append(chunk, "fiware-servicepath: /");

    string retStr = getRestFiware(url.str(), chunk, "");
    if (_debugMode) cout << "CURL return: " << retStr << endl;
    if (retStr!="error") {
        if (retStr != "[]") {
            if (_debugMode) cout << "Found!" << endl;
            return true;
        }
    }
    return false;  
}

bool updateEntity(){
    SEM_WAIT
    string js = "{\"Battery\": {\"value\":\"" + to_string(WS.Battery) + "\" , \"type\": \"Number\"}, \"BatteryUnit\": {\"value\":\"";
	    js += (WS.BatteryUnit + "\" , \"type\":\"Text\"}, ");
            js += "\"WindSpeedMin\": {\"value\":\"" + to_string(WS.WindSpeedMin) + "\" , \"type\": \"Number\"}, \"WindSpeedMinUnit\": {\"value\":\"";
            js += (WS.WindSpeedMinUnit + "\", \"type\":\"Text\"}, ");
            js += "\"WindSpeedAvg\": {\"value\":\"" + to_string(WS.WindSpeedAvg) + "\" , \"type\": \"Number\"}, \"WindSpeedAvgUnit\": {\"value\":\"";
            js += (WS.WindSpeedAvgUnit + "\", \"type\":\"Text\"}, ");
            js += "\"WindSpeedMax\": {\"value\":\"" + to_string(WS.WindSpeedMax) + "\" , \"type\": \"Number\"}, \"WindSpeedMaxUnit\": {\"value\":\"";
            js += (WS.WindSpeedMaxUnit + "\", \"type\":\"Text\"}, ");
            js += "\"WindDirMin\": {\"value\":\"" + to_string(WS.WindDirMin) + "\" , \"type\": \"Number\"}, \"WindDirMinUnit\": {\"value\":\"";
            js += (WS.WindDirMinUnit + "\", \"type\":\"Text\"}, ");
            js +=  "\"WindDirAvg\": {\"value\":\"" + to_string( WS.WindDirAvg) + "\" , \"type\": \"Number\"}, \"WindDirAvgUnit\": {\"value\":\"";
            js += (WS.WindDirAvgUnit + "\", \"type\":\"Text\"},");
            js +=  "\"WindDirMax\": {\"value\":\"" + to_string( WS.WindDirMax) + "\" , \"type\": \"Number\"}, \"WindDirMaxUnit\": {\"value\":\"";
            js +=  (WS.WindDirMaxUnit + "\", \"type\":\"Text\"},");
            js +=  "\"AirPressure\": {\"value\":\"" + to_string( WS.AirPressure) + "\" , \"type\": \"Number\"}, \"AirPressureUnit\": {\"value\":\"";
            js +=  (WS.AirPressureUnit + "\", \"type\":\"Text\"},");
            js +=  "\"AirTemperature\": {\"value\":\"" + to_string( WS.AirTemperature) + "\" , \"type\": \"Number\"}, \"AirTemperatureUnit\": {\"value\":\"";
            js +=  (WS.AirTemperatureUnit + "\", \"type\":\"Text\"},");
            js +=  "\"InternalTemperature\": {\"value\":" + to_string( WS.InternalTemperature) + " , \"type\": \"Number\"}, \"InternalTemperatureUnit\": {\"value\":\"";
            js +=  (WS.InternalTemperatureUnit + "\", \"type\":\"Text\"},");
            js +=  "\"RelativeHumidity\": {\"value\":\"" + to_string( WS.RelativeHumidity) + "\" , \"type\": \"Number\"}, \"RelativeHumidityUnit\": {\"value\":\"";
            js +=  (WS.RelativeHumidityUnit + "\", \"type\":\"Text\"},");
            js +=  "\"RainAcc\": {\"value\":\"" + to_string( WS.RainAcc) + "\" , \"type\": \"Number\"}, \"RainAccUnit\": {\"value\":\"";
            js +=  (WS.RainAccUnit + "\", \"type\":\"Text\"},");
            js +=  "\"RainDuration\": {\"value\":\"" + to_string( WS.RainDuration) + "\" , \"type\": \"Number\"}, \"RainDurationUnit\": {\"value\":\"";
            js +=  (WS.RainDurationUnit + "\", \"type\":\"Text\"},");
            js +=  "\"RainIntensity\": {\"value\":\"" + to_string( WS.RainIntensity) + "\" , \"type\": \"Number\"}, \"RainIntensityUnit\": {\"value\":\"";
            js +=  (WS.RainIntensityUnit + "\", \"type\":\"Text\"},");
            js +=  "\"RainPeak\": {\"value\":\"" + to_string( WS.RainPeak) + "\" , \"type\": \"Number\"}, \"RainPeakUnit\": {\"value\":\"";
            js +=  (WS.RainPeakUnit + "\", \"type\":\"Text\"},");
            js +=  "\"HailAcc\": {\"value\":\"" + to_string( WS.HailAcc) + "\" , \"type\": \"Number\"}, \"HailAccUnit\": {\"value\":\"";
            js +=  (WS.HailAccUnit + "\", \"type\":\"Text\"},");
            js +=  "\"HailDuration\": {\"value\":\"" + to_string( WS.HailDuration) + "\" , \"type\": \"Number\"}, \"HailDurationUnit\": {\"value\":\"";
            js +=  (WS.HailDurationUnit + "\", \"type\":\"Text\"},");
            js +=  "\"HailIntensity\": {\"value\":\"" + to_string( WS.HailIntensity) + "\" , \"type\": \"Number\"}, \"HailIntensityUnit\": {\"value\":\"";
            js +=  (WS.HailIntensityUnit + "\", \"type\":\"Text\"},");
            js +=  "\"HailPeak\": {\"value\":\"" + to_string( WS.HailPeak) + "\" , \"type\": \"Number\"}, \"HailPeakUnit\": {\"value\":\"";
            js +=  (WS.HailPeakUnit + "\", \"type\":\"Text\"},");
            js +=  "\"HeatingTemp\": {\"value\":\"" + to_string( WS.HeatingTemp) + "\" , \"type\": \"Number\"}, \"HeatingTempUnit\": {\"value\":\"";
            js +=  (WS.HeatingTempUnit + "\", \"type\":\"Text\"},");
            js +=  "\"HeatingVolt\": {\"value\":\"" + to_string( WS.HeatingVolt) + "\" , \"type\": \"Number\"}, \"HeatingVoltUnit\": {\"value\":\"";
            js +=  (WS.HeatingVoltUnit + "\", \"type\":\"Text\"},");
            js +=  "\"ReferenceVolt\": {\"value\":\"" + to_string( WS.ReferenceVolt) + "\" , \"type\": \"Number\"}, \"ReferenceVoltUnit\": {\"value\":\"";
            js +=  (WS.ReferenceVoltUnit + "\", \"type\":\"Text\"},");
	    js += "\"LocalTimeStamp\": {\"value\":\""+to_string(std::time(0))+"\", \"type\":\"Integer\"}}";        
    SEM_POST
    ostringstream url;

//    cout << "|" << js << "|" << endl;

//    cout << js.length() << endl;

//    cout << "|" << js.c_str() << "|" << endl;

//    cout << sizeof(js.c_str()) << endl;

    
    url << OrionHost << ":" << OrionPort << "/v2/entities/" << NodeID << "/attrs?options=keyValues";
    if (_debugMode) cout << "URL:\t" << url.str() << endl;
    if (_debugMode) cout << "JSON:\t" << js << endl;
    struct curl_slist *chunk = NULL;
    chunk = curl_slist_append(chunk, "Content-Type: application/json");
    chunk = curl_slist_append(chunk, "fiware-service: openiot");
    chunk = curl_slist_append(chunk, "fiware-servicepath: /");

    string retStr = getRestFiware(url.str(), chunk, js);
    if (retStr=="error") {
        if (_debugMode) cout << "ERROR: " << retStr << endl;
        return false;
    }
    return true;
}

bool createEntity(){
    ostringstream json;
    SEM_WAIT
    json << "{\"id\":\"" << NodeID << "\", \"type\":\"WeatherStation\" }";
    SEM_POST

    ostringstream url;
    url << OrionHost << ":" << OrionPort << "/v2/entities?options=keyValues";
    if (_debugMode) cout << "URL:\t" << url.str() << endl;
    if (_debugMode) cout << "JSON:\t" << json.str() << endl;
    struct curl_slist *chunk = NULL;
    chunk = curl_slist_append(chunk, "Content-Type: application/json");
    chunk = curl_slist_append(chunk, "fiware-service: openiot");
    chunk = curl_slist_append(chunk, "fiware-servicepath: /");

    string retStr = getRestFiware(url.str(), chunk, json.str());
    if (retStr.find("{\"error\":",0) || retStr.find("400",0) ) 
        return false;
    return true;
}

string getRestFiware(string url, curl_slist *chunk, string data) {
    CURL *curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        if (_debugMode) curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        if (data != "") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, -1L);
        }
        string Buffer;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &Buffer);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            if (_debugMode) cout << "CURL return: " << res << endl;
//            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return "error";
        }
//        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return Buffer;
    }
//    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return "error";
}


static size_t curlCallback(char *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
