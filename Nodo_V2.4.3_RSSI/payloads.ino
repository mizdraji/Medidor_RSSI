byte last_position_p[1]={0}; //indica la ultima posición donde se cargo cosas al payload
bool yasearmolacabecera=0;
unsigned long KWh_Ant[8];
unsigned long KVArh_Ant[8];
unsigned long T_S_Ant[8];
unsigned long C_T_Ant[8];


//Esta funcion verifica los datos del medidor que se actualizan periodicamente y carga en la memoria según el tipo de payload que se solicite
bool muestrearMedidor(byte tipoPayload) { //ingresar 1 para cargar payloadID, 2 para payloadContadores. Devuelve 1 si los datos estaban ok y consigue guardar
  byte loaded=0;
  get_timestamp(CMt); 
  
  //if (nodo.irda_OK && (tipoMedidor==0 || tipoMedidor==252 || tipoMedidor==253)) {
    //loadPayloadContadoresMono(strData1052,ciclo,dir_actual); //Modificar strData trifasico para que coincida con monofasico
    //if (dbrmraw && !printed) strData1052.print();
  //}
  if (nodo.irda_OK) {
    if (tipoPayload==1) { 
      loaded=loadPayloadID(strData); 
      return loaded; 
    }
    else if (tipoPayload==2) {
      loaded=loadPayloadContadoresMono(strData,ciclo,dir_actual);//Rellenar campo correspondiente al ciclo
      if (loaded==2) return 0;
      if (loaded==0) {
        checkDir();
        return 0;
      }
    }
    if (dbrmraw && !printed) printData(); 
  }
  
  if (tipoPayload==2 && loaded==1) {
    if (ciclo==8){
      dir_actual=dir_actual+PL_CONT_MONO_SIZE+2;
      slot_actual++;
      cambio_bloque_PL=1;
      if (slot_actual>=SLOTS_CANTIDAD_MAX) slot_actual=0;
      last_position_p[0]=0; 
      yasearmolacabecera=0;
      nodo.hay_paquetes_pendientes=1;
      nodo.bloqueo_reintentos=1; //Levanto el bloqueo para que no envie nada hasta que sea la hora
      //Imax=0;
      if (dbmem) {
        Serial.printf(" PAQUETE CERRADO. Slot y direccion siguiente: %d, %d \n",slot_actual,dir_actual);
        if (nodo.modo_wifi) {
          char buffers[54];
          sprintf(buffers, " PAQUETE CERRADO. Slot y direccion siguiente: %d, %d",slot_actual,dir_actual);
          telnet.println(buffers);telnet.print("\r");
        }
      }
    }
    leerPLGuardadas();
  }
  if (loaded==1) return 1;
}

void loadPayloadACT(char macAddr[]) {
  
  memset(payloadACT, 0, PL_ACT_SIZE); //Carga payloadID con '0'
  //payloadACT: CMD_ACT[1] MAC[6] = 7 bytes
  char comando[1]={CMD_ACTIVACION};
  byte last_position_p[1]={0}; //indica la ultima posición donde se cargo cosas a la payload act
  chargeToTheEnd(payloadACT, comando, 1,last_position_p); //1
  chargeToTheEnd(payloadACT, macAddr, 6,last_position_p); //6
  last_position_p[0] = 0; //

  if (dbplact) {
    Serial.println("============= PAYLOAD ACTIVACION (HEX)=============");
    for (int i = 0; i < PL_ACT_SIZE; i++) {
      Serial.print(payloadACT[i], HEX); Serial.print(" ");
    }
    Serial.println(""); Serial.println("=======================================");
  }
}

