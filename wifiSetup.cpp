#include <Arduino.h>
#include <inttypes.h>
#include <stdio.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h> //https://github.com/me-no-dev/ESPAsyncTCP
#elif defined(ESP32)
#include <WiFi.h>
#include <AsyncTCP.h> //https://github.com/me-no-dev/AsyncTCP
#endif

#include <WiFiClient.h> 
#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include <EEPROM.h>
//#include <AsyncElegantOTA.h> //https://github.com/ayushsharma82/AsyncElegantOTA  //https://randomnerdtutorials.com/esp32-ota-over-the-air-arduino/  //v2.2.6
#include <wifiSetup.h>



WifiSetup::WifiSetup(){
	server.setupClient(client);
	//client.setupServer(&server);
}

WifiSetup::~WifiSetup(){
}

bool WifiSetup::begin(void (*wifiCheck_cb)(bool*, bool*), Stream &serialPort){
	wifiCheckCB = wifiCheck_cb;
	server.begin(serialPort);
	client.begin(wifiCheck_cb, serialPort);
	return startWifiClient();
}

void WifiSetup::setCredentialsServer(char *appSSID, char *appPASS) {
    server.setCredentialsServer(appSSID,appPASS);
}

void WifiSetup::startServer(){
	server.startWifi();
}
void WifiSetup::stopServer(){
	server.stopWifi();
}
bool WifiSetup::startWifiClient(){
	return client.startWifi();
}
void WifiSetup::stopWifi(){
	client.stopWifi();
}

bool WifiSetup::checkWifi(){
	return client.checkWifi();
}

void WifiSetup::loop(){
	if( (checkMode() == wifiSetupServerMode) && server.exitMode() ){
			stopServer();
			startWifiClient();
	}
}

uint8_t WifiSetup::checkMode(){
	return mode;
}

//Public

WifiSetupServer::WifiSetupServer(uint8_t *ptrMode){
	mode = ptrMode;
    setPortServer();
    setCredentialsServer();
    setIp();
    setGateway();
    setMask();
}

WifiSetupServer::~WifiSetupServer(){
    setPortServer();
    setCredentialsServer();
    setIp();
    setGateway();
    setMask();
}

void WifiSetupServer::setupClient(WifiSetupClient &client){
	_client = &client;
}

void WifiSetupServer::setPortServer(uint8_t portServer){
    _port = portServer;
}
void WifiSetupServer::setCredentialsServer(char *appSSID , char *appPASS){
    _appSSID = appSSID;
    _appPASS = appPASS;
}
void WifiSetupServer::setIp(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4){
    _AppStaticIP = IPAddress(b1,b2,b3,b4);
}

void WifiSetupServer::setGateway(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4){
    _AppGateway = IPAddress(b1,b2,b3,b4);
}
void WifiSetupServer::setMask(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4){
    _AppSubnet = IPAddress(b1,b2,b3,b4);
}

void WifiSetupServer::begin(Stream &serialPort){
    //server = AsyncWebServer(_port);
    _myEeprom.begin();
    _debugSerial = &serialPort;
}

bool WifiSetupServer::startWifi(){
  bool statusApp = false;
  WiFi.mode(WIFI_AP_STA);
  WiFi.enableAP(true);
  WiFi.softAPConfig(_AppStaticIP, _AppGateway, _AppSubnet);
  statusApp = WiFi.softAP(_appSSID, _appPASS);
  if(!statusApp)
    return false;

  delay(100);
  statusApp = setupDefaultRoutes();
  //wifiMode = wifiServerMode;  
  *mode = wifiSetupServerMode;
  _debugSerial->println("Server mode up");
  _exit = false;
  return true;
    
}

void WifiSetupServer::stopWifi(){
  _exit = false;
  server.end();
  WiFi.softAPdisconnect(true);
  WiFi.enableAP(false);
  WiFi.disconnect();
  _debugSerial->println("Server mode down");
}

