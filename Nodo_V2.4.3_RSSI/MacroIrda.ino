/*
<----------------------------------------------
Macro Irda 1052 150 v1: Lector del puerto serial infrarrojo de medidores Elster A1052 plus, A102 y A150.
Desarrollado por Hugo Ferrari para Macro Intell S.A. en Abril del 2021
Esta versión detecta el tipo de medidor por la transmision del puerto de datos seriales infrarrojos
y se configura para obtener los parámetros de cada uno según corresponda.

Arranca intentando decodificar un A150, si falla 3 veces cambia de medidor. Si vuelve a fallar 3 veces 
cambia de nuevo hasta volver al A150. Tener en cuenta que para que considere erronea una trama de A1052 se necesitan
8 tramas de A150.

<----------------------------------------------
*/

void Elster_start(void) {
  inhibir_tamper=0;
  pinMode(intPin, INPUT_PULLUP);
  in = out = 0;
  attachInterrupt(digitalPinToInterrupt(intPin), detectaIrda, RISING);
  if(dbirda) Serial.println("(Inicio lectura Medidor)");
  last_us = micros();
}

void Elster_stop( void ) {
  inhibir_tamper=1;
  detachInterrupt(digitalPinToInterrupt(intPin));
  if(dbirda) Serial.println("(Detenido lectura Medidor)");
}



void IRAM_ATTR detectaIrda( void ) {
  unsigned long us = micros();
  unsigned long diff = us - last_us; //diff almacena el tiempo en us que transcurre entre interrupciones
  x = diff / BIT_PERIOD;
  decimal = modf(x, &entero);
  if ((decimal > 0.95) || (decimal < 0.05)) {
    last_us = us;
    int next = in + 1;
    if (next >= BUFF_SIZE) next = 0;
    data[in] =  ((diff + (BIT_PERIOD / 2)) / BIT_PERIOD);
    in = next;
  }
}


static int decode_buff(void) {
  if (in == out) return 0;
  int next = out + 1;
  if (next >= BUFF_SIZE) next = 0;
  int p = data[out]; //p es el valor entero mas cercano de tiempos de bit entre interrupciones
  
  //if (dbug) { Serial.print(data[out]); Serial.print(" "); if (p>500) Serial.println("<----------------------------------------------"); }   
  if (p>500) {   //si ha pasado mucho tiempo desde que no hay interrupciones, resetear los contadores
    err=0;
    pausas++;
    if (pausas>3) {
      detectaTipoMedididor();
      pausas=0;
    }
    cont_1m_t=0; //Poner a cero el contador de tiempo del falso tamper
    if (dbint) {
      Serial.println("<----------------------------------------------");   
      display.setTextSize(2);
      display.setCursor(0,50);
      display.print("<----------");
      display.display(); 
      display.clearDisplay();
    }
    idx = id = pSum = par = printed = idt = 0;
    trama150ok=trama1052ok=trama102ok=trama103ok=0;
    for (int i = 0; i < 190; ++i) {
      trama[i]=0;
    }
    for (int i = 0; i < 11; ++i){
      p_buff[i]=0;
    } 
    out = next;

    return 0;
  }
  if (err) {        
    out = next;
    return 0;
  }

  p_buff[id]=p;
  id++;
  pSum = (pSum==10)? p : ((pSum+p>10)? 10: pSum + p); //(expresión relacional) ? (expresion V) : (expresion F)

  if (pSum >= 10) {
    idx++; 
    id=0;
    dato = decodificador();
    for (int i = 0; i < 10; ++i) {
      p_buff[i]=0;
    } 
  }
  out = next;                 
  return 0;
}

byte decodificador() {
  int j = 0;
  for (int i = 0; i < 10; ++i)  {
    int tBit = p_buff [i];
    switch (tBit){
      case 0 : 
        break;
      case 1 : 
        buffOut [j] = 0;
        j++;
        break;
      case 2:
        buffOut [j] = 0;
        j++;
        buffOut [j] = 1;
        j++;
        break;
      case 3:
        buffOut [j] = 0;
        j++;
        buffOut [j] = 1;
        j++;      
        buffOut [j] = 1;
        j++;
        break;
      case 4:
        buffOut [j] = 0;
        j++;
        buffOut [j] = 1;
        j++;      
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        break;
      case 5:
        buffOut [j] = 0;
        j++;
        buffOut [j] = 1;
        j++;      
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        break;
      case 6:
        buffOut [j] = 0;
        j++;
        buffOut [j] = 1;
        j++;      
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        break;
      case 7:
        buffOut [j] = 0;
        j++;
        buffOut [j] = 1;
        j++;      
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;      
        buffOut [j] = 1;
        j++;
        break;
      case 8:
        buffOut [j] = 0;
        j++;
        buffOut [j] = 1;
        j++;      
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;      
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        break;
      case 9:
        buffOut [j] = 0;
        j++;
        buffOut [j] = 1;
        j++;      
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;      
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        break;
      case 10:
        buffOut [j] = 0;
        j++;
        buffOut [j] = 1;
        j++;      
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;      
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        buffOut [j] = 1;
        j++;
        break;

      default: break;       
    }
  } 
  
  bool b[8] = {0,0,0,0,0,0,0,0};
  //Serial.print("b[]: ");  
  for (int i = 1; i < 9; ++i) { 
      int ii = i-1;
      b[ii] = buffOut[i];
      //Serial.print(b[ii]); Serial.print(" ");
  }
  //Serial.println("-");
  
  int num = 0;
  for (int i = 0; i < 8; ++i) {
    switch (i){
      case 0: 
        num = num +1 * (int)b[i];
        break;
      case 1: 
        num = num +2 * (int)b[i];
        break;
      case 2: 
        num = num +4 * (int)b[i];
        break;
      case 3: 
        num = num +8 * (int)b[i];
        break;
      case 4: 
        num = num +16 * (int)b[i];
        break;
      case 5: 
        num = num +32 * (int)b[i];
        break;
      case 6: 
        num = num +64 * (int)b[i];
        break;
      case 7: 
        num = num +128 * (int)b[i];
        break;
    }
    
  }
  //Serial.println(num);

  byte salida = (byte)num;
 //  Serial.print("HEX: ");Serial.print(salida,HEX);Serial.print(" ");
  //Serial.print(salida,HEX);Serial.print(" ");
 // Serial.write(salida);Serial.println(" ");
  if (tipoMedidor==0 || tipoMedidor==252 || tipoMedidor==253) obtenerParametros1052(salida);
  else if (tipoMedidor==1 || tipoMedidor==254 || tipoMedidor==255) obtenerParametros150(salida);
  else if (tipoMedidor==2 || tipoMedidor==250 || tipoMedidor==251) obtenerParametros102(salida);
  else if (tipoMedidor==3 || tipoMedidor==248 || tipoMedidor==249) obtenerParametros103(salida);      
  num = 0;
  return salida;
}