void loadPayloadST(char payloadGuardada_[], unsigned long CMtCorte_) { //ingresar array con datos y ultima marca de tiempo registrada
  char MedidorTipo_[2]={0}; 
  char MedidorID_[16]={0};
  for (int i = 0; i < 2; ++i) MedidorTipo_[i]=payloadGuardada_[i+1];
  for (int i = 0; i < 16; ++i) MedidorID_[i]=payloadGuardada_[i+3];
  //Descomponer CMtCorte_
  char CMt_array_ST[4]={0};
  for (byte i=0; i<4; i++) {
    CMt_array_ST[3-i] = lowByte(CMtCorte_);
    CMtCorte_ >>= 8;   //desplazar aqui
  }
  memset(payloadST, 0, PAYLOADST_SIZE); //Carga payloadID con '0'
  //payloadST: CMD_SYNC_V2[1] MedidorTipo[2] MedidorID[16] CMtCorte[4] = 23 bytes
  CMD[0] = CMD_SYNC_V2; //0x08
  chargeToTheEnd(payloadST, CMD, 1,last_position_p); //1
  chargeToTheEnd(payloadST, MedidorTipo_, 2,last_position_p); //2
  chargeToTheEnd(payloadST, MedidorID_, 16,last_position_p);  //16
  chargeToTheEnd(payloadST, CMt_array_ST, 4,last_position_p); //4
  last_position_p[0] = 0; //

  if (dbplst) {
    Serial.println("============= PAYLOAD START (HEX)=============");
    if (nodo.modo_wifi) telnet.println("============= PAYLOAD START (HEX)=============\r");
    for (int i = 0; i < PAYLOADST_SIZE; i++) {
      Serial.print(payloadST[i], HEX); Serial.print(" ");
      if (nodo.modo_wifi) { telnet.print(String(payloadST[i], HEX)); telnet.print(" "); }
    }
    Serial.println(""); Serial.println("=======================================");
    if (nodo.modo_wifi) { telnet.println("\r"); telnet.println("=======================================\r"); }
  }
}

bool loadPayloadID(strInfo strData) { //ingresar estructura con datos. Devuelve 1 si pudo cargar ok.

  if (strData.MedidorTipo[0]==0) return 0; //Si el MedidorTipo está vacio no cargar.

  memset(payloadID, 0, PAYLOADID_SIZE); //Carga payloadID con '0'
  //payloadID: CMD_IDENT_V2[1] MedidorTipo[2] MedidorID[16] CMt[4] = 23 bytes
  
  CMD[0] = CMD_IDENT_V2; //0x09
  chargeToTheEnd(payloadID, CMD, 1,last_position_p); //1
  chargeToTheEnd(payloadID, strData.MedidorTipo, 2,last_position_p); //2
  chargeToTheEnd(payloadID, strData.MedidorID, 16,last_position_p);  //7+9=16
  chargeToTheEnd(payloadID, CMt_array, 4,last_position_p); //4
  last_position_p[0] = 0;

  if (dbplid) {
    Serial.println("============= PAYLOAD ID (HEX)=============");
    if (nodo.modo_wifi) telnet.println("============= PAYLOAD ID (HEX)=============\r");
    for (int i = 0; i < PAYLOADID_SIZE; i++) {
      Serial.print(payloadID[i], HEX); Serial.print(" ");
      if (nodo.modo_wifi) { telnet.print(String(payloadID[i], HEX)); telnet.print(" "); }
    }
    Serial.println("");    Serial.println("=======================================");
    if (nodo.modo_wifi) { telnet.println("\r"); telnet.println("=======================================\r"); }
  }
  return 1;
}

