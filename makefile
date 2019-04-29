# for raspbian, include libcurl4-openssl-dev package
all: 
	gcc -Wall -o wsplug wsplug.cpp -lstdc++ -lcurl -lpthread -std=c++11