bool WifiSetupServer::setupDefaultRoutes(){
  server.on("/scanWifi", HTTP_GET, [this](AsyncWebServerRequest *request){
	scanWifi(request);
  });
  
  server.on("/scanStatus", HTTP_GET, [this](AsyncWebServerRequest *request){
	  scanStatus(request);
   });
  server.on("/networks", HTTP_GET, [this](AsyncWebServerRequest *request){
	  showNetworks(request);
   });
  server.on("/end", HTTP_GET, [this](AsyncWebServerRequest *request){
	  changeWifiMode(request);
   });
  /*server.on("/updateWifi", HTTP_POST, [this](AsyncWebServerRequest *request){
	  updateWifi(request);
  });*/
  /*server.on("/updateServer", HTTP_POST, [this](AsyncWebServerRequest *request){
	  updateServer(request);
   });*/
  server.onNotFound( [this](AsyncWebServerRequest *request){
	  notFound(request);
  });

  AsyncCallbackJsonWebHandler* handlerUpdateWifi = new AsyncCallbackJsonWebHandler("/updateWifi", [this](AsyncWebServerRequest *request, JsonVariant& json) {
			updateWifi(request, json);
    });
  AsyncCallbackJsonWebHandler* handlerUpdateServer = new AsyncCallbackJsonWebHandler("/updateServer", [this](AsyncWebServerRequest *request, JsonVariant& json) {
			updateServer(request, json);
    });
  //server.onFileUpload(onUpload);
  //server.onRequestBody(onBody);
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, PUT, POST, DELETE, HEAD, OPTIONS");
  server.addHandler(handlerUpdateWifi);
  server.addHandler(handlerUpdateServer);

  //AsyncElegantOTA.begin(&server);
  server.begin();
}

//Privates
void WifiSetupServer::notFound(AsyncWebServerRequest *request)
{
  if (request->method() == HTTP_OPTIONS) {
    request->send(200);
  } else {
    request->send(404, "application/json", "{\"message\":\"Not found\"}");
  }
}

void WifiSetupServer::onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
  //Handle body
}

void WifiSetupServer::onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
  //Handle upload
}

void WifiSetupServer::changeWifiMode(AsyncWebServerRequest *request){
  _exit = true;
  request->send(200);
  /*if(!_client->startWifi()){
	startWifi();
  }*/
}

bool WifiSetupServer::exitMode(){
	return _exit;
}

void WifiSetupServer::scanWifi(AsyncWebServerRequest *request){
  StaticJsonDocument<100> data;
    WiFi.scanNetworks(true,true);
    if (WiFi.scanComplete() >= 0){
      data["status"] = "ended";
    }else{
      data["status"] = "scanning";
    }
    
    String response;
    serializeJson(data, response);
    request->send(200, "application/json", response);
}

void WifiSetupServer::scanStatus(AsyncWebServerRequest *request){
  StaticJsonDocument<100> data;
    if (WiFi.scanComplete() >= 0){
      data["status"] = "ended";
    }else{
      data["status"] = "scanning";
    }
    
    String response;
    serializeJson(data, response);
    request->send(200, "application/json", response);
}

void WifiSetupServer::showNetworks(AsyncWebServerRequest *request){
  DynamicJsonDocument data(4092);
    JsonArray networks = data.createNestedArray("networks");
    StaticJsonDocument<300> network;
    int n = WiFi.scanComplete();
    if(n > 0){
      for (int i = 0; i < n; i++){
        network["ssid"] = WiFi.SSID(i);
        network["channel"] =  WiFi.channel(i);
        network["rssi"] = WiFi.RSSI(i);
        network["encryptionType"] = readableEncryptionType(WiFi.encryptionType(i));
        networks.add(network);
      }     
    }
    String response;
    serializeJson(data, response);
    request->send(200, "application/json", response);
}

