/*
-  Manejo de memoria EEPROM (Flash)

*/

void initEEPROM() {
  int pos_mem_[SLOTS_CANTIDAD_MAX]={0};
  int pos_=0;
  for (int i = 0; i < SLOTS_CANTIDAD_MAX; ++i) {
    pos_mem_[i]=pos_;
    pos_=pos_ + (PL_CONT_MONO_SIZE+2);
  }
  //Init EEPROM
  if (dbmem) Serial.println("* * * iniciando EEPROM * * *");
  EEPROM.begin(EEPROM_SIZE);
  //writeStdby(1); // Marcar para que inicie en modo Stand by
  //modoWifi(1); // Marcar para que busque wifi al iniciar 
  //verificamos si el byte OTA está en 1, en ese caso buscará wifi
  byte reset_count=EEPROM.read(pos_reset_count);
  if (dbmem) Serial.print("leer reset_count: ");Serial.println(reset_count);
  if (reset_count==255) { //Si reset_count es "FF" limpiar memoria y resetear
    clearEEPROM();
    Serial.print("Reiniciando... ");
    delay(2000);
    ESP.restart();
    delay(2000);
  }
  else {
    writeResetCountToEEPROM(reset_count+1);
  }
  if (EEPROM.read(pos_modo_wifi)==1) {
    nodo.modo_wifi=1;
  }
  else nodo.modo_wifi=0;
  //Serial.print("pos_modo_wifi: ");Serial.print(pos_modo_wifi);
  if (dbmem) Serial.print("leer modo_wifi: ");Serial.println(nodo.modo_wifi);
  
  if (activacion_manual == 1) {
    nodo.is_activated=1;
    if (dbmem) Serial.println("se usaran las credenciales predefinidas");
  }
  else if (readCredFromEEPROM()==1) {
    nodo.is_activated=1;
  }
  else nodo.is_activated=0;
  if (dbmem) Serial.print("nodo activado: ");Serial.println(nodo.is_activated);

  if (EEPROM.read(pos_stdby)==1) {
    nodo.stdby=1;
    cont_stdby=UN_DIA;
  }
  else nodo.stdby=0;
  if (dbmem) Serial.print("stdby: ");Serial.println(nodo.stdby);


  slot_actual=leerPLGuardadas();
  dir_actual=pos_mem_[slot_actual];
  checkDir();
  dir_CMt_actual=leerCMtGuardados();
  if (dbmem) Serial.printf("Almacenar en direcciones dir_actual: %d dir_CMt_actual: %d \n",dir_actual,dir_CMt_actual);
  char payloadGuardada[PL_CONT_MONO_SIZE] = {0};
  readPLFromEEPROM(0,payloadGuardada); //leer la payload que está en el primer slot para armar el paquete START
  loadPayloadST(payloadGuardada,CMt_corte); //carga la payload START
}

//Esta funcion compara el byte en la posición de memoria con el que se va a escribir, si no es igual lo sobreescribe.
void updateEEPROM(int addr, byte dato_in) {
  byte dato_leido=EEPROM.read(addr);
  if (dato_leido!=dato_in) {
    EEPROM.write(addr, dato_in);
  }
}

//Esta funcion escribe en la memoria una payload completa, agrega 2 bytes al inicio que indican la longitud y flag enviado
int writePLToEEPROM(int addrOffset, char data_in_[], byte length_) {  //Ingresar direccion de memoria, array de datos y cantidad de bytes. Devuelve siguiente dirección 
  Elster_stop();
  if (addrOffset>=(SLOTS_CANTIDAD_MAX*(PL_CONT_MONO_SIZE+2))) {
    Serial.println("No hay slots libres");
    if (nodo.modo_wifi) telnet.println("No hay slots libres\r");
    Elster_start();
    return 0;
  }
  updateEEPROM(addrOffset, length_+1);
  updateEEPROM(addrOffset+1, 0x0);
  for (int i = 0; i < length_; i++)  {
    updateEEPROM(addrOffset + 2 + i, data_in_[i]);
  }
  EEPROM.commit();
  int nextAddr=addrOffset+length_+2;
  Elster_start();
  cambio_bloque_PL=1;
  return nextAddr;
}