void obtenerParametros150(byte d){   //Esta funcion detecta la trama del A150 y obtiene los parametros de la misma
  switch (d) {
    case 0x55: d=0x00; break;
    case 0x57: d=0x01; break;
    case 0x5D: d=0x02; break;
    case 0x5F: d=0x03; break;
    case 0x75: d=0x04; break;
    case 0x77: d=0x05; break;
    case 0x7D: d=0x06; break;
    case 0x7F: d=0x07; break;
    case 0xD5: d=0x08; break;
    case 0xD7: d=0x09; break;
    case 0xDD: d=0x0A; break;
    case 0xDF: d=0x0B; break;
    case 0xF5: d=0x0C; break;
    case 0xF7: d=0x0D; break;
    case 0xFD: d=0x0E; break;
    case 0xFF: d=0x0F; break;
  }

  if (!par) { //si es impar (par=0), es el primer nibble que llega
    lo = d;
    //Serial.print("lo: "); Serial.print(lo,BIN);
    par = 1;
  }
  else {    //si es par (par=1), es el segundo nibble que llega
    hi = d << 4;
    //Serial.print("hi: "); Serial.print(hi,BIN); Serial.print(" ");
    par = 0;
    crct = hi | lo;
    trama[idt] = crct;
    //Serial.print(idt); Serial.print(" ");
    //Serial.write(crct); Serial.print(" ");
    //Serial.print(crct,HEX); Serial.print(" ");
    idt++;
  }
  if (idt >= 92 && trama[idt-1]==0x03){
    trama150ok=detectaTipoMedididor();
    nodo.irda_OK=trama150ok;
    volvioIrda();
    if (dbirda) { 
      Serial.print("Trama A150: "); Serial.println(trama150ok);
      if (nodo.modo_wifi) { telnet.print("Trama A150: "); telnet.println(String(trama150ok)+"\r"); } 
    }
    idt=0;
  }
  else trama150ok=0; 
  
  if (trama150ok==1 && printed == 0 && (tipoMedidor==1 || tipoMedidor==254 || tipoMedidor==255)) {
    strData.clear();
    for (int i = 5; i < 17; ++i) { strData.Product_Code[i - 5] = trama[i - 1]; } //12 bytes
    strData.MedidorTipo[0]='e'; strData.MedidorTipo[1]='3'; //MedidorTipo: "e3"
    // for (int i = 17; i < 26; ++i) { strData.Free_text[i - 17] = trama[i - 1]; } //9 bytes  
    // for (int i = 28; i < 32; ++i) {strData.FT[i - 28] = trama[i - 1];}//4 bytes
    for (int i = 32; i < 37; ++i) { strData.KWh[i - 32] = trama[i - 1]; }//5 bytes
    lecturaCero();
    calculadorDmd();
    for (int i = 37; i < 42; ++i) { strData.KVArh[i - 37] = trama[i - 1]; }//5 bytes
    // strData.STATUS[0] = trama[41];
    for (int i = 43; i < 47; ++i) { strData.Tiempo_Servicio[i - 43] = trama[i - 1]; }//4 bytes
    // for (int i = 47; i < 52; ++i) { strData.KVA[i - 47] = trama[i - 1]; }//5 bytes
    // for (int i = 52; i < 57; ++i) { strData.KVAr[i - 52] = trama[i - 1]; }//5 bytes
    // strData.Cuadrante[0] = trama[56];
    // for (int i = 58; i < 60; ++i) { strData.PF[i - 58] = trama[i - 1]; }//2 bytes
    // strData.Meter_type[0] = trama[59];
    // strData.React_config[0] = trama[60];
    for (int i = 62; i < 64; ++i) { strData.IntensA[i - 62] = trama[i - 1]; }//2 bytes
    acumuladorIntensProm();
    // for (int i = 64; i < 66; ++i) { strData.Current_B[i - 64] = trama[i - 1]; }//2 bytes
    for (int i = 66; i < 68; ++i) { strData.TensionA[i - 66] = trama[i - 1]; }//2 bytes
    for (int i = 68; i < 71; ++i) { strData.Creep_Time[i - 68] = trama[i - 1]; }//3 bytes
    // strData.Dmd_Reset_Count[0] = trama[70];
    // strData.Reverse_Count[0] = trama[71];
    // strData.Meter_Reset_Count[0] = trama[72];
    for (int i = 0; i < 7; ++i) { strData.MedidorID[i]='0'; }
    for (int i = 74; i < 83; ++i) { strData.MedidorID[i - 67] = trama[i - 1]; }//9 bytes
    // for (int i = 83; i < 86; ++i) { strData.Dmd_Prev[i - 83] = trama[i - 1]; }//3 byte
    // for (int i = 86; i < 89; ++i) { strData.Dmd_Acum[i - 86] = trama[i - 1]; }//3 byte
    // for (int i = 89; i < 92; ++i) { strData.Dmd_Max[i - 89] = trama[i - 1]; }//3 byte
    if (dbprm) printData(); 
    //detectaTipoMedididor();
  }
}

