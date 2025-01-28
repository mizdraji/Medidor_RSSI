//Segunda version envia y recibe paquetes
//Envia un paquete, se recibe por nodered a travez de mqtt y se lee el RSSI de subida que es reenviado para su lectura en datoEntrante
//cuando se recibe el paquete se lee el RSSI del paquete recibido con lora.getRssi()
//Medidor RSSI con heltec lora esp32 
//Creado por German Mizdraji para Macro Intell S.A
//2023
//Version 2.3 ESP

#include <lorawan.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>

#define VERSION "Medidor RSSI             V2.3 ESP"       //Version

#define SERIAL_SPEED 115200         //Serial

//ABP Credentials 
const char *devAddr = "0106876e";       //"8c0dacb5": 25617; "01fa9919": nodo Medidor rssi; "ef491cef": 24337 nodo con bateria verde
const char *nwkSKey = "07e7e68ce894f662d84ed54d0c0c23fa";
const char *appSKey = "db62bbff43b4bf4f0f73e707076a14b5";
const char *DeviceNM = "Medidor RSSI";

const unsigned long interval = 5000;    // 5 s interval to send message
unsigned long previousMillis = 0;  // will store last time message sent
unsigned int counter = 0;     // message counter

int rssiValue=0;
char rssiSend[20]={0};           //variable para guardar dato a enviar en forma de string
char PrevSend[20]={0};            //variable para guardar el valor previo a enviar
char datoEntrante[20]={0};      //variable para guardar dato recibido


byte recvStatus = 0;
char uncero[1]={0};

#define tipo_modulo 0     //Es para definir el pinout de SPI con el modulo Lora. 0: ESP12, 1: ESP32

#if tipo_modulo == 0
//RFM95W + ESP12 (Macro version)
const sRFM_pins RFM_pins = {
  .CS = 15,
  .RST = 0,
  .DIO0 = 5,
  .DIO1 = 5,
  .DIO2 = 5,
  .DIO5 = -1,
};
#endif

#if tipo_modulo == 1
//SX1276 + ESP32 (TTGO Lora32)
//heltec wifi lora esp32 V2
const sRFM_pins RFM_pins = {
  .CS = 18,
  .RST = 14,
  .DIO0 = 26,
  .DIO1 = 33,
  .DIO2 = 32,
  .DIO5 = -1,
};
#endif


void setup() {
 
  // Setup loraid access
  Serial.begin(SERIAL_SPEED);
  delay(100);
  Serial.println("Start..");
  if(!lora.init()){
    Serial.println("RFM95 not detected");
    delay(5000);
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
    while(!recvStatus) {    //cada 3 seg envio uncero para obtener una respuesta
    Serial.print("recvStatus: "); Serial.println(recvStatus);
    recvStatus = lora.readData(datoEntrante);
      
    lora.sendUplink(uncero, strlen(uncero), 0, 1);      
    Serial.println("Sending uncero...");
    rssiValue = lora.getRssi();
    lora.update();
    delay(interval);
      }
      recvStatus=0;
}

void loop() {
  
  // Check interval overflow
  if(millis() - previousMillis > interval) {    //cada cierto invervalo envio uncero
    previousMillis = millis(); 
    
    lora.sendUplink(rssiSend, strlen(rssiSend), 0, 1);

    Serial.print("Tx: ");Serial.println(datoEntrante);
 
     }

  recvStatus = lora.readData(datoEntrante);
 
  if(recvStatus) {
  rssiValue = lora.getRssi();     //rssi de paquete recibido (downlink)
  itoa(rssiValue, rssiSend, 10);
  
  Serial.print("RSSI de bajada ====>> ");  Serial.println(rssiValue);

   recvStatus = 0;
     }
  
  // Check Lora RX
  lora.update();
}
