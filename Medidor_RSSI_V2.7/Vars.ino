//FUNCIONES AUXILIARES
void ProcesarDatoEntrante(char *inputData, int16_t *rssi_out, int16_t *rssiRX, String *name_out){
  //inputData: datoEntrada, dato recibido completo.
  //rssi_out: rssi recibido (es el valor que llego al gateway, seria el TX del nodo).
  //rssiRX: es el valor rssi leido en el nodo cuando llega un paquete.
  //name_out: es el nombre del gateway.

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
  *rssiRX = lora.getRssi();
}

//necesito enviar cada cierto invervalo un valor de rssi
// void PaqueteSalida(){
//   rssiValue = lora.getRssi();       //rssi de paquete recibido (downlink)
//   itoa(rssiValue, rssiSend, 10);    //Esta función se utiliza para convertir un valor numérico(rssiValue) en una cadena de texto(rssiSend).

//   lora.sendUplink(rssiSend, strlen(rssiSend), 0, 1);
//   send_count++;
//   if(abs(send_count - rcv_count) > diferencia){
//   rcv_count = 0;
//   send_count = 0;
//   Serial.println("...........................................");
//   display.setTextSize(2);
//   display.setCursor(0,35);
//   display.print("Rx: .....");
//   display.display();
//   display.clearDisplay();
// }








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
    // 
    // 
    // //Serial.print("send count: ");Serial.println(send_count);
    


    // //Serial.print("diferencia: ");Serial.println((send_count - rcv_count));
    