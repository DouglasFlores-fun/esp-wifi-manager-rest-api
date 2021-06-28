#include <wifiSetup.h>  //Agregar librería ESP WIFI MANAGER REST API
#define BTN_START_SERVER 5
WifiSetup myWifiManager; //Variable de la librería
 
void setup(){
    Serial.begin(9600); // Iniciar puerto serial
    //pinMode(BTN_START_SERVER, INPUT); // Iniciar el pin BTN_START_SERVER como entrada
    if (!myWifiManager.begin(&cbWifiManager, Serial)){ 
                myWifiManager.startServer(); // iniciar como servidor porque no hay wifi configurado en la EEPROM
        }
}
 
void loop(){
    myWifiManager.loop(); //Verificar si ha recibido una solicitud GET ó POST y dar respuesta.
    if((myWifiManager.checkMode() == wifiSetupClientMode) && myWifiManager.checkWifi()){ //checkWifi congelará el código llamando el código la función cb hasta conectarse al wifi ó se indique que se desea salir del cb, recordar que si startServer es asignado el valor true la respuesta de checkWifi será true
       Serial.println("Starting Server");
       myWifiManager.stopWifi(); //Detener el wifi como cliente
       myWifiManager.startServer(); //Iniciar el wifi como server
  }
}
 
void cbWifiManager(bool *notExit, bool *startServer){
    //if(digitalRead(BTN_START_SERVER)){
    if(shouldStartServer()){
        //Si el botón esta en alto (5v) salir del callback, e indicar que se desea iniciar como modo server para reconfigurar el wifi
        *startServer = true;  //hacer que el checkWifi responda como true.
    }
}

bool shouldStartServer(){
  if(Serial.available()){
      while(Serial.available()){
        Serial.read();
      }
      return true;
  }
  return false;
} 