void obtenerParametros1052(byte d){           //Esta funcion detecta la trama y obtiene los parametros de la misma
  if (d==0x02){   //Detecta el comienzo de trama
    k=r=0; printed=0;
    for (int i = 0; i < 30; ++i)    {
      renglon[i]=0x00; //null
    }
    strData.clear();
    //Serial.println("\n\nLector MI1052 v1 \n");
    return;
  }
  if (d==0x0D || d==0x0A)  return;//no guardar si es un retorno de carro o salto de linea
  renglon[k]=char(d);
  //Serial.println(d,HEX);
  //Serial.print(renglon[k]);
  k++;
  if (k>=29 || d==0x29) { //se lleno el renglon o hay un salto de linea o parentesis de cierre
    k=0;
    String renglon_str=renglon;
    if (renglon_str.startsWith("96.1.1(")){    //Product code
      int l = renglon_str.indexOf('(')+1;
      int m = 0; r=0;
      while(renglon[l]!=0x29 && m<12){ //mientras no contenga un ")" y este dentro del renglon
        strData.Product_Code[m++]=renglon[l++];
      }
    }
    else if (renglon_str.startsWith("0.0.0(")){    //Serial Number
      int l = renglon_str.indexOf('(')+1;
      int m = 0; r=0;
      while(renglon[l]!=0x29 && m<16){ //mientras no contenga un ")" y este dentro del renglon
        strData.MedidorID[m++]=renglon[l++];
      }
    }
    else if (renglon_str.startsWith("1.8.0(")){    //Cumulative Import Register (kWh) KWh
      int l = renglon_str.indexOf('(')+1;
      int m = 0; r=0;
      char Aux_[10]={0};
      while(renglon[l]!=0x2A && m<10){ //mientras no contenga un "*" y este dentro del renglon
        if (renglon[l] != 0x2E) Aux_[m++]=renglon[l++]; //Registra el valor sin el punto decimal (123456.123 KWh --> 123456123 Wh)
        else l++;
      }
      unsigned long Aux_ulong = atoi(&Aux_[0]);
      DECtoBCD(Aux_ulong,strData.KWh,5);
      //Serial.printf("ContadorAct: {%ld}\n",Aux_ulong);
    }
    else if (renglon_str.startsWith("3.8.0(")){    //Positive reactive energy (Q+) total(kVARh inductiva)
      int l = renglon_str.indexOf('(')+1;
      int m = 0; r=0;
      char Aux_[10]={0};
      while(renglon[l]!=0x2A && m<10){ //mientras no contenga un "*" y este dentro del renglon
        if (renglon[l] != 0x2E) Aux_[m++]=renglon[l++]; //Registra el valor sin el punto decimal 
        else l++;
      }
      unsigned long Aux_ulong = atoi(&Aux_[0]);
      DECtoBCD(Aux_ulong,strData.KVArh,5);
      //Serial.printf("ContadorReacInd: {%ld}\n",Aux_ulong);
    }
    else if (renglon_str.startsWith("4.8.0(")){    //Negative reactive energy (Q-)(kVARh capacitiva)
      /*int l = renglon_str.indexOf('(')+1;
      int m = 0; r=0;
      char Aux_[10]={0};
      while(renglon[l]!=0x2A && m<10){ //mientras no contenga un "*" y este dentro del renglon
        if (renglon[l] != 0x2E) Aux_[m++]=renglon[l++]; //Registra el valor sin el punto decimal 
        else l++;
      }
      unsigned long Aux_ulong = atoi(&Aux_[0]);
      Serial.printf("ContadorReacCap: {%ld}\n",Aux_ulong);*/
    }
    else if (renglon_str.startsWith("96.8.0(")){    //Time of operation (Hrs)
      int l = renglon_str.indexOf('(')+1;
      int m = 0; r=0;
      char Aux_[8]={0};
      while(renglon[l]!=0x2A && m<8){ //mientras no contenga un "*" y este dentro del renglon
        if (renglon[l] != 0x2E) Aux_[m++]=renglon[l++]; //Registra el valor sin el punto decimal 
        else l++;
      }
      unsigned long T_S_en_minutos=60*atoi(&Aux_[0]);
      DECtoBCD(T_S_en_minutos,strData.Tiempo_Servicio,4);
      //Serial.printf("TiempoServicio: {%ld Hrs}\n",Aux_ulong);
    }
    else if (renglon_str.startsWith("96.56.0(")){    //Creep time (Hrs)
      int l = renglon_str.indexOf('(')+1;
      int m = 0; r=0;
      char Aux_[4]={0};
      while(renglon[l]!=0x2A && m<4){ //mientras no contenga un "*" y este dentro del renglon
        if (renglon[l] != 0x2E) Aux_[m++]=renglon[l++]; //Registra el valor sin el punto decimal 
        else l++;
      }  
      unsigned int Aux_uint = atoi(&Aux_[0]);
      DECtoBCD(Aux_uint,strData.Creep_Time,3);
      //Serial.printf("CreepTime: {%ld}\n",Aux_uint);
    }
    else if (renglon_str.startsWith("32.5.0(")){    // Average voltage [V] Phase L1 
      int l = renglon_str.indexOf('(')+1;
      int m = 0; r=0;
      char Aux_[5]={0};
      while(renglon[l]!=0x2A && renglon[l]!='V' && m<5){ //mientras no contenga "*", "V" y este dentro del renglon
        if (renglon[l] != 0x2E) Aux_[m++]=renglon[l++]; //Registra el valor sin el punto decimal 
        else l++;
      }
      unsigned int Aux_uint = atoi(&Aux_[0]);
      DECtoBCD(Aux_uint,strData.TensionA,2);
      //Serial.printf("TensionA: {%ld}\n",Aux_uint);
    }
    else if (renglon_str.startsWith("52.5.0(")){    // Average voltage [V] Phase L2 
      int l = renglon_str.indexOf('(')+1;
      int m = 0; r=0;
      char Aux_[5]={0};
      while(renglon[l]!=0x2A && renglon[l]!='V' && m<5){ //mientras no contenga "*", "V" y este dentro del renglon
        if (renglon[l] != 0x2E) Aux_[m++]=renglon[l++]; //Registra el valor sin el punto decimal 
        else l++;
      } 
      unsigned int Aux_uint = atoi(&Aux_[0]);
      DECtoBCD(Aux_uint,strData.TensionB,2);
      //Serial.printf("TensionB: {%ld}\n",Aux_uint);
    }
    else if (renglon_str.startsWith("72.5.0(")){    // Average voltage [V] Phase L3 
      int l = renglon_str.indexOf('(')+1;
      int m = 0; r=0;
      char Aux_[5]={0};
      while(renglon[l]!=0x2A && renglon[l]!='V' && m<5){ //mientras no contenga "*", "V" y este dentro del renglon
        if (renglon[l] != 0x2E) Aux_[m++]=renglon[l++]; //Registra el valor sin el punto decimal 
        else l++;
      } 
      unsigned int Aux_uint = atoi(&Aux_[0]);
      DECtoBCD(Aux_uint,strData.TensionC,2);
      //Serial.printf("TensionC: {%ld}\n",Aux_uint);
    }
    else if (renglon_str.startsWith("31.5.0(")){    // Average current [A] Phase L1 
      int l = renglon_str.indexOf('(')+1;
      int m = 0; r=0;
      char Aux_[5]={0};
      while(renglon[l]!=0x2A && renglon[l]!='A' && m<5){ //mientras no contenga "*", "A" y este dentro del renglon
        if (renglon[l] != 0x2E) Aux_[m++]=renglon[l++]; //Registra el valor sin el punto decimal 
        else l++;
      } 
      unsigned int Aux_uint = atoi(&Aux_[0]);
      DECtoBCD(Aux_uint,strData.IntensA,2);
      //Serial.printf("IntensA: {%ld}\n",Aux_uint);
    }
    else if (renglon_str.startsWith("51.5.0(")){    // Average current [A] Phase L2 
      int l = renglon_str.indexOf('(')+1;
      int m = 0; r=0;
      char Aux_[5]={0};
      while(renglon[l]!=0x2A && renglon[l]!='A' && m<5){ //mientras no contenga "*", "A" y este dentro del renglon
        if (renglon[l] != 0x2E) Aux_[m++]=renglon[l++]; //Registra el valor sin el punto decimal 
        else l++;
      }
      unsigned int Aux_uint = atoi(&Aux_[0]);
      DECtoBCD(Aux_uint,strData.IntensB,2);
      //Serial.printf("IntensB: {%ld}\n",Aux_uint);
    }
    else if (renglon_str.startsWith("71.5.0(")){    // Average current [A] Phase L3 
      int l = renglon_str.indexOf('(')+1;
      int m = 0; r=0;
      char Aux_[5]={0};
      while(renglon[l]!=0x2A && renglon[l]!='A' && m<5){ //mientras no contenga "*", "A" y este dentro del renglon
        if (renglon[l] != 0x2E) Aux_[m++]=renglon[l++]; //Registra el valor sin el punto decimal 
        else l++;
      }
      unsigned int Aux_uint = atoi(&Aux_[0]);
      DECtoBCD(Aux_uint,strData.IntensC,2);
      //Serial.printf("IntensC: {%ld}\n",Aux_uint);
    }
    else r++;
    //Serial.print("renglon: ");Serial.print(renglon);
    //Serial.print("r= ");Serial.println(r);
    for (int i = 0; i < 30; ++i)    {
      renglon[i]=0x00; //null
    }
  }
  //if (r>=53) {detectaTipoMedididor(); r=0;} //Si en una trama (53 renglones) no encuentra ningun parametro el tipo de medidor está mal
  if (d==0x03 || r>=53){    //Si llega el dato del final de trama 
    trama1052ok=detectaTipoMedididor();
    nodo.irda_OK=trama1052ok;
    r=0;
    if (dbirda) { 
      Serial.print("Trama A1052: "); Serial.println(trama1052ok); 
      if (nodo.modo_wifi) { telnet.print("Trama A1052: "); telnet.println(String(trama1052ok)+"\r"); } 
    }
  }
  else trama1052ok=0;
  volvioIrda();
  if (trama1052ok && (tipoMedidor==0 || tipoMedidor==252 || tipoMedidor==253)){
    lecturaCero();
    calculadorDmd();
    acumuladorIntensProm();
    if (dbprm) printData(); 
  }
}

