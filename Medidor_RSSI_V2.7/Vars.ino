//FUNCIONES AUXILIARES
void ProcesarDatoEntrante(char *inputData, int16_t *rssi_out, String *name_out){
  char *token;
  
  // Obtener el primer token (rssi)
  token = strtok(inputData, ",");
  if (token != NULL) {
    int rssi = atoi(token);
    *rssi_out = rssi;             //valor rssi, salida por puntero
  }
 
  // Obtener el segundo token (gatewayname)
  token = strtok(NULL, ",");
  if (token != NULL) {
    *name_out = String(token);    //nombre gateway, salida por puntero
  }
}









// Función de interrupción para mensajes recibidos lora
void IRAM_ATTR onReceive() 
{
  if(!packetReceived){
   packetReceived = true;
  }
  //detachInterrupt(digitalPinToInterrupt(RFM_pins.DIO0));
  //Serial.println("interrupt");
}

//tarea que envia cada cierto tiempo el valor rssi
    // lora.sendUplink(rssiSend, strlen(rssiSend), 0, 1);
    // send_count++;
    // //Serial.print("send count: ");Serial.println(send_count);
    


    // //Serial.print("diferencia: ");Serial.println((send_count - rcv_count));
    // if(abs(send_count - rcv_count) > diferencia){
    // rcv_count = 0;
    // send_count = 0;
    // Serial.println("...........................................");
    // display.setTextSize(2);
    // display.setCursor(0,35);
    // display.print("Rx: .....");
    // display.display();
    // display.clearDisplay();