//Esta funcion escribe en la memoria la marca de tiempo CMt, agrega 1 byte al inicio que indica la longitud
int writeCMtToEEPROM(int addrOffset, char data_in_[], byte length_) {  //Ingresar direccion de memoria, array de datos y cantidad de bytes. Devuelve siguiente dirección 
  Elster_stop();
  if ( addrOffset>=((SLOTS_CANTIDAD_MAX*(PL_CONT_MONO_SIZE+2))+(CMT_CANTIDAD_MAX*5)) || addrOffset<SLOTS_CANTIDAD_MAX*(PL_CONT_MONO_SIZE+2)) {
    Serial.println("La direccion no corresponde");
    if (nodo.modo_wifi) telnet.println("La direccion no corresponde\r");
    Elster_start();
    return SLOTS_CANTIDAD_MAX*(PL_CONT_MONO_SIZE+2);
  }
  updateEEPROM(addrOffset, length_+1);
  for (int i = 0; i < length_; i++)  {
    updateEEPROM(addrOffset + 1 + i, data_in_[i]);
  }
  EEPROM.commit();
  int nextAddr=addrOffset+length_+1;
  if (nextAddr>=((SLOTS_CANTIDAD_MAX*(PL_CONT_MONO_SIZE+2))+(CMT_CANTIDAD_MAX*5))) {
    nextAddr=SLOTS_CANTIDAD_MAX*(PL_CONT_MONO_SIZE+2);
  }
  Elster_start();
  return nextAddr;
}

//Esta funcion lee el bloque escrito en la posición de memoria addrOffset y lo guarda en el array data_out_. Devuelve el byte "enviado"
byte readPLFromEEPROM(int addrOffset, char * data_out_) {
  byte length_=EEPROM.read(addrOffset);
  byte enviado_=EEPROM.read(addrOffset+1);
  for (int i = 0; i < length_; i++)  {
    data_out_[i]=EEPROM.read(addrOffset + 2 + i);
  }
  return enviado_;
}

//Esta funcion lee el bloque CMt escrito en la posición de memoria addrOffset y lo guarda en el array data_out_. 
void readCMtFromEEPROM(int addrOffset, char * data_out_) {
  byte length_=EEPROM.read(addrOffset);
  for (int i = 0; i < length_; i++)  {
    data_out_[i]=EEPROM.read(addrOffset + 1 + i);
  }
}

//Esta funcion sobreescribe los datos en addr, no agrega bytes adicionales.
void overWriteEEPROM(int addr, char data_in_[], byte length_) {
  Elster_stop();
  for (int i = 0; i < length_; i++)  {
    updateEEPROM(addr + i, data_in_[i]);
  }
  EEPROM.commit();
  Elster_start();
}

//Esta funcion lee los bloques de memoria (slots) guardados y los muestra por consola. Devuelve el siguiente slot donde se escribiría
byte leerPLGuardadas(){   
  int pos=0;
  byte slot=0;
  if (EEPROM.read(0)==0) {
    cantSlot=0;
    return 0;
  }
  if (dbmem&&dbplg) {
    Serial.println("Payloads guardadas: ");
    if (nodo.modo_wifi) telnet.println("Payloads guardadas:  \r");
  }
  while (EEPROM.read(pos)>=PL_CONT_MONO_SIZE) {
    if(slot<SLOTS_CANTIDAD_MAX){
      pos_mem[slot]=pos;  
    }
    else if (slot>=SLOTS_CANTIDAD_MAX) break;
    char payloadGuardada[PL_CONT_MONO_SIZE] = {0};
    byte env=readPLFromEEPROM(pos,payloadGuardada);
    if (dbmem&&dbplg) {
      Serial.printf("#%d--> pos: %d; env: %d; ",slot,pos,env);
      if (nodo.modo_wifi) {
        char buffers[26];
        sprintf(buffers,"#%d--> pos: %d; env: %d; ",slot,pos,env);
        telnet.print(buffers);
      }
      for (int i = 0; i < PL_CONT_MONO_SIZE; i++){
        if (payloadGuardada[i]<10) { Serial.print("0"); Serial.print(payloadGuardada[i],HEX); }
        else Serial.print(payloadGuardada[i],HEX);
        Serial.print(" ");
      }
      Serial.println("");
      if (nodo.modo_wifi) {
        for (int i = 0; i < PL_CONT_MONO_SIZE; i++){
          if (payloadGuardada[i]<10)  { telnet.print("0"); telnet.print(String(payloadGuardada[i],HEX)); }
          else telnet.print(String(payloadGuardada[i],HEX));
          telnet.print(" ");
        }
        telnet.println("\r");
      }
    } 
    pos= pos+PL_CONT_MONO_SIZE+2;
    slot++;
  }
  cantSlot=slot;
  if (dbmem&&dbplg) {
    Serial.printf("Se encontraron %d slots \n",cantSlot);
    if (nodo.modo_wifi) {
      char buffers[24];
      sprintf(buffers,"Se encontraron %d slots ",cantSlot);
      telnet.println(buffers);telnet.print("\r");
    } 
  }
  if (cantSlot>=SLOTS_CANTIDAD_MAX) {
    pos=0; slot=0;
  }
  return slot;
}