byte loadPayloadContadoresMono(strInfo strData, byte ciclo_, int dir) { 
  //Se ingresa una estructura con los datos monofasicos, el numero de ciclo (0-8) y la direccion de memoria donde se debe guardar. Devuelve 1 si pudo guardar ok.
  //V2.1 Mono: CMD_LECT_MONO_V2[1] MedidorTipo[2] MedidorID[16] CMt0[4] CMtd[1] = 24 bytes (cabecera)
  //         KWh0[5] KVArh0[5] Volt0[1] Imed0[2] Dmd_max0[2] Tiempo_Servicio0[4] Creep_Time0[3] = 22 bytes (Lectura 0)
  //         KWh1[3] KVArh1[3] Volt1[1] Imed1[2] Dmd_max1[2] Tiempo_Servicio1[1] Creep_Time1[1]= 13 bytes (Lectura 1)
  //         KWh2[3] KVArh2[3] Volt2[1] Imed2[2] Dmd_max2[2] Tiempo_Servicio2[1] Creep_Time2[1]= 13 bytes (Lectura 2)
  //         KWh3[3] KVArh3[3] Volt3[1] Imed3[2] Dmd_max3[2] Tiempo_Servicio3[1] Creep_Time3[1]= 13 bytes (Lectura 3)
  //         KWh4[3] KVArh4[3] Volt4[1] Imed4[2] Dmd_max4[2] Tiempo_Servicio4[1] Creep_Time4[1]= 13 bytes (Lectura 4)
  //         KWh5[3] KVArh5[3] Volt5[1] Imed5[2] Dmd_max5[2] Tiempo_Servicio5[1] Creep_Time5[1]= 13 bytes (Lectura 5)
  //         KWh6[3] KVArh6[3] Volt6[1] Imed6[2] Dmd_max6[2] Tiempo_Servicio6[1] Creep_Time6[1]= 13 bytes (Lectura 6)
  //         KWh7[3] KVArh7[3] Volt7[1] Imed7[2] Dmd_max7[2] Tiempo_Servicio7[1] Creep_Time7[1]= 13 bytes (Lectura 7) = 137 Bytes (total)

  unsigned long D_Activa;
  unsigned long D_Reactiva;
  char D_Activa_char[3];
  char D_Reactiva_char[3]; 
  char Voltaje[1];
  char Corriente_Med[2];
  char Demanda_Max[2];
  unsigned int D_T_S;
  char D_T_S_char[1];
  unsigned int D_C_T;
  char D_C_T_char[1];
  int sigDir;
  bool PLCM_OK=0;

  if (strData.MedidorTipo[0]==0) {
    return 2;
  } //Si el MedidorTipo está vacio no cargar.

  //===============================================================================Armar cabecera + Muestra 0 - 8
  if (yasearmolacabecera==0){ //no se armo la cabecera aun
    if (sePuedeEscribirSlot(dir)) {
      last_position_p[0]=0;
      memset(payloadContadores, 0xFF, PL_CONT_MONO_SIZE); //Carga payloadContadores con '0xFF'
      CMD[0]=CMD_LECT_MONO_V2; //0x10
      chargeToTheEnd(payloadContadores, CMD, 1,last_position_p); //1
      chargeToTheEnd(payloadContadores, strData.MedidorTipo, 2,last_position_p); //2
      chargeToTheEnd(payloadContadores, strData.MedidorID, 16,last_position_p);  //7+9=16
      chargeToTheEnd(payloadContadores, CMt_array, 4,last_position_p); //4
      chargeToTheEnd(payloadContadores, CMtd, 1,last_position_p); //1
      //Fin de cabecera
      if (ciclo_<8) {
        KWh_Ant[ciclo_]= BCDtoDEC(strData.KWh,5);
        chargeToTheEnd(payloadContadores, strData.KWh, 5,last_position_p); //KWh [5]
        KVArh_Ant[ciclo_]= BCDtoDEC(strData.KVArh,5);
        chargeToTheEnd(payloadContadores, strData.KVArh, 5,last_position_p); //KVArh [5]
        if (tipoMedidor==1 || tipoMedidor==254 || tipoMedidor==255)  { //solo para A150
          int V_=BCDtoDEC(strData.TensionA,2);
          if (V_>508) Voltaje[0]=V_/20;
          else Voltaje[0]=V_/2;
          chargeToTheEnd(payloadContadores, Voltaje, 1,last_position_p); //Volt [1]
          int Imed=strData.IAacum/strData.nI;
          //strData.IAacum=0; strData.nI=0;
          DECtoBCD(Imed,Corriente_Med,2);
          chargeToTheEnd(payloadContadores, Corriente_Med, 2,last_position_p); //Imed [2]
        }
        else {
          Voltaje[0]=0xFF;
          Corriente_Med[0]=0xFF;Corriente_Med[1]=0xFF;
          chargeToTheEnd(payloadContadores, Voltaje, 1,last_position_p); //Volt [1]
          chargeToTheEnd(payloadContadores, Corriente_Med, 2,last_position_p); //Imed [2]
        }
        unsigned int Dmd_max_ = strData.Dmd_max/100; //Dmd_max en KWh con 1 decimal
        //strData.Dmd_max=0;
        DECtoBCD(Dmd_max_,Demanda_Max,2);
        chargeToTheEnd(payloadContadores, Demanda_Max, 2, last_position_p); //Dmd_max [2]
        lectura0=1; //Levanto esta bandera para que macroirda resetee los contadores de demanda y corriente promedio
        T_S_Ant[ciclo_]= BCDtoDEC(strData.Tiempo_Servicio,4);
        chargeToTheEnd(payloadContadores, strData.Tiempo_Servicio, 4, last_position_p); //Tiempo_Servicio [4]
        C_T_Ant[ciclo_]= BCDtoDEC(strData.Creep_Time,3);
        chargeToTheEnd(payloadContadores, strData.Creep_Time, 3, last_position_p); //Creep_Time [3]
      }
      sigDir=writePLToEEPROM(dir,payloadContadores,PL_CONT_MONO_SIZE);
      PLCM_OK=1;
      yasearmolacabecera=1;
      if (dbmem) {
        Serial.printf("Payload guardada en EEPROM. Ciclo: %d. dir: %d \n",ciclo_, dir);
        if (nodo.modo_wifi) {
          char buffers[47];
          sprintf(buffers,"Payload guardada en EEPROM. Ciclo: %d. dir: %d",ciclo_, dir);
          telnet.println(buffers);telnet.print("\r");
        }
      }
    }
    else {
      PLCM_OK=0;
      if (dbmem) {
        Serial.println("No puedo guardar en esa direccion");
        if (nodo.modo_wifi) telnet.println("No puedo guardar en esa direccion\r");
      } 
    }
  }
  
  //===============================================================================Actualizar Bloque 2 Cabecera
  else if (yasearmolacabecera && ciclo_==8) {
    /*memset(Bloque2Cabecera, 0, B2C_MONO_SIZE); 
    byte last_position_c[1]={0}; //indica la ultima posición donde se cargo cosas a la cabecera
    chargeToTheEnd(Bloque2Cabecera, strData.Tiempo_Servicio, 4, last_position_c); //4
    chargeToTheEnd(Bloque2Cabecera, strData.Creep_Time, 3, last_position_c); //3
    //DECtoBCD(Imax,Corriente_Max,2);
    //chargeToTheEnd(Bloque2Cabecera, Corriente_Max, 2,last_position_c); //2
    last_position_c[0]={0};
    actualizarCabecera(payloadContadores, Bloque2Cabecera, B2C_MONO_SIZE);
    overWriteEEPROM(dir+26,Bloque2Cabecera,B2C_MONO_SIZE);*/
    PLCM_OK=1;
    if (dbmem) {
      Serial.printf("Cerrando paquete. Ciclo: 8 dir: %d \n",dir);
      if (nodo.modo_wifi) {
        char buffers[67];
        sprintf(buffers,"Cerrando paquete. Ciclo: 8 dir: %d",dir);
        telnet.println(buffers);telnet.print("\r");
      }
    }
  } 

  //=============================================================================== Agregar Muestra 1 - 7
  else if (yasearmolacabecera && ciclo_>=0 && ciclo_<8) { //ya se armó la cabecera
    KWh_Ant[ciclo_]= BCDtoDEC(strData.KWh,5);
    KVArh_Ant[ciclo_]= BCDtoDEC(strData.KVArh,5);
    D_Activa= KWh_Ant[ciclo_] - KWh_Ant[ciclo_-1];  
    DECtoBCD(D_Activa,D_Activa_char,3);
    chargeToTheEnd(payloadContadores, D_Activa_char, 3,last_position_p); //KWh [3]
    D_Reactiva= KVArh_Ant[ciclo_] - KVArh_Ant[ciclo_-1];
    DECtoBCD(D_Reactiva,D_Reactiva_char,3);
    chargeToTheEnd(payloadContadores, D_Reactiva_char, 3,last_position_p); //KVArh [3]
    if (tipoMedidor==1 || tipoMedidor==254 || tipoMedidor==255)  { //solo para A150
      int V_=BCDtoDEC(strData.TensionA,2);
      if (V_>508) Voltaje[0]=V_/20;
      else Voltaje[0]=V_/2;
      chargeToTheEnd(payloadContadores, Voltaje, 1,last_position_p); //Volt [1]
      int Imed=strData.IAacum/strData.nI;
      //strData.IAacum=0; strData.nI=0;
      DECtoBCD(Imed,Corriente_Med,2);
      chargeToTheEnd(payloadContadores, Corriente_Med, 2,last_position_p); //Imed [2]
    }
    else {
      Voltaje[0]=0xFF;
      Corriente_Med[0]=0xFF;Corriente_Med[1]=0xFF;
      chargeToTheEnd(payloadContadores, Voltaje, 1,last_position_p); //Volt [1]
      chargeToTheEnd(payloadContadores, Corriente_Med, 2,last_position_p); //Imed [2]
    }
    unsigned int Dmd_max_ = strData.Dmd_max/100; //Dmd_max en KWh con 1 decimal
    //strData.Dmd_max=0;
    DECtoBCD(Dmd_max_,Demanda_Max,2);
    chargeToTheEnd(payloadContadores, Demanda_Max, 2, last_position_p); //Dmd_max [2]
    lectura0=1; //Levanto esta bandera para que macroirda resetee los contadores de demanda y corriente promedio
    T_S_Ant[ciclo_]= BCDtoDEC(strData.Tiempo_Servicio,4);
    D_T_S= T_S_Ant[ciclo_] - T_S_Ant[ciclo_-1];
    DECtoBCD(D_T_S, D_T_S_char, 1);
    chargeToTheEnd(payloadContadores, D_T_S_char, 1, last_position_p); //Tiempo_Servicio [1]
    C_T_Ant[ciclo_]= BCDtoDEC(strData.Creep_Time,3);
    D_C_T= C_T_Ant[ciclo_] - C_T_Ant[ciclo_-1];
    DECtoBCD(D_C_T, D_C_T_char, 1);
    chargeToTheEnd(payloadContadores, D_C_T_char, 1, last_position_p); //Creep_Time [1]

    overWriteEEPROM(dir+2,payloadContadores,PL_CONT_MONO_SIZE);
    PLCM_OK=1;
    
    if (dbmem) {
      Serial.printf("Payload actualizada en EEPROM. Se agrego muestra %d. dir: %d \n",ciclo_,dir);
      if (nodo.modo_wifi) {
        char buffers[62];
        sprintf(buffers,"Payload actualizada en EEPROM. Se agrego muestra %d. dir: %d",ciclo_,dir);
        telnet.println(buffers);telnet.print("\r");
      }
    }
    //if (ciclo_==7) {last_position_p[0]=0; yasearmolacabecera=0;}
  }

  if (dbplcm) {
    Serial.printf("============= PAYLOAD CONTADORES MONO (HEX) (CICLO: %d) =============",ciclo_);
    if (nodo.modo_wifi) {
      char buffers[68];
      sprintf(buffers,"============= PAYLOAD CONTADORES MONO (HEX) (CICLO: %d) =============",ciclo_);
      telnet.println(buffers);telnet.print("\r");
    }
    for (int i = 0; i < PL_CONT_MONO_SIZE; i++) {
      if (payloadContadores[i]<10)  {
        Serial.print("0"); Serial.print(payloadContadores[i],HEX);
        if (nodo.modo_wifi) { telnet.print("0"); telnet.print(String(payloadContadores[i],HEX)); }
      }
      else {
        Serial.print(payloadContadores[i],HEX);
        if (nodo.modo_wifi) telnet.print(String(payloadContadores[i],HEX));
      }
      Serial.print(" ");
      if (nodo.modo_wifi) telnet.print(" ");
    }
    Serial.println("=======================================");
    if (nodo.modo_wifi) telnet.println("=======================================\r");
  }
  return PLCM_OK;
}

