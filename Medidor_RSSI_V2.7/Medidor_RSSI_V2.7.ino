//Segunda version envia y recibe paquetes
//Envia un paquete, se recibe por nodered a travez de mqtt y se lee el RSSI de subida que es reenviado para su lectura en datoEntrante
//cuando se recibe el paquete se lee el RSSI del paquete recibido con lora.getRssi()
//Medidor RSSI con heltec lora esp32 
//Creado por German Mizdraji para Macro Intell S.A
//2023
//Version 2.6 heltek
//cada 3 mensajes enviados si no se recibe respuestas aparecen puntos para saber que no hay un gateway disponible

#include <lorawan.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "Vars.h"

void setup() {
  Serial.begin(SERIAL_SPEED);
  //setup OLED
  pinMode(OLED_RST,OUTPUT);
  digitalWrite(OLED_RST, LOW);              // set GPIO16 low to reset OLED
  delay(20); 
  digitalWrite(OLED_RST, HIGH);
  
  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR, false, false)) {       // Address 0x3C for 128x32
     Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.display();
  delay(2000); // Pause for 2 seconds
  
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

 
  //Setup LoRa
  Serial.println("Start..");
  if(!lora.init()){
    Serial.println("RFM95 not detected");
    delay(5000);
    return;
  }
  lora.setDeviceClass(CLASS_C);     // Set LoRaWAN Class change CLASS_A or CLASS_C
  lora.setDataRate(SF7BW125);       // Set Data Rate
  lora.setChannel(CH0);            //Set Channel Default
  lora.setNwkSKey(nwkSKey);
  lora.setAppSKey(appSKey);
  lora.setDevAddr(devAddr);
  Serial.print("Device name: "); Serial.println(DeviceNM);
  Serial.print("nwkSKey: "); Serial.println(nwkSKey);
  Serial.print("appSKey: "); Serial.println(appSKey);
  Serial.print("devAddr: "); Serial.println(devAddr);
  //pinMode(RFM_pins.DIO0, INPUT_PULLUP);

  //attachInterrupt(digitalPinToInterrupt(RFM_pins.DIO0), onReceive,  CHANGE);            //habilita interrupciones para mensajes recibidos lora, se utiliza CHANGE para cuando la señal cambia HIGH <-->LOW. Con RISING se generan multiples interrupciones.
    
  while(!recvStatus) {                                   //cada 3 seg envio uncero para obtener una respuesta
    if(millis() - previousMillis > 10000) {
      previousMillis = millis();
      char uncero[1]={0};    
      lora.sendUplink(uncero, strlen(uncero), 0, 1);
      display.clearDisplay();      
      Serial.println("Sending uncero...");
      display.setCursor(0,0);
      display.setTextSize(2);
      display.print("Sending uncero...");
      display.display();
      //rssiValue = lora.getRssi();
      //recvStatus = lora.readData(datoEntrante);
      //Serial.print("datoentrante: ");Serial.println(datoEntrante);
      
    
    }
    recvStatus = lora.readData(datoEntrante);
    if(recvStatus) {
      //lora.readData(datoEntrante);
      Serial.print("datoentrante: ");Serial.println(datoEntrante);
      break;
    }
    lora.update();
  }
  Serial.println("salidel while");
  Serial.print("recvStatus: ");Serial.println(recvStatus);
  memset(datoEntrante, 0, sizeof(datoEntrante));
  recvStatus=0;
  
}

void loop() {
    // Check Lora RX
     lora.update();
     }


// Función de interrupción para mensajes recibidos lora
void IRAM_ATTR onReceive() {
  recvStatus = lora.readData(datoEntrante); // Cambia bandera cuando hay un paquete recibido
  Serial.println("ONRECEIEVE");
  
  
}

  