//Esta funcion lee los bloques de memoria CMt guardados e identifica el mayor. Devuelve la siguiente direccion donde se escribiría
int leerCMtGuardados(){   
  int pos=SLOTS_CANTIDAD_MAX*(PL_CONT_MONO_SIZE+2);
  int pos_mayor=pos;
  byte slot=0;
  byte cantSlot_=0;
  unsigned long CMt_corte_leido=0;
  if (EEPROM.read(pos)==0) {
    cantSlot_=0;
    return pos;
  }
  while (EEPROM.read(pos)==0x05) {
    char CMt_array_[4] = {0};
    readCMtFromEEPROM(pos,CMt_array_);
    unsigned long CMt_corte_leido = (unsigned long)CMt_array_[0] << 24 | (unsigned long)CMt_array_[1] << 16 | (unsigned long)CMt_array_[2] << 8 | CMt_array_[3];
    if (dbmem&&dbcmtc) {
      Serial.printf("CMt corte leido #%d (HEX): ",slot);
      if (nodo.modo_wifi) {
        char buffers[27];
        sprintf(buffers,"CMt corte leido #%d (HEX): ",slot);
        telnet.print(buffers);
      }
      for (int i = 0; i < 4; i++){
        Serial.print(CMt_array_[i],HEX);
        Serial.print(" ");
      }
      Serial.printf("; (DEC): %d; Posicion en memoria: %d \n",CMt_corte_leido,pos);
      if (nodo.modo_wifi) {
        char buffers[45];
        sprintf(buffers,"; (DEC): %d; Posicion en memoria: %d",CMt_corte_leido,pos);
        telnet.println(buffers);telnet.print("\r");
      } 
    }
    if (CMt_corte_leido>CMt_corte) {
      CMt_corte=CMt_corte_leido;
      pos_mayor=pos;
    }
    pos= pos+5;
    slot++;
    if (slot>=CMT_CANTIDAD_MAX) break;
  }
  cantSlot_=slot;
  if (dbmem) {
    Serial.printf("Se encontraron %d slots CMt\n",cantSlot_); 
    Serial.printf("El CMt_corte mayor es: %d en la direccion %d \n",CMt_corte,pos_mayor);
    if (nodo.modo_wifi) {
      char buffers[82];
      sprintf(buffers,"Se encontraron %d slots CMt\nEl CMt_corte mayor es: %d en la direccion %d",cantSlot_,CMt_corte,pos_mayor);
      telnet.println(buffers);telnet.print("\r");
    }
  }
  if (cantSlot_<CMT_CANTIDAD_MAX) {
    return pos;
  }
  else if (pos_mayor>=((SLOTS_CANTIDAD_MAX*(PL_CONT_MONO_SIZE+2))+(CMT_CANTIDAD_MAX*5))-5) {
    return SLOTS_CANTIDAD_MAX*(PL_CONT_MONO_SIZE+2);
  }
  else return pos_mayor+5;
}

