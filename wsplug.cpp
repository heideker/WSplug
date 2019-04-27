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

using namespace std;


string SerialPort;
u_int32_t SerialSpeed;
u_int8_t SerialBits;
u_int8_t SerialStopBits;
char SerialParity;

bool readSetup();

int main(int argc, char *argv[]) {
    char *portname = "/dev/ttyUSB0";
    int fd;
    int wlen;

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


int set_interface_attribs(int fd, int speed)
{
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
                        IMvar.debugMode = true;
                    else
                        IMvar.debugMode = false; 
                }
                if (token == "CPUSingleStat") {
                    if (value == "1")
                        IMvar.CPUSingleStat = true;
                    else
                        IMvar.CPUSingleStat = false; 
                }
                if (token == "LogMode") {
                    if (value == "1")
                        IMvar.LogMode = true;
                    else
                        IMvar.LogMode = false; 
                }
                if (token == "LogType") {
                    IMvar.LogType = UCase(value);
                }
                if (token == "LogFileName") {
                    IMvar.LogFileName = value;
                }
                if (token == "NodeName") {
                    IMvar.NodeName = value;
                }
                if (token == "NodeUUID") {
                    IMvar.NodeUUID = value;
                }
                if (token == "KindOfNode") {
                    IMvar.KindOfNode = value;
                }
               if (token == "MySQLMode") {
                    if (value == "1")
                        IMvar.MySQLMode = true;
                    else
                        IMvar.MySQLMode = false; 
                }
                if (token == "MySQLHost") {
                    IMvar.MySQLHost = value;
                }
                if (token == "MySQLUser") {
                    IMvar.MySQLUser = value;
                }
                if (token == "MySQLPasswd") {
                    IMvar.MySQLPasswd = value;
                }
                if (token == "ServerMode") {
                    if (value == "1")
                        IMvar.ServerMode = true;
                    else
                        IMvar.ServerMode = false; 
                }
                if (token == "ServerPort") {
                    IMvar.ServerPort = stoi(value);
                }
                if (token == "OrionMode") {
                    if (value == "1")
                        IMvar.OrionMode = true;
                    else
                        IMvar.OrionMode = false; 
                }
                if (token == "OrionHost") {
                    IMvar.OrionHost = value;
                }
                if (token == "OrionPort") {
                    IMvar.OrionPort = stoi(value);
                }
                if (token == "SampplingTime") {
                    IMvar.SampplingTime = stoi(value);
                }
                if (token == "LogIntervall") {
                    IMvar.LogIntervall = stoi(value);
                }
                if (token == "OrionPublisherTime") {
                    IMvar.OrionPublisherTime = stoi(value);
                }
                if (token == "DockerStat") {
                    if (value == "1")
                        IMvar.DockerStat = true;
                    else
                        IMvar.DockerStat = false; 
                }
                if (token == "CPUStat") {
                    if (value == "1")
                        IMvar.CPUStat = true;
                    else
                        IMvar.CPUStat = false; 
                }
                if (token == "CPUPathStat") {
                    IMvar.CPUPathStat = value;
                }
                if (token == "CPUPathArch") {
                    IMvar.CPUPathArch = value;
                }       
                if (token == "NetworkStat") {
                    if (value == "1")
                        IMvar.NetworkStat = true;
                    else
                        IMvar.NetworkStat = false; 
                }
                if (token == "NetworkPathStat") {
                    IMvar.NetworkPathStat = value;
                }
                if (token == "ProcessNames") {
                    IMvar.ProcessNames = splitString(trim(value), ' ');
                }
                if (token == "DockerNames") {
                    IMvar.DockerNames = splitString(trim(value), ' ');
                }
 
                if (token == "DiskStat") {
                    if (value == "1")
                        IMvar.DiskStat = true;
                    else
                        IMvar.DiskStat = false; 
                }
            }
        }
        File.close();
        if (IMvar.debugMode) dumpVar(IMvar);
        return 1;
    } else {
        cout << "Error reading monifog.conf\n" << endl;
        return 0;
    }
}

