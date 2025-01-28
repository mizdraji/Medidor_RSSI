//Segunda version envia y recibe paquetes
//Envia un paquete, se recibe por nodered a travez de mqtt y se lee el RSSI de subida que es reenviado para su lectura en datoEntrante
//cuando se recibe el paquete se lee el RSSI del paquete recibido con lora.getRssi()
//Medidor RSSI con heltec lora esp32 
//Creado por German Mizdraji para Macro Intell S.A
//2023
//Version 2.0

#include <lorawan.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define VERSION "Medidor RSSI                V2.0"       //Version

#define SERIAL_SPEED 115200         //Serial

//ABP Credentials 
const char *devAddr = "ef491cef";       //"ef491cef"; modulo con bateria
const char *nwkSKey = "D8E5A8E01BDDFAF48E059992119BE745";
const char *appSKey = "3F19E1EF0DA5286E178C5B84FC9E23AE";
const char *DeviceNM = "TLV2_DPEC_24337";

const unsigned long interval = 1000;    // 10 s interval to send message
unsigned long previousMillis = 0;  // will store last time message sent
unsigned int counter = 0;     // message counter

int rssiValue=0;
char datoEntrante[255]={0};
byte recvStatus = 0;
char uncero[1]={0};

#define tipo_modulo 1     //Es para definir el pinout de SPI con el modulo Lora. 0: ESP12, 1: ESP32

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

//pins OLED
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define OLED_ADDR 0x3C
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
//-------------------------------------------------------------------------------

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

  // set channel to random
  lora.setChannel(CH0);
  
  // Put ABP Key and DevAddress here
  lora.setNwkSKey(nwkSKey);
  lora.setAppSKey(appSKey);
  lora.setDevAddr(devAddr);
  
  Serial.print("Device name: "); Serial.println(DeviceNM);
  Serial.print("nwkSKey: "); Serial.println(nwkSKey);
  Serial.print("appSKey: "); Serial.println(appSKey);
  Serial.print("devAddr: "); Serial.println(devAddr);


 
}

void loop() {
  
  // Check interval overflow
  if(millis() - previousMillis > interval) {    //cada cierto invervalo envio uncero
    previousMillis = millis(); 
  //  rssiValue = 0;

  //  sprintf(myStr, "Counter-%d", counter); 

    Serial.print("Sending: ");
    Serial.println("uncero");
    
    lora.sendUplink(uncero, strlen(uncero), 0, 1);
    Serial.print("Tx: ");Serial.println(datoEntrante);
    display.setCursor(0,0);
    display.setTextSize(1);
    display.print("Tx: ");
    display.setCursor(0,10);
    display.setTextSize(2);
    display.print(datoEntrante);  display.print(" dbm ");

    display.setTextSize(1);
    display.setCursor(0,35);
    display.print("Rx: ");
    display.setTextSize(2);
    display.setCursor(0,45);
    display.print(rssiValue);display.print(" dbm ");
    display.display();
    display.clearDisplay();

    //counter++;
     }

  recvStatus = lora.readData(datoEntrante);
  if(recvStatus) {
 
  rssiValue = lora.getRssi();     //rssi de paquete recibido (downlink)

  Serial.print("RSSI de bajada ====>> ");  Serial.println(rssiValue);
  display.setTextSize(1);
  display.setCursor(0,35);
  display.print("Rx: ");
  display.setTextSize(2);
  display.setCursor(0,45);
  display.print(rssiValue);display.print(" dbm ");
  
  display.setCursor(0,0);
  display.setTextSize(1);
  display.print("Tx: ");
  display.setCursor(0,10);
  display.setTextSize(2);
  display.print(datoEntrante);display.print(" dbm ");
  display.display();
  display.clearDisplay();
   //recvStatus = 0;
     }
  
  // Check Lora RX
  lora.update();
}