/*void WifiSetupServer::updateWifi(AsyncWebServerRequest *request){
  StaticJsonDocument<100> data;
    if ((request->hasParam("ssid", true)) && (request->hasParam("pass", true))){
        _myEeprom.clearSSID();
        _myEeprom.clearPass();
        _myEeprom.updateSSID(request->getParam("ssid",true)->value());
        _myEeprom.updatePass(request->getParam("pass",true)->value());
        EEPROM.commit();
        data["ssid"] = _myEeprom.getSSID();
        data["pass"] = _myEeprom.getPass();
        String response;
        serializeJson(data, response);
        request->send(200, "application/json", response);
    }else{
      request->send(404, "application/json", "{\"error\":\"Invalid request\"}");
    }
}*/

void WifiSetupServer::updateWifi(AsyncWebServerRequest *request, JsonVariant &json){
  JsonObject bodyData = json.as<JsonObject>();
  StaticJsonDocument<100> data;
    if(bodyData.containsKey("ssid") && bodyData.containsKey("pass")){
        _myEeprom.clearSSID();
        _myEeprom.clearPass();
        _myEeprom.updateSSID(bodyData["ssid"].as<String>());
        _myEeprom.updatePass(bodyData["pass"].as<String>());
        EEPROM.commit();
        data["ssid"] = _myEeprom.getSSID();
        data["pass"] = _myEeprom.getPass();
        String response;
        serializeJson(data, response);
        request->send(200, "application/json", response);
    }else{
      request->send(404, "application/json", "{\"error\":\"Invalid request\"}");
    }
}


/*void WifiSetupServer::updateServer(AsyncWebServerRequest *request){
  StaticJsonDocument<100> data;
    if (request->hasParam("server", true)) {
        _myEeprom.clearServer();
        _myEeprom.updateServer(request->getParam("server",true)->value());
        EEPROM.commit();
        data["server"] = String(_myEeprom.getServer(0))+"."+String(_myEeprom.getServer(1))+"."+String(_myEeprom.getServer(2))+"."+String(_myEeprom.getServer(3));
        String response;
        serializeJson(data, response);
        request->send(200, "application/json", response);
    }else{
      request->send(404, "application/json", "{\"error\":\"Invalid request\"}");
    }
}*/

void WifiSetupServer::updateServer(AsyncWebServerRequest *request, JsonVariant &json){
  JsonObject bodyData = json.as<JsonObject>();
  StaticJsonDocument<100> data;
    if (bodyData.containsKey("server")) {
        _myEeprom.clearServer();
        _myEeprom.updateServer(bodyData["server"].as<String>());
        EEPROM.commit();
        data["server"] = String(_myEeprom.getServer(0))+"."+String(_myEeprom.getServer(1))+"."+String(_myEeprom.getServer(2))+"."+String(_myEeprom.getServer(3));
        String response;
        serializeJson(data, response);
        request->send(200, "application/json", response);
    }else{
      request->send(404, "application/json", "{\"error\":\"Invalid request\"}");
    }
}



String WifiSetupServer::readableEncryptionType(uint8_t encType)
{
  String encTypeAsString;
  
  switch(encType)
  {
    case wifiSetup_ENC_TYPE_TKIP:
    {
      encTypeAsString = "WPA";
      break;
    }
    case wifiSetup_ENC_TYPE_WEP:
    {
      encTypeAsString = "WEP";
      break;
    }
    case wifiSetup_ENC_TYPE_CCMP:
    {
      encTypeAsString = "WPA2";
      break;
    }
    case wifiSetup_ENC_TYPE_NONE:
    {
      encTypeAsString = "None";
      break;
    }
    case wifiSetup_ENC_TYPE_AUTO:
    {
      encTypeAsString = "Auto";
      break;
    }
    default:
    {
      encTypeAsString = "Other";
      break;
    }
  }

  return encTypeAsString;
}


