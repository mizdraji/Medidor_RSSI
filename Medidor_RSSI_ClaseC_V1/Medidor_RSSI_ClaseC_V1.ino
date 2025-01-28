//primera version envia paquetes funcionando


#include <lorawan.h>
#include "SSD1306.h"



#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define OLED_ADDR 0x3C

SSD1306  display(OLED_ADDR, OLED_SDA, OLED_SCL);

//ABP Credentials 
const char *devAddr = "01fa9919";
const char *nwkSKey = "1a0b4e5246e55ff17f405edc5dd8e65e";
const char *appSKey = "ea520a0fac3c44a80d04ea5f4bfadfce";

const unsigned long interval = 10000;    // 10 s interval to send message
unsigned long previousMillis = 0;  // will store last time message sent
unsigned int counter = 0;     // message counter

char myStr[50]="hola";
char outStr[255];
byte recvStatus = 0;


//heltec wifi lora esp32 V2
const sRFM_pins RFM_pins = {
  .CS = 18,
  .RST = 14,
  .DIO0 = 26,
  .DIO1 = 33,
  .DIO2 = 32,
  .DIO5 = -1,
};


char buffer_rx[255];

void setup() {

  pinMode(OLED_RST,OUTPUT);
  digitalWrite(OLED_RST, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(OLED_RST, HIGH);
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(5,5,"LoRa Receiver"); 
  display.display();

  display.drawString(5,25,"German");//LoRa Initializing OK!");
  display.display();

  
  // Setup loraid access
  Serial.begin(115200);
  delay(5000);
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

/*  ## For US915
| data_rate    | Name  | Config          | Direction   
|--------------|-------|-----------------|----------
| 0            | DR0   | SF10 BW 125 KHz | Uplink
| 1            | DR1   | SF9 BW 125 KHz  | Uplink
| 2            | DR2   | SF8 BW 125 KHz  | Uplink
| 3            | DR3   | SF7 BW 125 KHz  | Uplink
| 4            | DR4   | SF8 BW 500 KHz  | Uplink
| 5:7          | RFU   |    N/A          | N/A
| 8            | DR8   | SF12 BW 500 KHz  | Downlink
| 9            | DR9   | SF11 BW 500 KHz  | Downlink
| 10           | DR10  | SF10 BW 500 KHz  | Downlink
| 11           | DR11  | SF9 BW 500 KHz  | Downlink
| 12           | DR12  | SF8 BW 500 KHz  | Downlink
| 13           | DR13  | SF7 BW 500 KHz  | Downlink
*/

  // set channel to random
  lora.setChannel(MULTI);


  
  // Put ABP Key and DevAddress here
  lora.setNwkSKey(nwkSKey);
  lora.setAppSKey(appSKey);
  lora.setDevAddr(devAddr);

}

void loop() {
  
  // Check interval overflow
  if(millis() - previousMillis > interval) {
    previousMillis = millis(); 

   

    Serial.print("Sending: ");
    Serial.println(myStr);
    //Serial.println(lora.getRssi());
    


    lora.sendUplink(myStr, strlen(myStr), 0, 1);        //espera ack= 0 , con 1 no funciona por alguna razon
    
  }

  recvStatus = lora.readData(outStr);
  if(recvStatus) {
    Serial.print("====>> ");
    Serial.println(outStr);
  }
  
  // Check data
  recvStatus = lora.readData(buffer_rx);
  if(recvStatus) {
    Serial.println(buffer_rx);
  }

  //Serial.println(lora.getRssi());
  // Check Lora RX
  lora.update();
}
