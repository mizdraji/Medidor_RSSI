
/*****************************************************************************************
* Nodo Tele Lector LoRaWAN V2
*****************************************************************************************
* V2.4.3_RSSI: Versión funcional modificada de los nodos telelectores, 
* Opto conectado a pin 39.
* queda pendiente hacer mas chico el codigo sacando las cosas que no se usan.<----------
* 
*****************************************************************************************
* Desarrollado por Hugo Ferrari para Macro Intell S.A. en Marzo del 2023.
* Modificado por German Mizdraji Octubre 2023
*****************************************************************************************/
#include <lorawan.h>
#include "configuracion.h"
#include "ABPcredentials.h"
#include "Memoria.h"
#include "Hardware.h"
#include "LoraTec.h"
#include "Telnet.h"
#include "luces.h"
#include <ArduinoOTA.h>
//#include <ESP8266WiFi.h>
#include "ESP8266_Utils_OTA.hpp"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//********************************************************************************************************************//
//pins OLED
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define OLED_ADDR 0x3C
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

//******************************************************************************************************************************



int32_t prevMillis = 0;
#define interval 2000

Ticker real_time_clock;

uint8_t estado_blinker = 0;

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
  display.setTextSize(1);
  display.setCursor(0,40);
  display.print("Desarrollado por Macro Intell, departamento de Electronica.");
  display.display();
  delay(1000);
  display.clearDisplay();


  Serial.begin(SERIAL_SPEED);
  Serial.println("");
  config_pines();


  Elster_start();
  strData.IAacum=0;
  strData.nI=0;

  //real_time_clock.attach(0.01,RTC);
  real_time_clock.attach(1, RTC);                               //objeto ticker real time clock, cada 1 segundo, funcion callback RTC
  prevMillis=millis();                                           //guarda tiempo de donde va a empezar el loop
  
  prevMillis_TC = millis();
   
}

void loop() {
    if (millis() - prevMillis > interval) {            //entra 1 vez cada cierto tiempo > a 1 seg
    prevMillis = millis();

    display.setTextSize(2);
    display.setCursor(0,0);
    display.println("Medidor");
//    for(int i=0, i<10,i++){
    display.print(strData.MedidorID);
    display.display();
  //  display.clearDisplay(); 
//   Serial.println("looooooooop");
//  display.setTextSize(2);
//  display.setCursor(0,0);
//  display.println("VERSION");
//  display.display();
//  delay(500);
//  display.clearDisplay();
  }

  int rd = decode_buff();
  //telnet.loop();
  //ArduinoOTA.handle();
      
}