//Esta funcion verifica que el slot se pueda escribir (si esta vacio o tiene marca de enviado)
bool sePuedeEscribirSlot(int addr) {
  if (addr>=(SLOTS_CANTIDAD_MAX*(PL_CONT_MONO_SIZE+2))) { //Se excede del espacio de memoria, no es escribir
    return 0;
  }
  if (EEPROM.read(addr)==0) { //Es porque el slot está vacío, escribir
    return 1;
  }
  else if (EEPROM.read(addr+1)==1) { //Enviado, sobreescribir
    return 1; 
  }
  else if (EEPROM.read(addr+1)==2) { //Papelera (proximo a sobreescribirse)
    return 1;
  }
  else return 0;  //No enviado, no escribir
}

//Esta funcion devuelve 1 si la memoria se lleno con payloads no leidas
bool memoriaLlena() {
  int pos_=0;
  for (int i = 0; i < SLOTS_CANTIDAD_MAX; ++i){
    pos_=pos_ + PL_CONT_MONO_SIZE + 2; 
    if (EEPROM.read(pos_)==0) { //la longitud es cero, slot vacio
      return 0;
    }
    else if (EEPROM.read(pos_+1)==1) { //ya fue enviado, se puede escribir
      return 0;
    }
    else if (EEPROM.read(pos_+1)==2) { //esta en papelera, se puede escribir
      return 0;
    }
  }
  return 1;
}

void marcarPaquete(int addr, byte marca) {
  Elster_stop();
  updateEEPROM(addr+1,marca);
  EEPROM.commit();
  Elster_start();
}

//Esta funcion verifica la memoria globalmente y reasigna direcciones
void checkDir() {
  int pos_mem_[SLOTS_CANTIDAD_MAX]={0};
  int pos_=0;
  for (int i = 0; i < SLOTS_CANTIDAD_MAX; ++i) {
    pos_mem_[i]=pos_;
    pos_=pos_ + PL_CONT_MONO_SIZE + 2; 
  }
  if (!sePuedeEscribirSlot(dir_actual) && memoriaLlena()){ //si no se puede escribir en la direccion y la memoria está llena, marcar todo como papelera
    if (dbmem) {
      Serial.println("La memoria esta llena. Se marcaran como papelera todos los slot.");
      if (nodo.modo_wifi) telnet.println("La memoria esta llena. Se marcaran como papelera todos los slot.\r");
    }
    for (int i = 0; i < SLOTS_CANTIDAD_MAX; ++i) {
      marcarPaquete(pos_mem_[i],2);  
    } 
    cambio_bloque_PL=1;
  }
  if (!sePuedeEscribirSlot(dir_actual) && !memoriaLlena()) { //no se puede escribir en esa direccion pero hay otras libres
    if (dbmem) {
      Serial.printf("No se puede escribir en la direccion %d \n",dir_actual);
      if (nodo.modo_wifi) { telnet.print("No se puede escribir en la direccion "); telnet.println(String(dir_actual)+"\r"); }
    }
    bool haylugarenelfondo=0;
    for (int i = slot_actual; i < SLOTS_CANTIDAD_MAX; ++i) {
      if (sePuedeEscribirSlot(pos_mem_[i])) {
        dir_actual=pos_mem_[i];
        slot_actual=i;
        haylugarenelfondo=1;
        if (dbmem) {
          Serial.printf("Hay lugar en el fondo, Usar direccion %d \n",dir_actual);
          if (nodo.modo_wifi) { telnet.print("Hay lugar en el fondo, Usar direccion "); telnet.println(String(dir_actual)+"\r"); }
        }
        break;
      }
    }
    if (!haylugarenelfondo) { //No hay direcciones de memoria > dir_actual disponibles
      for (int i = 0; i < slot_actual; ++i) {
        if (sePuedeEscribirSlot(pos_mem_[i])) {
          dir_actual=pos_mem_[i];
          slot_actual=i;
          if (dbmem) {
            Serial.printf("Usar direccion %d \n",dir_actual);
            if (nodo.modo_wifi) { telnet.print("Usar direccion "); telnet.println(String(dir_actual)+"\r"); }
          }
          break;
        }
      }
    }
  }
}

