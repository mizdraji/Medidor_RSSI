//Configuración lora
void config_lora()
{
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
}