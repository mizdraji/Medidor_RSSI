//Segunda version envia y recibe paquetes
//Envia un paquete, se recibe por nodered a travez de mqtt y se lee el RSSI de subida que es reenviado para su lectura en datoEntrante
//cuando se recibe el paquete se lee el RSSI del paquete recibido con lora.getRssi()
//Medidor RSSI con heltec lora esp32 
//Creado por German Mizdraji para Macro Intell S.A
//
//Version 2.7 heltek - 2025
//Reestructuración, se crean funciones de configuración para un código más limpio.

#include <TaskScheduler.h>
#include <lorawan.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "Vars.h"
#include "Display.h"
#include "Task.h"

void setup() {
  Serial.begin(SERIAL_SPEED);
  
  config_OLED();
  init_OLED();

  attachInterrupt(digitalPinToInterrupt(RFM_pins.DIO0), onReceive,  RISING);            //habilita interrupciones para mensajes recibidos lora, se utiliza CHANGE para cuando la señal cambia HIGH <-->LOW. Con RISING se generan multiples interrupciones.

  //Setup LoRa
  config_lora();

  config_task();          // config Scheduler
  
}

void loop() 
{
    PDR.execute(); // Es necesario ejecutar el runner en cada loop
    lora.update();

  recvStatus = 0;

     if(packetReceived && lora.readData(datoEntrante) > 1)   // Nuevo mensaje recibido.
     {
        packetReceived = false;
        //lora.readData(datoEntrante);
        DEBUG_PRINT("Datoentrante: "); DEBUG_PRINTLN(datoEntrante);
        ProcesarDatoEntrante(datoEntrante, &rssi_rcv, &rssiValue, &get_name);  
        //Se ingresa el datoEntrante y se obtiene: rssi_rcv (rssi recibido en el gw, es el TX del nodo).
        //rssiValue: es el valor rssi leido en el nodo cuando llega un paquete.
        //get_name: es el nombre del gateway.
        DEBUG_PRINT("rssi_rcv: "); DEBUG_PRINTLN(rssi_rcv);
        DEBUG_PRINT("rssiValue: "); DEBUG_PRINTLN(rssiValue);
        DEBUG_PRINT("get_name: "); DEBUG_PRINTLN(get_name);
        mostrarDisplay(rssi_rcv, rssiValue, &get_name);
     }
     
}




  