void obtenerParametros102(byte d){   //Esta funcion detecta la trama del A102 y obtiene los parametros de la misma
  trama[idt] = d;
  // Serial.print(idt); Serial.print("- ");
  // Serial.write(d); Serial.print(", ");
   //Serial.print(d,HEX); Serial.println(" ");

  if (idt >= 108 && trama[idt]==0x03){
    idt=0;
    trama102ok=detectaTipoMedididor();
    nodo.irda_OK=trama102ok; 
    volvioIrda();
    if (dbirda) { 
      Serial.print("Trama A102: "); Serial.println(trama102ok); 
      if (nodo.modo_wifi) { telnet.print("Trama A102: "); telnet.println(String(trama102ok)+"\r"); } 
    }
    //if (trama102ok) idt=0;
  }
  else trama102ok=0;

  idt++; 
  
  if (trama102ok==1 && printed == 0 && (tipoMedidor==2 || tipoMedidor==250 || tipoMedidor==251)) {
    strData.clear();
    for (int i = 4; i < 16; ++i) { strData.Product_Code[i - 4] = trama[i]; } //12 bytes
    strData.MedidorTipo[0]='e'; strData.MedidorTipo[1]='1'; //MedidorTipo: "e1"
    //for (int i = 16; i < 25; ++i) { strData.Free_text[i - 16] = trama[i]; } //9 bytes  
    for (int i = 49; i < 54; ++i) { strData.KWh[i - 49] = trama[i]; }//5 bytes
    lecturaCero();
    calculadorDmd();
    for (int i = 54; i < 59; ++i) { strData.KVArh[i - 54] = trama[i]; }//5 bytes
    int j;
    if (trama[108]==3) j=0;
    else if (trama[109]==3) j=1;
    //else if (trama[108]==0x43) j=1;
    char T_S_en_horas[3];
    for (int i = 85+j; i < 88+j; ++i) { T_S_en_horas[i - 85-j] = trama[i]; } //3 bytes
    unsigned long T_S_en_minutos=60*BCDtoDEC(T_S_en_horas,3);
    DECtoBCD(T_S_en_minutos,strData.Tiempo_Servicio,4);
    for (int i = 82+j; i < 85+j; ++i) { strData.Creep_Time[i - 82-j] = trama[i]; }//3 bytes
    for (int i = 0; i < 8; ++i) { strData.MedidorID[i]='0'; }
    for (int i = 38; i < 46; ++i) { strData.MedidorID[i - 30] = trama[i]; }//9 bytes   
    if (dbprm) printData(); 
  }
}
void obtenerParametros103(byte d){   //Esta funcion detecta la trama del A103 y obtiene los parametros de la misma
  trama[idt] = d;
  // Serial.print(idt); Serial.print("- ");
  //Serial.write(d); Serial.print(", ");
  // Serial.print(d,HEX); Serial.println(", ");
  
  if (idt >= 174 && trama[idt]==0x03){
    idt=0;
    trama103ok=detectaTipoMedididor();
    nodo.irda_OK=trama103ok;
    volvioIrda();
    if (dbirda) { 
      Serial.print("Trama A103: "); Serial.println(trama103ok); 
      if (nodo.modo_wifi) { telnet.print("Trama A103: "); telnet.println(String(trama103ok)+"\r"); } 
    }
    //if (trama103ok) idt=0;
  }
  else trama103ok=0;

  idt++; 
  
  if (trama103ok==1 && printed == 0 && (tipoMedidor==3 || tipoMedidor==248 || tipoMedidor==249)) {
    strData.clear();
    for (int i = 4; i < 16; ++i) { strData.Product_Code[i - 4] = trama[i]; } //12 bytes
    strData.MedidorTipo[0]='e'; strData.MedidorTipo[1]='2'; //MedidorTipo: "e2"
    // for (int i = 16; i < 25; ++i) { strData.Free_text[i - 16] = trama[i]; } //9 bytes  
    for (int i = 49; i < 54; ++i) { strData.KWh[i - 49] = trama[i]; }//5 bytes
    lecturaCero();
    calculadorDmd();
    for (int i = 54; i < 59; ++i) { strData.KVArh[i - 54] = trama[i]; }//5 bytes
    int j, jj;
    if (trama[174]==3 && trama[79]==0xE4) {j=0; jj=0;}
    else if (trama[175]==3 && trama[80]==1) {j=0; jj=1;}
    else if (trama[175]==3 && trama[80]==0xE4) {j=0; jj=1;}
    else if (trama[176]==3) {j=1; jj=0;}
    //for (int i = 85+jj; i < 88+jj; ++i) { strData.Tiempo_Servicio[i - 84-jj] = trama[i]; }//4 bytes
    char T_S_en_horas[3];
    for (int i = 85+jj; i < 88+jj; ++i) { T_S_en_horas[i - 85-jj] = trama[i]; } //3 bytes
    unsigned long T_S_en_minutos=60*BCDtoDEC(T_S_en_horas,3);
    DECtoBCD(T_S_en_minutos,strData.Tiempo_Servicio,4);
    for (int i = 82+jj; i < 85+jj; ++i) { strData.Creep_Time[i - 82-jj] = trama[i]; }//3 bytes
    for (int i = 0; i < 8; ++i) { strData.MedidorID[i]='0'; }
    for (int i = 38; i < 46; ++i) { strData.MedidorID[i - 30] = trama[i]; }//9 bytes
    if (dbprm) printData();  
  }
}

