
#define MAX_SF 2 //Se refiere a la posicion dentro del vector de SF, es decir el maximo sera: SF9
#define MAX_RETRY_SAME_SF 2
#define MAX_DELTA_TIME_UPDATE 28800 //8hs en seg, es el tiempo maximo que se admite para actualizar CMt
const unsigned char SFvector[4] = { SF7BW125, SF8BW125, SF9BW125, SF10BW125 }; //toma solamente hasta los primeros 4 SF
uint8_t SF_actual = 0; //para saber cual se esta usando
uint8_t SF_is_set = 0; //para saber si esta seteado el SF
uint8_t SF_index = 0; //para movernos dentro del vector
uint8_t cont_same_SF = 0; //para probar mas d euna vez cada SF

//variables para timeout
#define tick_time 100 //base de tiempo para el delay
#define timeout 80 //timeout * tick_time = tiempo de time out = 8 segundos

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
const sRFM_pins RFM_pins = {
  .CS = 18,
  .RST = 14,
  .DIO0 = 26,
  .DIO1 = 33,
  .DIO2 = 32,
  .DIO5 = -1,
};
#endif

/*La funcion send package contampla el seteo de SF
  siempre que haya un SF seteado (SF_is_set = 1), se intenta mover el SF para que no quede estancando siempre en un mismo SF
  inicialmente no hay ningun SF seteado (SF_is_set = 0), tambien cada vez que falla un ack o dato--> SF_is_set se pone en 0
*/
uint8_t sendPackage( char *data_to_send, uint8_t len, uint8_t rta_type, bool canal_por_defecto) {
  Elster_stop();
  uint8_t rta = 0; //es para almacenar el resultado de lora.readAck()
  uint8_t rcv = 0; //para hacer la consulta de datos entrantes
  uint8_t dato_ok = 0;
  uint8_t cont_timeout = 0;

  if (canal_por_defecto) {
    lora.setChannel(DEFAULT_CHANNEL);
  }
  else lora.setChannel(MULTI);
  
  if (canal_por_defecto) {
    lora.setDataRate(DEFAULT_SF);
  }
  else if (SF_is_set) {  
    if(SF_actual > 0){
      uint8_t sf_ant = SF_actual - 1;
      lora.setDataRate(SFvector[sf_ant]);
      SF_actual = sf_ant;
      if (dbspk) {
        Serial.print("Reducir SF: "); Serial.println(SF_actual);
        if (nodo.modo_wifi) { telnet.print("Reducir SF: "); telnet.println(String(SF_actual)+"\r"); }
      }
    }
    else {
      lora.setDataRate(SFvector[SF_actual]);
      if (dbspk) {
        Serial.print("usar SF_actual: "); Serial.println(SF_actual);
        if (nodo.modo_wifi) { telnet.print("usar SF_actual: "); telnet.println(String(SF_actual)+"\r"); }
      }
    }
  }
  else { //no esta seteado el SF aun...
    lora.setDataRate(SFvector[SF_index]);
    SF_actual = SF_index;
    if (dbspk) {
      Serial.print("-->Test SF: "); Serial.println(SF_index);
      if (nodo.modo_wifi) { telnet.print("-->Test SF: "); telnet.println(String(SF_index)+"\r"); }
    }
    cont_same_SF++;
    if (cont_same_SF >= MAX_RETRY_SAME_SF) {
      cont_same_SF = 0;
      SF_index++;
    }
    if (SF_index > MAX_SF) {
      SF_index = 0;
    }
  }
  
  if (canal_por_defecto) lora.sendUplink(data_to_send, len, 0, 1);
  else lora.sendUplink(data_to_send, len, 1, 1);

  switch (rta_type)
  {
    case espera_ACK:
      while (!rta && cont_timeout < timeout) {
        //Serial.println(">>esperando ack<<");
        lora.update();
        rta = lora.readAck();

        delay(tick_time);
        cont_timeout++; //contador de time out
        //Serial.print("cont_timeout: "); Serial.println(cont_timeout);
      }
      if (rta) {
        if (dbspk) {
          Serial.println("-->ack ok");
          if (nodo.modo_wifi) telnet.println("-->ack ok\r");
        }
        SF_is_set = 1;
        Elster_start();
        return 1; //llego el ack, con retur rompemos el while y salimos
      }
      else {
        if (dbspk) {
          Serial.println("-->NO ACK, time out");
          if (nodo.modo_wifi) telnet.println("-->NO ACK, time out\r");
        }
        SF_is_set = 0;
        Elster_start();
        return 0; //final por time out
      }
      break;

    case espera_Dato:
      //
      while (!dato_ok && cont_timeout < timeout) { //termina el while cuando termina el time out. dato_ok es 1 solo si el procesamiento fue correcto, sino seguira intentando hasta que termine el timout
        lora.update();
        //Serial.println(">>esperando dato<<");
        rcv  = lora.readData(datoEntrante);   //cargo los datos leidos en el array, luego los proceso, si el procesamiento esta okey, ahi recien es dato_ok = 1
        if (rcv > 1) {
          dato_ok = procesarDatoEntrante();
        }
        delay(tick_time);
        cont_timeout++; //contador de time out
      }
      if (dato_ok) {
        if (dbspk) {
          Serial.println("-->dato OK");
          if (nodo.modo_wifi) telnet.println("-->dato OK\r");
        }
        SF_is_set = 1;

        Elster_start();
        return 1;
      }
      else {
        if (dbspk) {
          Serial.println("-->dato NO ok o ningun dato recibido, time out");
          if (nodo.modo_wifi) telnet.println("-->dato NO ok o ningun dato recibido, time out\r");
        }
        SF_is_set = 0;
        
        Elster_start();
        return 0;
      }
      break;
    case no_espera_ACK:
      Elster_start();
      return 1;
    break;

  }
}

