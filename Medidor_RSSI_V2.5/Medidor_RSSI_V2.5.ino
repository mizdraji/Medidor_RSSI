//Segunda version envia y recibe paquetes
//Envia un paquete, se recibe por nodered a travez de mqtt y se lee el RSSI de subida que es reenviado para su lectura en datoEntrante
//cuando se recibe el paquete se lee el RSSI del paquete recibido con lora.getRssi()
//Medidor RSSI con heltec lora esp32 
//Creado por German Mizdraji para Macro Intell S.A
//2023
//Version 2.5 heltek
//se agrega para recibir nombre del gateway

#include <lorawan.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

const unsigned long interval = 15000;    // 15 s interval to send message
unsigned long previousMillis = 0;  // will store last time message sent
unsigned int counter = 0;     // message counter

int rssiValue=0;
char rssiSend[20]={0};           //variable para guardar dato a enviar en forma de string

char datoEntrante[255]={0};      //variable para guardar dato recibido
char *token;                 

byte recvStatus = 0;
char uncero[1]={0};

void setup() {
  //setup OLED
  pinMode(OLED_RST,OUTPUT);
  digitalWrite(OLED_RST, LOW);    // set GPIO16 low to reset OLED
  delay(20); 
  digitalWrite(OLED_RST, HIGH);
  
  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR, false, false)) { // Address 0x3C for 128x32
     Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print(VERSION);
  display.display();
  display.setTextSize(1);
  display.setCursor(0,40);
  display.print("Desarrollado por Macro Intell, departamento de Electronica.");
  display.display();
  delay(1000);
  display.clearDisplay();

 
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
  //lora.setChannel(MULTI);    // set channel to random
  
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
    if(millis() - previousMillis > 7000) {
    previousMillis = millis();    
    lora.sendUplink(uncero, strlen(uncero), 0, 1);
    display.clearDisplay();      
    Serial.println("Sending uncero...");
    display.setCursor(0,0);
    display.setTextSize(2);
    display.print("Sending uncero...");
    display.display();
    rssiValue = lora.getRssi();
    
    }
    recvStatus = lora.readData(datoEntrante);
    lora.update();
    
      }
      recvStatus=0;
}

int rssi_rcv = 0;
String get_name;    //variable para guardar gatewayname

void loop() {
  
  // Check interval overflow
  if(millis() - previousMillis > interval) {    //cada cierto invervalo envio valor de rssi
    previousMillis = millis(); 

    
    lora.sendUplink(rssiSend, strlen(rssiSend), 0, 1);
    
    Serial.print("Tx: ");Serial.println(rssi_rcv);
    display.setCursor(0,0);
    display.setTextSize(2);
    display.print("Tx: ");display.print(rssi_rcv);//  display.print(" dbm ");
    //display.setCursor(0,10);
    //display.setTextSize(1);
    //display.print(datoEntrante);//  display.print(" dbm ");

    display.setTextSize(2);
    display.setCursor(0,35);
    display.print("Rx: ");display.print(rssiValue);
    display.setCursor(0,50);
    display.print(get_name);
    //display.setTextSize(1);
    //display.setCursor(0,45);
    //display.print(rssiValue);//display.print(" dbm ");
    display.display();
    display.clearDisplay();
     }

  recvStatus = lora.readData(datoEntrante);
  
  // Obtener el primer token (rssi)
   token = strtok(datoEntrante, ",");
   if (token != NULL) {
   int rssi = atoi(token);
   rssi_rcv = rssi;
   //Serial.print("rssi: ");
   //Serial.println(rssi);
  }
  //Serial.println(rssi_rcv);

  // Obtener el segundo token (gatewayname)
  token = strtok(NULL, ",");
  if (token != NULL) {
    String gatewayname(token);
    get_name = gatewayname;
    Serial.print("gateway: ");Serial.println(get_name);
  }
 
  if(recvStatus) {
  rssiValue = lora.getRssi();     //rssi de paquete recibido (downlink)
  itoa(rssiValue, rssiSend, 10);
  
  Serial.print("RSSI de bajada ====>> ");  Serial.println(rssiValue);
  display.setTextSize(2);
  display.setCursor(0,35);
  display.print("Rx: ");display.print(rssiValue);
  display.setCursor(0,50);
    display.print(get_name);
  //display.setTextSize(1);
  //display.setCursor(0,45);
  //display.print(rssiValue);//display.print(" dbm ");

  
  
  display.setCursor(0,0);
  display.setTextSize(2);
  display.print("Tx: ");display.print(rssi_rcv);
  //display.setCursor(0,10);
  //display.setTextSize(1);
  //display.print(datoEntrante);//display.print(" dbm ");
  display.display();
  display.clearDisplay();
   //recvStatus = 0;
     }
  
  // Check Lora RX
  lora.update();
  }

  
