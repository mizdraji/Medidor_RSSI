/* Version 2.7 Medidor RSSI con heltec lora esp32 
** 2025 **
** Creado por German Mizdraji
** Ahora lora se recibe como interrupción.
*/
#include <lorawan.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

const unsigned long interval = 15000;      // 15 s interval to send message
unsigned long previousMillis = 0;          // will store last time message sent
unsigned int counter = 0;                  // message counter
int rcv_count = 0;                // contador de mensajes recibidos
int send_count = 0;               // contador de mensajes enviados
int diferencia = 3;               //diferencia para comparar send_count y rcv_count

int rssi_rcv = 0;
int rssiValue=0;
char rssiSend[20]={0};                     //variable para guardar dato a enviar en forma de string
String get_name;                           //variable para guardar gatewayname

char datoEntrante[255]={0};                //variable para guardar dato recibido
char *token;                 

byte recvStatus = 0;
char uncero[1]={0};

void IRAM_ATTR onReceive();

void setup() {
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
  //lora.setChannel(MULTI);                               // set channel to random
  
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
    
    while(!recvStatus) {                                   //cada 3 seg envio uncero para obtener una respuesta
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
 attachInterrupt(digitalPinToInterrupt(RFM_pins.DIO0), onReceive,  CHANGE);            //habilita interrupciones para mensajes recibidos lora, se utiliza CHANGE para cuando la señal cambia HIGH <-->LOW. Con RISING se generan multiples interrupciones.
}

void loop() {
  
  // Check interval overflow
  if(millis() - previousMillis > interval) {                  //cada cierto invervalo envio valor de rssi
    previousMillis = millis(); 
    lora.sendUplink(rssiSend, strlen(rssiSend), 0, 1);
    send_count++;
    //Serial.print("send count: ");Serial.println(send_count);
    
    Serial.print("Tx: ");Serial.println(rssi_rcv);
    display.setCursor(0,0);
    display.setTextSize(2);
    display.print("Tx: ");display.print(rssi_rcv);              // display.print(" dbm ");

    display.setTextSize(2);
    display.setCursor(0,30);
    display.print("Rx: ");display.print(rssiValue);
    display.setCursor(0,50);
    display.print(get_name);
    
    display.display();
    display.clearDisplay();

    //Serial.print("diferencia: ");Serial.println((send_count - rcv_count));
    if(abs(send_count - rcv_count) > diferencia){
    rcv_count = 0;
    send_count = 0;
    Serial.println("...........................................");
    display.setTextSize(2);
    display.setCursor(0,30);
    display.print("Rx: .....");
    display.display();
    display.clearDisplay();
  
    }
  }
  
//  recvStatus = lora.readData(datoEntrante);
  if(recvStatus) {

  Serial.print("datoentrante: ");Serial.println(datoEntrante);

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
    //Serial.print("gateway: ");Serial.println(get_name);
    
  }


  rcv_count++;
  //Serial.print("rcv_count: ");Serial.println(rcv_count);
  rssiValue = lora.getRssi();     //rssi de paquete recibido (downlink)
  //Serial.print("lora Get RSSI: "); Serial.println(rssiValue)
  itoa(rssiValue, rssiSend, 10);
  
  Serial.print("RSSI de bajada ====>> ");  Serial.println(rssiValue);
  display.setTextSize(2);
  display.setCursor(0,30);
  display.print("Rx: ");display.print(rssiValue);
  display.setCursor(0,50);
  display.print(get_name);

  display.setCursor(0,0);
  display.setTextSize(2);
  display.print("Tx: ");display.print(rssi_rcv);
  display.display();
  display.clearDisplay();
  memset(datoEntrante, 0, sizeof(datoEntrante)); // Borra cualquier contenido anterior
  //recvStatus = 0;
  }

    // Check Lora RX
     lora.update();
     }


// Función de interrupción para mensajes recibidos lora
 void IRAM_ATTR onReceive() {
   recvStatus = lora.readData(datoEntrante); // Cambia bandera cuando hay un paquete recibido
 }

  