uint8_t procesarDatoEntrante( void ) {
  uint8_t cmd = atoi(&datoEntrante[0]);
  cmd_rcv=cmd;
  //Serial.print("cmd: "); Serial.println(cmd, HEX);

  //debug
  Serial.println("=============procesarDatoEntrante: DATO ENTRANTE=============");
  if (nodo.modo_wifi) telnet.println("=============procesarDatoEntrante: DATO ENTRANTE=============\r");
  for (int i = 0; i <= 40; i++)  {
    Serial.print(datoEntrante[i]);    Serial.print(" ");
    if (nodo.modo_wifi) { telnet.print(String(datoEntrante[i], HEX)); telnet.print(" "); }
  }
  Serial.println("");  Serial.println("=============================================================");
  if (nodo.modo_wifi) { telnet.println("\r"); telnet.println("=============================================================\r"); }

  if (cmd==CMD_SYNC || cmd==CMD_SYNC_V2 || cmd==CMD_IDENT_V2 || cmd==CMD_LECT_MONO_V2 || cmd==CMD_LECT_TRI_V2 || cmd==0x10) {
    unsigned long master_CMt;
    if (cmd<10) master_CMt=atoi(&datoEntrante[2]);  // cant de seg desde 1/1/2000 a las 00:00:00
    else if (cmd<100) master_CMt=atoi(&datoEntrante[3]);
    else master_CMt=atoi(&datoEntrante[4]);
    memset(datoEntrante, 0, INPUTBUFF); //Limpiar datoEntrante
    if (master_CMt>=CMt) {
      int parte_entera = master_CMt/86400;
      int32_t master_master_time = master_CMt - parte_entera*86400;
      if (nodo.is_sync==0) {
        if (master_master_time >= 0 && master_master_time <= 86400) {
          CMt=master_CMt;
          master_time = master_master_time;
          Serial.print("-->Master_CMt recibido: ");        Serial.println(master_CMt);
          if (nodo.modo_wifi) { telnet.print("-->Master_CMt recibido: "); telnet.println(String(master_CMt)+"\r"); }
          return 1;
        }
      }
      else if (master_CMt<CMt+MAX_DELTA_TIME_UPDATE) {
        //nodo ya sincronizado, actualizar hora solo si es menor que la actual + 8hs
        if (master_master_time >= 0 && master_master_time <= 86400) {
          CMt=master_CMt;
          master_time = master_master_time;
          Serial.print("-->Master_CMt actualizado: ");        Serial.println(master_CMt);
          if (nodo.modo_wifi) { telnet.print("-->Master_CMt actualizado: "); telnet.println(String(master_CMt)+"\r"); }
          return 1;
        }
      }
    }
  }
  else if (cmd==CMD_MODO_WIFI_ON) {
    modoWifi(1);
    Serial.println("-->CMD_MODO_WIFI_ON recibido. Reiniciando ");
    delay(2000);
    ESP.restart();
    delay(2000);
  }
  else if (cmd==CMD_MODO_WIFI_OFF) {
    modoWifi(0);
    Serial.println("-->CMD_MODO_WIFI_OFF recibido. Reiniciando ");
    delay(2000);
    ESP.restart();
    delay(2000);
  }
  else if (cmd==CMD_CLEAR) {
    Serial.println("-->CMD_CLEAR recibido. Limpiando memoria ");
    clearEEPROM();
    delay(2000);
    ESP.restart();
    delay(2000);
    memset(datoEntrante, 0, INPUTBUFF); //Limpiar datoEntrante
    return 1;
  }
  else if (cmd==CMD_RESET) {
    Serial.println("-->CMD_RESET recibido. Reiniciando ");
    delay(2000);
    ESP.restart();
    delay(2000);
  }
  else if (cmd==CMD_TAMPER_ACT) {
    Serial.println("-->CMD_TAMPER_ACT recibido. ");
    nodo.falso_tamper_act = 0;
    memset(datoEntrante, 0, INPUTBUFF); //Limpiar datoEntrante
    return 1;
  }
  else if (cmd==CMD_TAMPER_DES) {
    Serial.println("-->CMD_TAMPER_DES recibido. ");
    nodo.falso_tamper_des = 0;
    memset(datoEntrante, 0, INPUTBUFF); //Limpiar datoEntrante
    return 1;
  }
  else if (cmd==CMD_ACTIVACION) {
    Serial.println("-->CMD_ACTIVACION recibido. ");
    String resp_s = String(datoEntrante);
    //Serial.print("resp_s: "); Serial.println(resp_s);
    short coma1 = resp_s.indexOf(",")+1;
    short coma2 = resp_s.indexOf(",",coma1);
    short coma3 = resp_s.indexOf(",",coma2+1);
    byte mac_dest[3];
    byte macAddr_[6];
    WiFi.macAddress(macAddr_);
    Serial.print("MAC_dest: "); 
    for (int i = 0; i < 3; ++i)    {
      mac_dest[i] = datoEntrante[coma3+1+i];
      Serial.printf("%X ",mac_dest[i]);
    }
    Serial.println(""); 
    Serial.print("macAddr_: "); 
    for (int i = 0; i < 6; ++i)    {
      Serial.printf("%X ",macAddr_[i]);
    }
    Serial.println(""); 
    if (mac_dest[0]==macAddr_[3] && mac_dest[1]==macAddr_[4] && mac_dest[2]==macAddr_[5]){
      String devAddr_s = resp_s.substring(coma1,coma2);
      //Serial.print("devAddr_s: "); Serial.println(devAddr_s);
      unsigned long devID_nuevo=atoi(&datoEntrante[coma2+1]);
      //Serial.print("devID_nuevo: "); Serial.println(devID_nuevo);
      char devAddr_char[9]={0};
      devAddr_s.toCharArray(devAddr_char, 9);
      //Serial.print("devAddr_char: "); Serial.println(devAddr_char);
      writeCredToEEPROM(devID_nuevo,devAddr_char);
      memset(datoEntrante, 0, INPUTBUFF); //Limpiar datoEntrante
      mac_correcta=1;
      return 1;
    }
    else {
      mac_correcta=0;
      memset(datoEntrante, 0, INPUTBUFF); //Limpiar datoEntrante
      return 1;
    }
  }
  else if (cmd==CMD_STDBY_ON) {
    if (nodo.stdby == 0) {
      Serial.println("-->CMD_STDBY_ON recibido. El nodo se encuentra silenciado ahora");
      memset(datoEntrante, 0, INPUTBUFF); //Limpiar datoEntrante
      nodo.stdby = 1;
      cont_stdby = UN_DIA;
      writeStdby(1); //Escribir en memoria el byte stdby
    }
    return 1;
  }
  else if (cmd==CMD_STDBY_OFF) {
    if (nodo.stdby == 1) {
      Serial.println("-->CMD_STDBY_OFF recibido. No se silenciara el nodo");
      memset(datoEntrante, 0, INPUTBUFF); //Limpiar datoEntrante
      nodo.stdby = 0;
      cont_stdby = 0;
      writeStdby(0); //Escribir en memoria el byte stdby
      char uncmd[1]={CMD_STDBY_OFF};
      sendPackage(uncmd, 1, no_espera_ACK, 1);
      Serial.println("============= SEND STDBY OFF CMD =============");
      if (nodo.modo_wifi) telnet.println("============= SEND STDBY OFF CMD =============\r");
    }
    return 1;
  }
  else if (cmd==CMD_LAST_PKT) {
    Serial.println("-->CMD_LAST_PKT recibido. Buscando paquete mas reciente");
    memset(datoEntrante, 0, INPUTBUFF); //Limpiar datoEntrante
    byte slot_last_pkt = lastPL(); //Slot del paquete mas reciente
    sendStoredPkt(CMD_LAST_PKT,slot_last_pkt);
    return 1;
  }
  else if (cmd==CMD_SPEC_PKT) {
    byte slot_spec=atoi(&datoEntrante[3]);
    Serial.printf("-->CMD_SPEC_PKT recibido. Buscando paquete en slot %d\n",slot_spec);
    memset(datoEntrante, 0, INPUTBUFF); //Limpiar datoEntrante
    sendStoredPkt(CMD_SPEC_PKT,slot_spec);
    return 1;
  }
  else if (cmd==CMD_RE_SYNC) {
    Serial.println("-->CMD_RE_SYNC recibido. ");
    unsigned long master_CMt=atoi(&datoEntrante[3]);    
    memset(datoEntrante, 0, INPUTBUFF); //Limpiar datoEntrante
    int parte_entera = master_CMt/86400;
    int32_t master_master_time = master_CMt - parte_entera*86400;
    if (master_master_time >= 0 && master_master_time <= 86400) {
      CMt=master_CMt;
      master_time = master_master_time;
      Serial.print("-->Master_CMt forzado: ");        Serial.println(master_CMt);
      if (nodo.modo_wifi) { telnet.print("-->Master_CMt forzado: "); telnet.println(String(master_CMt)+"\r"); }
      get_timestamp(master_CMt);
      char response[5]={CMD_RE_SYNC,CMt_array[0],CMt_array[1],CMt_array[2],CMt_array[3]};
      sendPackage(response, 5, no_espera_ACK, 1);
      Serial.println("============= PAYLOAD RE SYNC (HEX)=============");
      if (nodo.modo_wifi) telnet.println("============= PAYLOAD RE SYNC (HEX)=============\r");
      for (int i = 0; i < 5; i++) {
        Serial.print(response[i], HEX); Serial.print(" ");
        if (nodo.modo_wifi) { telnet.print(String(response[i], HEX)); telnet.print(" "); }
      }
      Serial.println(""); Serial.println("=======================================");
      if (nodo.modo_wifi) { telnet.println("\r"); telnet.println("=======================================\r"); }
      return 1;
    }
  }
  else {
    memset(datoEntrante, 0, INPUTBUFF); //Limpiar datoEntrante
    return 0;
  }
}