int dirParaEnviar() { //devuelve la direccion del primer paquete pendiente (enviado=0 o =2), si no hay paquetes pendientes devuelve -1
  if (cambio_bloque_PL) {
    //Si algo cambio en la memoria buscar posicion del bloque PL más reciente
    unsigned long mayor_CMt=0;
    byte slot_del_mayor=SLOTS_CANTIDAD_MAX-1;
    for (int i = 0; i < SLOTS_CANTIDAD_MAX; ++i) {
      byte len_= EEPROM.read(pos_mem[i]);
      byte enviado_= EEPROM.read(pos_mem[i]+1);
      if (len_>0 && (enviado_==0 || enviado_==2) && pos_mem[i]!=dir_actual) { //el slot no esta vacio y no fue enviado, no devuelve la dir del paquete actual
        char CMt_aux_[4] = {0};
        for (int ii = 0; ii < 4; ii++)  {
          CMt_aux_[ii]=EEPROM.read(pos_mem[i] + 21 + ii);
        }
        unsigned long CMt_aux = (unsigned long)CMt_aux_[0] << 24 | (unsigned long)CMt_aux_[1] << 16 | (unsigned long)CMt_aux_[2] << 8 | CMt_aux_[3];
        if (CMt_aux>mayor_CMt) { //Busca el CMt más reciente
          mayor_CMt=CMt_aux;
          slot_del_mayor=i;
        }
      }
    }
    if (dbmem) Serial.printf("Cambio bloque PL, enviar primero el slot %d, CMt: %ld\n",slot_del_mayor,mayor_CMt);
    slot_para_enviar=slot_del_mayor;
    cambio_bloque_PL=0;
  }

  for (byte i = 0; i < SLOTS_CANTIDAD_MAX; ++i) { 
    byte slot_para_evaluar=SLOTS_CANTIDAD_MAX;
    if (i<=slot_para_enviar) slot_para_evaluar=slot_para_enviar-i; //Decrementa para que se envíe los paquetes de más reciente a menos reciente
    else {
      slot_para_evaluar=SLOTS_CANTIDAD_MAX-(i-slot_para_enviar);
      if (dbmem) Serial.printf("Slot para evaluar=%d, dir=%d, i=%d\n",slot_para_evaluar,pos_mem[slot_para_evaluar],i);
    }
    byte len_= EEPROM.read(pos_mem[slot_para_evaluar]);
    byte enviado_= EEPROM.read(pos_mem[slot_para_evaluar]+1);
    if (len_>0 && (enviado_==0 || enviado_==2) && first_time) { //el slot no esta vacio y no fue enviado, devuelve la dir del paquete actual la primera vez
      slot_para_enviar=slot_para_evaluar;
      if (dbmem) Serial.printf("Primera vez. Slot para enviar: %d, dir: %d, i: %d\n",slot_para_enviar,pos_mem[slot_para_enviar],i);
      return pos_mem[slot_para_enviar];
    }
    else if (len_>0 && (enviado_==0 || enviado_==2) && pos_mem[slot_para_evaluar]!=dir_actual) { //el slot no esta vacio y no fue enviado, no devuelve la dir del paquete actual
      slot_para_enviar=slot_para_evaluar;
      if (dbmem) Serial.printf("Dir actual: %d, Slot para enviar: %d, dir: %d, i: %d\n",dir_actual,slot_para_enviar,pos_mem[slot_para_enviar],i);
      return pos_mem[slot_para_enviar];
    }
  }
  return -1;
}

void modoWifi(bool in){
  Elster_stop();
  updateEEPROM(pos_modo_wifi,in);
  EEPROM.commit();
  Elster_start();
}

