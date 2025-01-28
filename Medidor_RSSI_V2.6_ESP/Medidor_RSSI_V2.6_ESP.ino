//Segunda version envia y recibe paquetes
//Envia un paquete, se recibe por nodered a travez de mqtt y se lee el RSSI de subida que es reenviado para su lectura en datoEntrante
//cuando se recibe el paquete se lee el RSSI del paquete recibido con lora.getRssi()
//Medidor RSSI con heltec lora esp32 
//Creado por German Mizdraji para Macro Intell S.A
//2023
//Version 2.6 ESP

#include <lorawan.h>
#include <Wire.h>
#include "config.h"

const unsigned long interval = 15000;    // 5 s interval to send message
int32_t previousMillis = 0;  // will store last time message sent
unsigned int counter = 0;     // message counter
int rcv_count = 0;                // contador de mensajes recibidos
int send_count = 0;               // contador de mensajes enviados
int diferencia = 3;               //diferencia para comparar send_count y rcv_count

int rssi_rcv = 0;
int rssiValue=0;
char rssiSend[20]={0};           //variable para guardar dato a enviar en forma de string

char datoEntrante[254]={0};      //variable para guardar dato recibido
String get_name;                           //variable para guardar gatewayname
char *token;


byte recvStatus = 0;
char uncero[1]={0};

void setup() {
 
  // Setup loraid access
  Serial.begin(SERIAL_SPEED);
  Serial.println("Start..");
  if(!lora.init()){
    Serial.println("RFM95 not detected");
    delay(3000);
    return;
  }

  // Set LoRaWAN Class change CLASS_A or CLASS_C
  lora.setDeviceClass(CLASS_C);

  // Set Data Rate
  lora.setDataRate(SF7BW125);
  

  //Set Channel Default
   lora.setChannel(CH0);
 // lora.setChannel(MULTI);    // set channel to random
  
  // Put ABP Key and DevAddress here
  lora.setNwkSKey(nwkSKey);
  lora.setAppSKey(appSKey);
  lora.setDevAddr(devAddr);
  
  Serial.print("Device name: "); Serial.println(DeviceNM);
  Serial.print("nwkSKey: "); Serial.println(nwkSKey);
  Serial.print("appSKey: "); Serial.println(appSKey);
  Serial.print("devAddr: "); Serial.println(devAddr);

    
    rssiValue = lora.getRssi();
    Serial.println(rssiValue);
    
    recvStatus = lora.readData(datoEntrante);
    previousMillis = millis();
    while(!recvStatus) {    //cada 3 seg envio uncero para obtener una respuesta
      if((millis() - previousMillis) > 3000) {
        previousMillis = millis();
        lora.sendUplink(uncero, strlen(uncero), 0, 1);  
        Serial.println("Sending uncero...");
        rssiValue = lora.getRssi();
        }
    recvStatus = lora.readData(datoEntrante);
    lora.update();
      }
      recvStatus=0;
}

void loop() {
  if(millis() - previousMillis > interval) {     //cada cierto invervalo envio valor de rssi
    previousMillis = millis(); 
    lora.sendUplink(rssiSend, strlen(rssiSend), 1, 1);
    send_count++;
    Serial.print("Tx: ");Serial.println(rssi_rcv);

    if(abs(send_count - rcv_count) > diferencia){
    rcv_count = 0;
    send_count = 0;
    Serial.println("...........................................");
    }
  }

  recvStatus = lora.readData(datoEntrante);
 
 // Obtener el primer token (rssi)
   token = strtok(datoEntrante, ",");
   if (token != NULL) {
   int rssi = atoi(token);
   rssi_rcv = rssi;
  }
 
  // Obtener el segundo token (gatewayname)
  token = strtok(NULL, ",");
  if (token != NULL) {
    String gatewayname(token);
    get_name = gatewayname;
    Serial.print("gateway: ");Serial.println(get_name);
  }
  
  if(recvStatus) {
  rcv_count++;
  rssiValue = lora.getRssi();     //rssi de paquete recibido (downlink)
  itoa(rssiValue, rssiSend, 10);
  
  Serial.print("RSSI de bajada ====>> ");  Serial.println(rssiValue);

   recvStatus = 0;
     }
  
  // Check Lora RX
  lora.update();
}