//Esta funcion solo carga al final del payload los datos que se le pasan, se le pasa un array de bytes y la cantidad de datos (size_of)
void chargeToTheEnd(char * destination_, char data_[], byte length_, byte * last_position_) {
  byte index = 0;
  for (index = 0; index < length_; index++)
  {
    destination_[index + last_position_[0]] = data_[index];
  }
  last_position_[0] = last_position_[0] + length_;
}

void actualizarCabecera(char * destination_, char bloque2_[], byte length_) { 
  for (int i = 0; i < length_; ++i) {
    destination_[i+24]=bloque2_[i];
  }
}

unsigned long BCDtoDEC(char val_[], byte len_) { //ingresar array y cantidad de elementos
  unsigned long out=0;
  for (byte i=0; i<len_ ; i++){
    out = out * 100 + (val_[i] / 16 * 10) + (val_[i] % 16);
  }
  return out;
}

void DECtoBCD(unsigned long in_, char * out_, byte size_) {  //ingresar valor decimal, array BCD, cantidad total de elementos BCD
  char sal_[5]={0};
  byte despl=5-size_;
  byte im_=0; // indice maximo, indica la cantidad de elementos de 2 cifras que se requieren
  unsigned long aux=0;
  byte ix_=0; //Cantidad de elementos 0x00
  if (in_>=100000000) {im_=4;}
  else if (in_>= 1000000) {im_=3; ix_=1;}
  else if (in_>= 10000) {im_=2; ix_=2;}
  else if (in_>= 100) {im_=1; ix_=3;}
  else if (in_>= 1) {im_=0; ix_=4;}
  for (int i = 0; i < im_+1; ++i)  {
    aux=in_/pow(100,im_-i);
    sal_[i+ix_]=(aux / 10 * 16) + (aux % 10);
    in_= in_ - aux*pow(100,im_-i);
  }
  for (int i_ = 0; i_ < size_; ++i_) {
    out_[i_]=sal_[i_+despl];
  }
}
