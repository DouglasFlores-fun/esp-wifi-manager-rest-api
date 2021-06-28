 
#ifndef WIFISETUP_H_
#define WIFISETUP_H_

#include <Arduino.h>
#include <inttypes.h>
#include <stdio.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
//#include <WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include <EEPROM.h>

#define wifiSetupServerMode 0
#define wifiSetupClientMode 1
#define wifiSetupNotMode 3

class WifiSetupEeprom {
    private:
    
    public:
        WifiSetupEeprom();
        ~WifiSetupEeprom();
        void begin();
        
        void clearSSID();
        void clearPass();
        void clearServer();
        void updateSSID(String ssid);
        void updatePass(String pass);
        void updateServer(String ip);
        String getSSID();
        bool existSSID();
        String getPass();
        byte getServer(byte n_byte);
};


class WifiSetupClient{
    private:
		//WifiSetupServer *_server;
		Stream *_debugSerial;
		WifiSetupEeprom _myEeprom;
		void (*wifiCheckCB)(bool*, bool*);
		uint8_t *mode;
		
    public:
		WifiSetupClient(uint8_t *ptrMode);
		~WifiSetupClient();
		void begin(void (*wifiCheck_cb)(bool*, bool*), Stream &serialPort = Serial);
		//void setupServer(WifiSetupServer &server);
		bool startWifi();
		bool checkWifi();
		void stopWifi();
};

class WifiSetupServer {
    private:
		WifiSetupClient *_client;
        bool setupDefaultRoutes();
        void scanWifi(AsyncWebServerRequest *request);
        void scanStatus(AsyncWebServerRequest *request);
        void showNetworks(AsyncWebServerRequest *request);
        void changeWifiMode(AsyncWebServerRequest *request);
        //void updateWifi(AsyncWebServerRequest *request);
        void updateWifi(AsyncWebServerRequest *request, JsonVariant &json);
        //void updateServer(AsyncWebServerRequest *request);
        void updateServer(AsyncWebServerRequest *request, JsonVariant &json);
        void notFound(AsyncWebServerRequest *request);
        void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
        void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
        
        String readableEncryptionType(uint8_t encType);
        
        uint8_t _port;
        uint8_t *mode;
        char *_appSSID;
        char *_appPASS;
        IPAddress _AppStaticIP; //ESP static ip
        IPAddress _AppGateway;   //IP Address of your WiFi Router (Gateway)
        IPAddress _AppSubnet;  //Subnet mask
        Stream *_debugSerial;
        bool _exit = false;
        
        AsyncWebServer server = AsyncWebServer(80);
        WifiSetupEeprom _myEeprom;
        
        
    
    public:
        WifiSetupServer(uint8_t *ptrMode);
        ~WifiSetupServer();
        
        
        void setPortServer(uint8_t portServer = 80);
        void setCredentialsServer(char *appSSID = "UdbDevice", char *appPASS = "MotaAndScada");
        void setIp(uint8_t b1 = 192, uint8_t b2 = 168, uint8_t b3 = 1, uint8_t b4 = 1);
        void setGateway(uint8_t b1 = 192, uint8_t b2 = 168, uint8_t b3 = 1, uint8_t b4 = 1);
        void setMask(uint8_t b1 = 255, uint8_t b2 = 255, uint8_t b3 = 255, uint8_t b4 = 0);
        
        void begin(Stream &serialPort = Serial);
        void setupClient(WifiSetupClient &client);
        bool startWifi();
        void stopWifi();
        bool exitMode();
              
};


class WifiSetup{
    private:
        bool wifiMode;
        void (*wifiCheckCB)(bool*, bool*);
        uint8_t mode = 0; 
        WifiSetupServer server = WifiSetupServer(&mode);
        WifiSetupClient client = WifiSetupClient(&mode);
    public:
		WifiSetup();
		~WifiSetup();
		bool begin(void (*wifiCheck_cb)(bool*, bool*), Stream &serialPort = Serial);
		void startServer();
		void stopServer();
		bool startWifiClient();
		bool checkWifi();
		void stopWifi();
		void loop();
		uint8_t checkMode();
};

#endif