bool detectaTipoMedididor(){
  String Product_Code_str="";
  bool sontodosceros=0;
  //Serial.print("tipoMedidor: "); Serial.println(tipoMedidor);
  if (tipoMedidor==0 || tipoMedidor==252 || tipoMedidor==253){ //A1052
    for (int i = 0; i < 12; ++i) {Product_Code_str = Product_Code_str + strData.Product_Code[i];}
    // Serial.print("Product_Code_str: ");Serial.println(Product_Code_str);
    if (Product_Code_str.indexOf("Elster A1052") != -1 && r<53) {//Si encuentra "A1052" en el product code
      tipoMedidor=0;
      strData.MedidorTipo[0]='f'; strData.MedidorTipo[1]='1'; //MedidorTipo: "f1"
      if (dbirda) { 
        Serial.println("A1052 detectado"); 
        if (nodo.modo_wifi) telnet.println("A1052 detectado\r");
      }
      pausas=0;
      return 1;  
      // Serial.println("tipoMedidor: 0 (A1052)");
    }
    else if (Product_Code_str.indexOf("A1052 Plus") != -1 && r<53) {//Si encuentra "A1052" en el product code
      tipoMedidor=0;
      strData.MedidorTipo[0]='f'; strData.MedidorTipo[1]='2'; //MedidorTipo: "f2"
      if (dbirda) { 
        Serial.println("A1052 Plus detectado");
        if (nodo.modo_wifi) telnet.println("A1052 Plus detectado\r");
      }
      pausas=0;
      return 1;  
      // Serial.println("tipoMedidor: 0 (A1052)");
    }
    else if (tipoMedidor==0) {tipoMedidor=252; return 0;}//A1052 no detectado a la 1era
    else if (tipoMedidor==252) {tipoMedidor=253; return 0;}//A1052 no detectado a la 2da 
    else if (tipoMedidor==253) {tipoMedidor=1; return 0;}//A1052 no detectado a la 3era, cambia a A150
  }
  else if (tipoMedidor==1 || tipoMedidor==254 || tipoMedidor==255){ //A150
    //Serial.print("# Medidor: ");
    for (int i = 73; i < 82; ++i) { //Primero verifico que no haya todo 0's en el serial number
      //Serial.write(trama[i]);
      if (trama[i]==0) {
        sontodosceros=1;
      }
      else {sontodosceros=0; break;}
    }
    //Serial.println("");
    if (sontodosceros) {
      //Serial.println("SON TODOS NULL");
      if (tipoMedidor==1) {tipoMedidor=254;} //A150 no detectado a la 1era
      else if (tipoMedidor==254) {tipoMedidor=255;}//A150 no detectado a la 2da
      else if (tipoMedidor==255) {tipoMedidor=2;}//A150 no detectado a la 3era, cambia a A102
      return 0;
    }
    else if (trama[0]==0x01 && trama[1]==0x00 && trama[2]==0x57 && trama[3]==0x02 && trama[91]==0x03) {
      for (int i = 5; i < 17; ++i) { Product_Code_str = Product_Code_str + char(trama[i - 1]); } //12 bytes
      // Serial.print("Product_Code_str: ");Serial.println(Product_Code_str);
      if (Product_Code_str.indexOf("A150") != -1) {
        tipoMedidor=1;  //Si encuentra "A150" en el product code
        if (dbirda) { 
          Serial.println("A150 detectado");
          if (nodo.modo_wifi) telnet.println("A150 detectado\r");
        }
        pausas=0;
        //trama150ok = 1;
        //Serial.println("\n\nLector MI150 v1 \n");
        return 1;
        // Serial.println("tipoMedidor: 1 (A150)");
      }
      else return 0;
    }
    else {
      //trama150ok=0;
      if (tipoMedidor==1) {tipoMedidor=254;} //A150 no detectado a la 1era
      else if (tipoMedidor==254) {tipoMedidor=255;}//A150 no detectado a la 2da
      else if (tipoMedidor==255) {tipoMedidor=2;}//A150 no detectado a la 3era, cambia a A102
      return 0;
    }
  }
  else if ((tipoMedidor==2 || tipoMedidor==250 || tipoMedidor==251) /*&& encontrado==0*/){ //A102
    if (trama[0]==0x01 && trama[1]==0x00 && trama[2]==0x68 && trama[3]==0x02 && (trama[109]==0x03 || trama[108]==0x03)) {
    //if (trama[0]==0x01 && trama[1]==0x00 && trama[2]==0x68 && trama[3]==0x02) {
      for (int i = 5; i < 17; ++i) { Product_Code_str = Product_Code_str + char(trama[i - 1]); } //12 bytes
      // Serial.print("Product_Code_str: ");Serial.println(Product_Code_str);
      if (Product_Code_str.indexOf("A102") != -1) { //Si encuentra "A102" en el product code
        tipoMedidor=2;
        encontrado=1;  
        Serial.print("encontrado: ");Serial.println(encontrado);
        if (dbirda) { 
          Serial.println("A102 detectado");
          if (nodo.modo_wifi) telnet.println("A102 detectado\r"); 
        }
        pausas=0;
        return 1;
      }
      else return 0;
    }
    else {
      if (tipoMedidor==2 /*&& encontrado==0*/) {tipoMedidor=250;} //A102 no detectado a la 1era
      else if (tipoMedidor==250 /*&& encontrado==0*/) {tipoMedidor=251;}//A102 no detectado a la 2da
      else if (tipoMedidor==251 /*&& encontrado==0*/) {tipoMedidor=3;}//A102 no detectado a la 3era, cambia a A103
      return 0;
    }
  }
  else if (tipoMedidor==3 || tipoMedidor==248 || tipoMedidor==249){ //A103
    if (trama[0]==0x01 && trama[1]==0x00 && trama[2]==0xAA && trama[3]==0x02 && (trama[174]==0x03 || trama[175]==0x03 || trama[176]==0x03) ) {
      for (int i = 5; i < 17; ++i) { Product_Code_str = Product_Code_str + char(trama[i - 1]); } //12 bytes
      // Serial.print("Product_Code_str: ");Serial.println(Product_Code_str);
      if (Product_Code_str.indexOf("A103") != -1) {
        tipoMedidor=3;  //Si encuentra "A103" en el product code
        if (dbirda) { 
          Serial.println("A103 detectado");
          if (nodo.modo_wifi) telnet.println("A103 detectado\r"); 
        }
        pausas=0;
        return 1;
      }
      else return 0;
    }
    else {
      if (tipoMedidor==3) {tipoMedidor=248;} //A102 no detectado a la 1era
      else if (tipoMedidor==248) {tipoMedidor=249;}//A103 no detectado a la 2da
      else if (tipoMedidor==249) {tipoMedidor=0;}//A103 no detectado a la 3era, cambia a A1052
      return 0;
    }
  }  
}

