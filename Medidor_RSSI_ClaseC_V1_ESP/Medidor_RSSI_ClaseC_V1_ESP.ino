//primera version envia paquetes funcionando


#include <lorawan.h>
//#include <LoRa.h>

//int txPower = 14;       // Nivel de potencia de transmisión en dBm
//int outputPin = PA_OUTPUT_PA_BOOST_PIN; 

#define DEFAULT_CHANNEL CH0
#define DEFAULT_SF SF7BW125


//ABP Credentials 
const char *devAddr = "ef491cef";
const char *nwkSKey = "D8E5A8E01BDDFAF48E059992119BE745";
const char *appSKey = "3F19E1EF0DA5286E178C5B84FC9E23AE";

const unsigned long interval = 20000;    // 10 s interval to send message
unsigned long previousMillis = 0;  // will store last time message sent
unsigned int counter = 0;     // message counter

char myStr[10]="hola";
char buffer_rx[255]={0};
byte recvStatus = 0;


//esp8266
const sRFM_pins RFM_pins = {
  .CS = 15,
  .RST = 0,
  .DIO0 = 5,
  .DIO1 = 5,
  .DIO2 = 5,
  .DIO5 = -1,
};


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

  //RFM_Set_Tx_Power(20, PA_BOOST_PIN);
  //lora.setTxPower1(txPower, outputPin);
 // LoRa.setTxPower(txPower, outputPin);

  // Set Data Rate
  lora.setDataRate(DEFAULT_SF);

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

  // set channel default
  lora.setChannel(DEFAULT_CHANNEL);     //MULTI: set channel random

  // Put ABP Key and DevAddress here
  lora.setNwkSKey(nwkSKey);
  lora.setAppSKey(appSKey);
  lora.setDevAddr(devAddr);
  lora.sendUplink(myStr, strlen(myStr), 0, 1);
}

void loop() {

//Check interval overflow
 if(millis() - previousMillis > interval) {
   previousMillis = millis(); 

   Serial.print("Sending: ");
    Serial.println(myStr);
  //Serial.println(lora.getRssi());
    


  lora.sendUplink(myStr, strlen(myStr), 0, 1);        //espera ack= 0 , con 1 no funciona por alguna razon
    
 }

  
  // Check data
  recvStatus = lora.readData(buffer_rx);
  if(recvStatus) {
    Serial.println(buffer_rx);
    Serial.println("recibi algo no se ");
  }

  //Serial.println(lora.getRssi());
  // Check Lora RX
  lora.update();
}
