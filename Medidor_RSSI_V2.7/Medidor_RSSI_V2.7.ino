//Segunda version envia y recibe paquetes
//Envia un paquete, se recibe por nodered a travez de mqtt y se lee el RSSI de subida que es reenviado para su lectura en datoEntrante
//cuando se recibe el paquete se lee el RSSI del paquete recibido con lora.getRssi()
//Medidor RSSI con heltec lora esp32 
//Creado por German Mizdraji para Macro Intell S.A
//
//Version 2.7 heltek - 2025
//Reestructuración, se crean funciones de configuración para un código más limpio.

#include <lorawan.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "Vars.h"
#include "Display.h"

void setup() {
  Serial.begin(SERIAL_SPEED);
  
  config_OLED();
  init_OLED();

  //Setup LoRa
  config_lora();

  attachInterrupt(digitalPinToInterrupt(RFM_pins.DIO0), onReceive,  RISING);            //habilita interrupciones para mensajes recibidos lora, se utiliza CHANGE para cuando la señal cambia HIGH <-->LOW. Con RISING se generan multiples interrupciones.
  
}

void loop() {
    // Check Lora RX
     lora.update();
     }


// Función de interrupción para mensajes recibidos lora
void IRAM_ATTR onReceive() {
  recvStatus = lora.readData(datoEntrante); // Cambia bandera cuando hay un paquete recibido
  Serial.print("Datoentrante: "); Serial.println(datoEntrante);
  Serial.println("ONRECEIEVE");
  
  
}

  