WifiSetupEeprom::WifiSetupEeprom(){
}

WifiSetupEeprom::~WifiSetupEeprom(){
}

void WifiSetupEeprom::begin(){
	EEPROM.begin(512);
}


void WifiSetupEeprom::clearSSID(){
  for (int i = 0; i < 32; ++i) { EEPROM.write(i, 0); }
}

void WifiSetupEeprom::clearPass(){
  for (int i = 32; i < 64; ++i) { EEPROM.write(i, 0); }
}

void WifiSetupEeprom::clearServer(){
  for (int i = 64; i < 68; ++i) { EEPROM.write(i, 0); }
}

void WifiSetupEeprom::updateSSID(String ssid){
  for (int i = 0; i < ssid.length(); ++i){
            EEPROM.write(i, ssid[i]);
  }
}

void WifiSetupEeprom::updatePass(String pass){
  for (int i = 0; i < pass.length(); ++i){
            EEPROM.write(i+32, pass[i]);
  }
}

void WifiSetupEeprom::updateServer(String ip){
  int len = 0;
  char buf[20];
  ip.toCharArray(buf,sizeof(buf));
  char *p = buf;
  char *str;
  while ((str = strtok_r(p, ".", &p)) != NULL){ // delimiter is the semicolon
      EEPROM.write(len+64, atoi(str));
      len++;
  }
}

String WifiSetupEeprom::getSSID(){
  char data[32];
  for (int i = 0; i < 32; ++i){
      data[i] = EEPROM.read(i);
  }
  return String(data);
}

bool WifiSetupEeprom::existSSID(){
  uint8_t value = EEPROM.read(0);
  if(value > 0)
    return true;
  return false;
}

String WifiSetupEeprom::getPass(){
  char data[32];
  for (int i = 0; i < 32; ++i){
      data[i] = EEPROM.read(i+32);
  }
  return String(data);
}

byte WifiSetupEeprom::getServer(byte n_byte){
  return EEPROM.read(n_byte+64);
}



WifiSetupClient::WifiSetupClient(uint8_t *ptrMode){
	mode = ptrMode;
}
WifiSetupClient::~WifiSetupClient(){
}

void WifiSetupClient::begin(void (*wifiCheck_cb)(bool*, bool*), Stream &serialPort){
	_debugSerial = &serialPort;
	wifiCheckCB = wifiCheck_cb;
}


/*void WifiSetupClient::setupServer(WifiSetupServer &server){
	_server = &server;
}*/

bool WifiSetupClient::startWifi(){
  *mode = wifiSetupClientMode;
  if(_myEeprom.existSSID()){
	WiFi.mode(WIFI_STA);
	WiFi.reconnect();
	_debugSerial->println("Wifi Client Up");
    WiFi.begin(_myEeprom.getSSID().c_str(), _myEeprom.getPass().c_str());
    *mode = wifiSetupClientMode;
    //wifiMode = wifiClientMode;
    checkWifi();
    //
	 return true;
  }else{
	 *mode = wifiSetupNotMode;
	 return false;
    //_server.startWifi();
  }
  
}


bool WifiSetupClient::checkWifi(){
  if (WiFi.status() != WL_CONNECTED) {
    _debugSerial->print("Connecting to WiFi");
    bool notExit = true;
    bool startServer = false;
    while ((WiFi.status() != WL_CONNECTED ) && notExit && !startServer) {
      delay(1);
      (*wifiCheckCB)(&notExit, &startServer);
      if(!notExit)
		return false;
	  if(startServer)
		return true;
      _debugSerial->print(".");
    }
    _debugSerial->print("\n");
    _debugSerial->println("Connected to the WiFi network");
    _debugSerial->print("IP Address: ");
    _debugSerial->println(WiFi.localIP());
  }
  return false;
}

void WifiSetupClient::stopWifi() {
  WiFi.disconnect();
  _debugSerial->println("Wifi Client down");
}
