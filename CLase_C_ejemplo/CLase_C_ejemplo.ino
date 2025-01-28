#include <lorawan.h>

//ABP Credentials 
const char *devAddr = "ef491cef";       //"ef491cef"; modulo con bateria
const char *nwkSKey = "D8E5A8E01BDDFAF48E059992119BE745";
const char *appSKey = "3F19E1EF0DA5286E178C5B84FC9E23AE";
const char *DeviceNM = "TLV2_DPEC_24337";

const unsigned long interval = 10000;    // 10 s interval to send message
unsigned long previousMillis = 0;  // will store last time message sent
unsigned int counter = 0;     // message counter

char myStr[50];
char datoEntrante[255];
byte recvStatus = 0;
                    
const sRFM_pins RFM_pins = {
  .CS = 15,
  .RST = 0,
  .DIO0 = 5,
  .DIO1 = 5,
  .DIO2 = 5,
  .DIO5 = -1,
};

char uncero[1]={0};

void setup() {
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
  //lora.sendUplink(uncero, 1, 0, 1);
}

void loop() {
  
  // Check interval overflow
  if(millis() - previousMillis > interval) {
    previousMillis = millis(); 

    sprintf(myStr, "Counter-%d", counter); 

    Serial.print("Sending: ");
    Serial.println(myStr);
    
    lora.sendUplink(myStr, strlen(myStr), 0, 1);
    Serial.print("RSSI de subida: ");Serial.println(lora.getRssi());
    counter++;
  }

  recvStatus = lora.readData(datoEntrante);
  if(recvStatus) {
    Serial.print("RSSI de bajada ====>> ");
    Serial.println(datoEntrante);
    recvStatus=0;
    //Serial.print(RegPktRssiValue);
    
  }
  
  // Check Lora RX
  lora.update();
}
