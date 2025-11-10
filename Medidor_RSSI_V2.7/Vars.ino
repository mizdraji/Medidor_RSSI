//FUNCIONES AUXILIARES
void ProcesarDatoEntrante(char *inputData, int16_t *rssi_out, int16_t *rssiRX, String *name_out){
  //inputData: datoEntrada, dato recibido completo.
  //rssi_out: rssi recibido (es el valor que llego al gateway, seria el TX del nodo).
  //rssiRX: es el valor rssi leido en el nodo cuando llega un paquete.
  //name_out: es el nombre del gateway.
  //formato del datoEntrante: rssi, gatewayname, ej: { -115, EE15 }
  //static uint8_t rcv_count = 0;
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
    rcv_count++;    //solo aumento cuando el datorecibido posee una coma
  }
  *rssiRX = lora.getRssi();

  

  memset(inputData, 0, strlen(inputData));           //Se limpia el buffer del datoEntrante.
}

// Función de interrupción para mensajes recibidos lora
void IRAM_ATTR onReceive() 
{
  if(!packetReceived){
   packetReceived = true;
  }
}
    