uint8_t initLoraTec( void )
{
  if (lora.init())
  {
    lora.setDeviceClass(CLASS_C);
    //lora.setTxPower1(15); //max, fijo
    lora.setDataRate(DEFAULT_SF);
    if (RANDOM_CHANNEL)
    {
      lora.setChannel(MULTI);
    }
    else
    {
      lora.setChannel(DEFAULT_CHANNEL);
    }

    Serial.print("nwkSKey: "); Serial.println(nwkSKey);
    Serial.print("appSKey: "); Serial.println(appSKey);
    
    lora.setNwkSKey(nwkSKey);
    lora.setAppSKey(appSKey);
    #if defined devID
      lora.setDevAddr(devAddr); //Activacion Manual, devAddr predefinido
      Serial.print("devAddr: "); Serial.println(devAddr);
    #else 
      lora.setDevAddr(devAddr_AA); //Activacion Automatica, devAddr definido por server
      Serial.print("devAddr_AA: "); Serial.println(devAddr_AA);
    #endif
    //SF_is_set = 0;
    return 1; //todo OK
  }
  else
  {
    Serial.println("-->LoraTec: error transceptor");
    if (nodo.modo_wifi) telnet.println("-->LoraTec: error transceptor\r");
    return 0; //error de transeptor
  }
}