void printData(){
  Serial.print("Product_Code: "); for (int i = 0; i < 12; ++i){ Serial.write(strData.Product_Code[i]); } Serial.println(" "); //12 bytes
  if (nodo.modo_wifi) { telnet.print("Product_Code: "); telnet.print(strData.Product_Code); telnet.println("\r"); }
  Serial.print("MedidorTipo: "); for (int i = 0; i < 2; ++i){Serial.print(strData.MedidorTipo[i]);} Serial.println(" ");
  if (nodo.modo_wifi) { telnet.print("MedidorTipo: "); telnet.print(strData.MedidorTipo); telnet.println("\r"); }   
  Serial.print("KWh: "); for (int i = 0; i < 5; ++i) { printHex(strData.KWh[i]); } Serial.println(" ");//5 bytes
  if (nodo.modo_wifi) { telnet.print("KWh: "); for (int i = 0; i < 5; ++i) { telnet.print(String(strData.KWh[i],HEX)); } telnet.println("\r"); }
  Serial.print("KVArh: "); for (int i = 0; i < 5; ++i) { printHex(strData.KVArh[i]); } Serial.println(" ");//5 bytes
  if (nodo.modo_wifi) { telnet.print("KVArh: "); for (int i = 0; i < 5; ++i) { telnet.print(String(strData.KVArh[i],HEX)); } telnet.println("\r"); }
  Serial.print("Tiempo_Servicio: "); for (int i = 0; i < 4; ++i) { printHex(strData.Tiempo_Servicio[i]); } Serial.println(" ");//4 bytes
  if (nodo.modo_wifi) { telnet.print("Tiempo_Servicio: "); for (int i = 0; i < 4; ++i) { telnet.print(String(strData.Tiempo_Servicio[i],HEX)); } telnet.println("\r"); }
  if (tipoMedidor==0 || tipoMedidor==1 || tipoMedidor==254 || tipoMedidor==255)  { //solo para A1052 y A150
    Serial.print("IntensA: "); for (int i = 0; i < 2; ++i) { printHex(strData.IntensA[i]); } Serial.println(" ");//2 bytes
    if (nodo.modo_wifi) { telnet.print("IntensA: "); for (int i = 0; i < 2; ++i) { telnet.print(String(strData.IntensA[i],HEX)); } telnet.println("\r"); }
    Serial.print("IntensB: "); for (int i = 0; i < 2; ++i) { printHex(strData.IntensB[i]); } Serial.println(" ");//2 bytes
    if (nodo.modo_wifi) { telnet.print("IntensB: "); for (int i = 0; i < 2; ++i) { telnet.print(String(strData.IntensB[i],HEX)); } telnet.println("\r"); }
    Serial.print("IntensC: "); for (int i = 0; i < 2; ++i) { printHex(strData.IntensC[i]); } Serial.println(" ");//2 bytes
    if (nodo.modo_wifi) { telnet.print("IntensC: "); for (int i = 0; i < 2; ++i) { telnet.print(String(strData.IntensC[i],HEX)); } telnet.println("\r"); }
    Serial.print("TensionA: "); for (int i = 0; i < 2; ++i) { printHex(strData.TensionA[i]); } Serial.println(" ");//2 bytes
    if (nodo.modo_wifi) { telnet.print("TensionA: "); for (int i = 0; i < 2; ++i) { telnet.print(String(strData.TensionA[i],HEX)); } telnet.println("\r"); }
    Serial.print("TensionB: "); for (int i = 0; i < 2; ++i) { printHex(strData.TensionB[i]); } Serial.println(" ");//2 bytes
    if (nodo.modo_wifi) { telnet.print("TensionB: "); for (int i = 0; i < 2; ++i) { telnet.print(String(strData.TensionB[i],HEX)); } telnet.println("\r"); }
    Serial.print("TensionC: "); for (int i = 0; i < 2; ++i) { printHex(strData.TensionC[i]); } Serial.println(" ");//2 bytes
    if (nodo.modo_wifi) { telnet.print("TensionC: "); for (int i = 0; i < 2; ++i) { telnet.print(String(strData.TensionC[i],HEX)); } telnet.println("\r"); }
  }
  Serial.print("Creep_Time: "); for (int i = 0; i < 3; ++i) {printHex(strData.Creep_Time[i]); } Serial.println(" ");//3 bytes
  
  Serial.print("MedidorID: "); for (int i = 0; i < 16; ++i) { 
    Serial.write(strData.MedidorID[i]);}     Serial.println(" ");//9 bytes
  if (nodo.modo_wifi) { telnet.print("MedidorID: "); telnet.print(strData.MedidorID); telnet.println("\r"); }
  if (tipoMedidor==0 || tipoMedidor==1 || tipoMedidor==254 || tipoMedidor==255)  { //solo para A1052 y A150
    int ImedA=strData.IAacum/strData.nI;
    int ImedB=strData.IBacum/strData.nI;
    int ImedC=strData.ICacum/strData.nI;
    Serial.print("Imed A: "); Serial.println(ImedA);
    if (nodo.modo_wifi) { telnet.print("Imed A: "); telnet.println(String(ImedA)+"\r"); }
    Serial.print("Imed B: "); Serial.println(ImedB);
    if (nodo.modo_wifi) { telnet.print("Imed B: "); telnet.println(String(ImedB)+"\r"); }
    Serial.print("Imed C: "); Serial.println(ImedC);
    if (nodo.modo_wifi) { telnet.print("Imed C: "); telnet.println(String(ImedC)+"\r"); }
  }
  Serial.print("Demanda: "); Serial.println(strData.Dmd_4*4);
  if (nodo.modo_wifi) { telnet.print("Demanda: "); telnet.println(String(strData.Dmd_4*4)+"\r"); }
  Serial.print("Demanda max: "); Serial.println(strData.Dmd_max);
  if (nodo.modo_wifi) { telnet.print("Demanda max: "); telnet.println(String(strData.Dmd_max)+"\r"); }
  printed=1; 
}

