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
#include "swissknife.h"

using namespace std;

// Globals
string SerialPort;
u_int32_t SerialSpeed;
u_int8_t SerialBits;
u_int8_t SerialStopBits;
char SerialParity;
bool _debugMode = false;
bool OrionMode = false;
string OrionHost;
uint16_t OrionPort;
bool EventDriven = false;
uint32_t PublishInterval;
string NodeID;
bool IoTAgentMode = false;


//functions
bool readSetup();
bool readSetupFromCL(int argc, char *argv[]);
int set_interface_attribs(int fd, int speed);
void dumpVar();

int main(int argc, char *argv[]) {
    char *portname = "/dev/ttyUSB0";
    int fd;
    int wlen;
    if (argc>1) {
        if (!readSetupFromCL(argc, argv)) return -1;
    }

    fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        printf("Error opening %s: %s\n", portname, strerror(errno));
        return -1;
    }
    /*baudrate 115200, 8 bits, no parity, 1 stop bit */
    set_interface_attribs(fd, B19200);
    //set_mincount(fd, 0);                /* set to pure timed read */

    /* simple output */
    do {
        unsigned char buf[100];
        int rdlen;

        rdlen = read(fd, buf, sizeof(buf) - 1);
        if (rdlen > 0) {
            buf[rdlen] = 0;
            printf("Read %d: %s", rdlen, buf);
        } else if (rdlen < 0) {
            printf("Error from read: %d: %s\n", rdlen, strerror(errno));
        } else {  /* rdlen == 0 */
            printf("Timeout from read\n");
        }               
        /* repeat read to get full message */
    } while (1);
}


int set_interface_attribs(int fd, int speed) {
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

void set_mincount(int fd, int mcount)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error tcgetattr: %s\n", strerror(errno));
        return;
    }

    tty.c_cc[VMIN] = mcount ? 1 : 0;
    tty.c_cc[VTIME] = 5;        /* half second timer */

    if (tcsetattr(fd, TCSANOW, &tty) < 0)
        printf("Error tcsetattr: %s\n", strerror(errno));
}




bool readSetup(){
    ifstream File;
    string line;
    File.open ("wsplug.conf");
    if (File.is_open()) {
        string token;
        string value;
        while (! File.eof()){
            getline(File, line);
            if (line[0] != '#') {
                token = trim(line.substr(0, line.find("=")));
                value = trim(line.substr(line.find("=")+1, line.length()-1));
                if (token == "debugMode") {
                    if (value == "1")
                        _debugMode = true;
                    else
                        _debugMode = false; 
                }
                if (token == "NodeID") {
                    NodeID = value;
                }
                if (token == "SerialPort") {
                    SerialPort = value;
                }
                if (token == "SerialSpeed") {
                    SerialSpeed = stoi(value);
                }
                if (token == "SerialBits") {
                    SerialBits = stoi(value);
                }
                if (token == "SerialStopBits") {
                    SerialStopBits = stoi(value);
                }
                if (token == "SerialParity") {
                    SerialStopBits = value.c_str()[0];
                }
                if (token == "OrionMode") {
                    if (value == "1")
                        OrionMode = true;
                    else
                        OrionMode = false; 
                }
                if (token == "OrionHost") {
                    OrionHost = value;
                }
                if (token == "OrionPort") {
                    OrionPort = stoi(value);
                }
                if (token == "EventDriven") {
                    if (value == "1")
                        EventDriven = true;
                    else
                        EventDriven = false; 
                }
                if (token == "PublishInterval") {
                    PublishInterval = stoi(value);
                }
                if (token == "IoTAgentMode") {
                    if (value == "1")
                        IoTAgentMode = true;
                    else
                        IoTAgentMode = false; 
                } 
            }
        }
        File.close();
        if (_debugMode) dumpVar();
        return 1;
    } else {
        cout << "Error reading monifog.conf\n" << endl;
        return 0;
    }
}

bool readSetupFromCL(int argc, char *argv[]){
    int i;
    bool error = false;
    string token;
    string value;
    for (i=1; i<argc; i++) {
        string line(argv[i]);
        if (line[0] != '-' && line[1] != '-') {
            token = trim(line.substr(2, line.find("=")));
            value = trim(line.substr(line.find("=")+1, line.length()-1));
            if (token == "debugMode") {
                if (value == "1")
                    _debugMode = true;
                else
                    _debugMode = false; 
            }
            if (token == "NodeID") {
                NodeID = value;
            }
            if (token == "SerialPort") {
                SerialPort = value;
            }
            if (token == "SerialSpeed") {
                SerialSpeed = stoi(value);
            }
            if (token == "SerialBits") {
                SerialBits = stoi(value);
            }
            if (token == "SerialStopBits") {
                SerialStopBits = stoi(value);
            }
            if (token == "SerialParity") {
                SerialStopBits = value.c_str()[0];
            }
            if (token == "OrionMode") {
                if (value == "1")
                    OrionMode = true;
                else
                    OrionMode = false; 
            }
            if (token == "OrionHost") {
                OrionHost = value;
            }
            if (token == "OrionPort") {
                OrionPort = stoi(value);
            }
            if (token == "EventDriven") {
                if (value == "1")
                    EventDriven = true;
                else
                    EventDriven = false; 
            }
            if (token == "PublishInterval") {
                PublishInterval = stoi(value);
            }
            if (token == "IoTAgentMode") {
                if (value == "1")
                    IoTAgentMode = true;
                else
                    IoTAgentMode = false; 
            } 
        } else {
            error = true;
            break;
        }
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