void sendStoredPkt(byte CMD_, byte target_slot) {
  //Envía el paquete almacenado en la posicion target_slot (0-15), 
  //Agrega 4 bytes de cabecera: {CMD_,target_slot,target_len,target_enviado}
  if (target_slot<cantSlot) {
    int target_dir = pos_mem[target_slot];
    char target_payload[PL_CONT_MONO_SIZE+4]={0};
    byte target_len=EEPROM.read(target_dir);
    byte target_enviado=readPLFromEEPROM(target_dir,target_payload);
    for (int i = PL_CONT_MONO_SIZE+4; i > 3; --i) target_payload[i]=target_payload[i-4]; //Desplazar el arreglo 4 bytes hacia atras
    byte last_position_l[1]={0}; //indica la ultima posición donde se cargo cosas a la target_payload
    char cabecera_[4]={CMD_,target_slot,target_len,target_enviado};
    chargeToTheEnd(target_payload, cabecera_, 4,last_position_l); //1
    Serial.println("============= SEND STORED PAYLOAD (HEX)=============");
    if (nodo.modo_wifi) telnet.println("============= SEND STORED PAYLOAD (HEX)=============\r");
    for (int i = 0; i < PL_CONT_MONO_SIZE+4; i++) {
      Serial.print(target_payload[i], HEX); Serial.print(" ");
      if (nodo.modo_wifi) { telnet.print(String(target_payload[i], HEX)); telnet.print(" "); }
    }
    Serial.println("");    Serial.println("=======================================");
    if (nodo.modo_wifi) { telnet.println("\r"); telnet.println("=======================================\r"); }
    sendPackage(target_payload, PL_CONT_MONO_SIZE+4, no_espera_ACK, 1);
  }
  else {
    char target_payload[3]={CMD_,0,0};
    if (target_slot<SLOTS_CANTIDAD_MAX) target_payload[1]=target_slot;
    Serial.println("============= SEND STORED PAYLOAD (HEX)=============");
    if (nodo.modo_wifi) telnet.println("============= SEND STORED PAYLOAD (HEX)=============\r");
    for (int i = 0; i < 3; i++) {
      Serial.print(target_payload[i], HEX); Serial.print(" ");
      if (nodo.modo_wifi) { telnet.print(String(target_payload[i], HEX)); telnet.print(" "); }
    }
    Serial.println("");    Serial.println("=======================================");
    if (nodo.modo_wifi) { telnet.println("\r"); telnet.println("=======================================\r"); }
    sendPackage(target_payload, 3, no_espera_ACK, 1);
  }
}