void printHex(byte d) {
  if (d<10)  {
    Serial.print("0");
    Serial.print(d,HEX);
  }
  else Serial.print(d,HEX);
}

void volvioIrda() {
  if (nodo.irda_OK && falso_tamper_disparado) {
    falso_tamper_disparado=0;
    Serial.println("-->VOLVIO LA SEÑAL IRDA");
    if (nodo.modo_wifi) telnet.println("-->VOLVIO LA SEÑAL IRDA\r");
    nodo.falso_tamper_des=1;
    unsigned long millis_desde_lectura0= millis() - millis_lectura0;
    if (millis_desde_lectura0 >= 930000) {
      //Si han pasado mas de 15.5min desde la ultima lectura de demanda..
      Serial.println("Tiempo para demanda excedido. Se reiniciaron los contadores de demanda");
      lectura0=1;
    }
  }
}

void lecturaCero() {
  if (lectura0) {
    strData.KWh_0= BCDtoDEC(strData.KWh,5);
    millis_lectura0= millis();
    strData.Dmd_4=0;
    strData.Dmd_max=0;
    strData.IAacum=strData.IBacum=strData.ICacum=0;
    strData.nI=0;
    lectura0=0;
  }
}

void calculadorDmd() {
  if (calculardmd) {  
    unsigned long millis_desde_lectura0= millis() - millis_lectura0;
    if (millis_desde_lectura0 <= 930000) {
      //Si no han pasado mas de 15.5min desde la ultima lectura..
      Serial.print("Calculando demanda. ");
      millis_lectura0=millis();

      unsigned long KWh_=BCDtoDEC(strData.KWh,5);
      strData.Dmd_4=KWh_ - strData.KWh_0;
      strData.KWh_0=KWh_;
      Serial.print("KW_ dmd: ");Serial.println(KWh_);
    }
    else {
      //Si han pasado mas de 15.5min desde la ultima lectura..
      Serial.println("Tiempo para demanda excedido");
      strData.Dmd_4=0;
    }
    calculardmd=0;
    muestreadodmd=1;
    unsigned int Dmd_=strData.Dmd_4*4;
    if (Dmd_>strData.Dmd_max) {
      strData.Dmd_max=Dmd_;
    } 
  }
}

void acumuladorIntensProm() { 
  strData.IAacum=strData.IAacum+BCDtoDEC(strData.IntensA,2);
  strData.IBacum=strData.IAacum+BCDtoDEC(strData.IntensB,2);
  strData.ICacum=strData.IAacum+BCDtoDEC(strData.IntensC,2);
  strData.nI++;
}