void clearEEPROM() {
  Serial.println("Limpiando memoria EEPROM");
  // apagar el led integrado
  digitalWrite(pin_led, HIGH);
  Elster_stop();
  EEPROM.begin(EEPROM_SIZE);
  // write a 0 to all bytes of the EEPROM
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  // encender y apagar el led integrado 2 veces
  digitalWrite(pin_led, LOW);
  delay(1000);
  digitalWrite(pin_led, HIGH);
  delay(1000);
  digitalWrite(pin_led, LOW);
  delay(1000);
  digitalWrite(pin_led, HIGH);
  delay(1000);
  digitalWrite(pin_led, LOW);
  EEPROM.end();
  Elster_start();
  cambio_bloque_PL=1;
}
//Esta funcion escribe en la memoria las credenciales de activación, agrega 2 byte que indican la longitud 
void writeCredToEEPROM(unsigned long devID_in, char devAddr_in[]) {  
  Elster_stop();
  char devID_array_[4];
  for (byte i=0; i<4; i++) { //Descomponer long
    devID_array_[3-i] = lowByte(devID_in);
    devID_in >>= 8;   //desplazar aqui
  }
  updateEEPROM(pos_bit_act , 0x01); //bit de activacion, escribir "1"
  updateEEPROM(pos_devID , 0x05); //longitud de devID (5 bytes)
  for (int i = 0; i < 4; i++)  { //escribir devID
    updateEEPROM(pos_devID + 1 + i, devID_array_[i]);
  }
  updateEEPROM(pos_devAddr , 0x09); //longitud de devAddr (9 bytes)
  for (int i = 0; i < 8; i++)  { //escribir devAddr
    updateEEPROM(pos_devAddr + 1 + i, devAddr_in[i]);
  }
  EEPROM.commit();
  Elster_start();
}

//Esta funcion lee el bloque de credenciales, devuelve el bit de activacion
byte readCredFromEEPROM() {
  byte bit_act_=EEPROM.read(pos_bit_act);
  if (bit_act_==0x01) {
    nodo.is_activated=1;
    byte length_=EEPROM.read(pos_devID);
    char devID_out_[4]={0};
    for (int i = 0; i < length_-1; i++)  {
      devID_out_[i]=EEPROM.read(pos_devID + 1 + i);
    }
    devID_AA = (unsigned long)devID_out_[0] << 24 | (unsigned long)devID_out_[1] << 16 | (unsigned long)devID_out_[2] << 8 | devID_out_[3];
    Serial.print("devID leido: "); Serial.println(devID_AA);
    length_=EEPROM.read(pos_devAddr);
    //char devAddr_out_[8]={0};
    for (int i = 0; i < length_-1; i++)  {
      devAddr_AA[i]=EEPROM.read(pos_devAddr + 1 + i);
    }
    Serial.print("devAddr leido: "); Serial.println(devAddr_AA);
    //for (int i = 0; i < 8; ++i) Serial.print(devAddr[i]); Serial.println(" ");
  }
  return bit_act_;
}

void writeResetCountToEEPROM(byte reset_count_) {
  Elster_stop();
  updateEEPROM(pos_reset_count,reset_count_);
  EEPROM.commit();
  Elster_start();
}

void writeStdby(bool in){
  Elster_stop();
  updateEEPROM(pos_stdby,in);
  EEPROM.commit();
  Elster_start();
}

byte lastPL() {
  //Devuelve el slot del paquete PL guardado mas reciente. Si la memoria esta vacia, devuelve SLOTS_CANTIDAD_MAX
  unsigned long mayor_CMt=0;
  byte slot_del_mayor=SLOTS_CANTIDAD_MAX;
  for (int i = 0; i < SLOTS_CANTIDAD_MAX; ++i) {
    byte len_= EEPROM.read(pos_mem[i]);
    byte enviado_= EEPROM.read(pos_mem[i]+1);
    if (len_>0) { //el slot no esta vacio
      char CMt_aux_[4] = {0};
      for (int ii = 0; ii < 4; ii++)  {
        CMt_aux_[ii]=EEPROM.read(pos_mem[i] + 21 + ii);
      }
      unsigned long CMt_aux = (unsigned long)CMt_aux_[0] << 24 | (unsigned long)CMt_aux_[1] << 16 | (unsigned long)CMt_aux_[2] << 8 | CMt_aux_[3];
      if (CMt_aux>mayor_CMt) { //Busca el CMt más reciente
        mayor_CMt=CMt_aux;
        slot_del_mayor=i;
      }
    }
  }
  return slot_del_mayor;
}
