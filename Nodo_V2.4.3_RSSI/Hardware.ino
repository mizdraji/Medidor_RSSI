//REINTENTOS
#define MAX_REINTENTOS 6    //es la cantidad  maxima de reintentos que hace con un intervalo de tiempo pequeño, luego espera un tiempo mas largo y vuelve a intentar. Min 6 para que use todos los SF
#define MAX_REINTENTO_SAME_PACKAGE 3 //cantidad maxima de reintentos cortos en el envio de paquetes de contadores
#define MAX_REINTENTO_ACT 10    //Maximo de pedidos de activacion automatica
#define MAX_REINTENTO_NULO 10   //Maximo de paquetes nulos (paquetes de cambio de canal por defecto)
#define MAX_INTENTOS_UPLINK 10  //Maximo de reintentos por periodo de transmision
#define MAX_PAUSAS_LARGAS 3 //Maximo de pausas largas que se realizan cuando los reintentos cortos fallan, superado este numero se espera un dia completo

uint8_t change_payload_once = 0;
bool muestreado=0;
bool muestreadodmd=0;
bool inhibir_tamper=0;
bool falso_tamper_disparado=0;
bool mac_correcta=0; //se levanta cuando la mac recibida en el pedido de activacion es la misma que la del dispositivo
bool trans_time=0; // Bandera para que entre a transmitir los paquetes de contadores
bool liberar_bloqueo=1; //bandera para liberar el bloqueo para que entre una sola vez a transmitir los contadores


// INTERVALOS (en segundos)
#define delta_t 1 //1 segundo
#define INTERVAL1 60 //60 seg = 1 min
#define INTERVAL8 28800 //28800 seg = 8 hs
#define INTERVAL05 300 //5 min, se usa para verificar canal por defecto
#define LONG_TIME_TO_WAIT 600 //son 10 minutos, en s
#define UN_DIA 86400 //24hs en s
#define ESPERA_FALLO_AA 7200 //7200s: 2h. Tiempo de espera para reiniciar por fallo en el proceso de activacion auto
int INTERVAL_RANDOM=120; //2 minutos, contador para reintento de envio de paquete contadores

#define falla_interval 86400   //intervalo para verificar si el nodo falla, 1 vez por dia = 86400
int32_t cont_falla = 0;        //contador falla interval

//CONTADORES para temporizar todas las funciones
int32_t cont_1s = 0;
int32_t cont_cont = 0; //Usado para el reintento de envio de paquetes de contadores
int32_t cont_stdby = 0; //usado para los paquetes en modo stdby
int cont_1min=0; //Usado para guardar CMt en EEPROM
int cont_dc=0;  //Usado para poner el canal por defecto
int cont_1m_t=0; //Usado para el falso tamper

void config_pines( void )
{
  pinMode(pin_led, OUTPUT);
  pinMode(pin_led_2, OUTPUT);
  digitalWrite(pin_led, LOW);
  digitalWrite(pin_led_2, LOW);
  //pinMode(pin_tamper, INPUT_PULLDOWN_16);
}

void RTC( void ) {
  if (nodo.is_sync) {//solo si se sincronizó comienza incrementar el tiempo real
    CMt=CMt+delta_t;
    master_time = master_time + delta_t;
    if (master_time >= 86400) {
      master_time = master_time - 86400;
    }
    if ((master_time%28800)>=(OFFSET_FIJO+offset_id)) trans_time=1; // Bandera para que entre a transmitir los paquetes de contadores
    else trans_time=0;
    if ((master_time%28800)<=10) liberar_bloqueo=1; //bandera para liberar el bloqueo para que entre una sola vez a transmitir los contadores
    
    //GUARDAR CMT EN EEPROM
    cont_1min++;
    if ( cont_1min >= INTERVAL1) {
      cont_1min=0;
      get_timestamp(CMt);
      Serial.printf("Guardando CMt: %d en dir: %d \n",CMt,dir_CMt_actual);
      dir_CMt_actual=writeCMtToEEPROM(dir_CMt_actual,CMt_array,4);
    }
    uint8_t hora = abs(master_time / 3600);
    uint8_t minutos = abs((master_time % 3600) / 60);
    uint8_t segundos = (master_time % 3600) % 60;
    ciclo=asignaCiclo(master_time);

    if (dbtsk>0) {
      printHora(hora,minutos,segundos);
    }

    if (nodo.is_identify) {//solo si se identifico comienza a muestrear
      if (minutos==0 || minutos==15 || minutos==30 || minutos==45) {
        if (muestreadodmd==0) {
          calculardmd=1; //Levanto esta bandera para que macroirda calcule la demanda
        }
      }
      else if (muestreadodmd==1) muestreadodmd=0;
      //Es la hora de muestrear? miramos el minutero... cuando vale 0, es la hora
      //Descomentar para hacer debug a las 17:59:50 //if ( minutos == 0 || (master_time>=61190 && master_time<61199) || (master_time>=28740 && master_time<28799) || (master_time>=57540 && master_time<57599) || (master_time>=86340 && master_time<86399)) {
      if ( (minutos == 0 && segundos>10) || (master_time>=28740 && master_time<28799) || (master_time>=57540 && master_time<57599) || (master_time>=86340 && master_time<86399)) {        
        if (muestreado==0) {
          if (dbtsk) {
            Serial.println("-->Muestreando");
            if (nodo.modo_wifi) telnet.println("-->Muestreando\r");
          }
          muestreado=muestrearMedidor(2); //Muestrea y carga paquete de contadores
          Serial.printf("Muestreado= %d",muestreado);
        }
      }
      else if (muestreado==1) {
        muestreado=0;
      }
    }
  }
}

//TASK PERIODICO DE ENVIO DE PAQUETES DE CONTADORES
void contTask( void ) {
  cont_cont++;

  //ENVIO DE PAQUETES PENDIENTES LA PRIMERA VEZ
  if (!nodo.stdby && first_time && nodo.is_identify) {
    bool sisepudo=enviarTodo();
    Serial.printf("Primera vez. enviarTodo: %d \n",sisepudo);
    if (!sisepudo) {
      INTERVAL_RANDOM=random_time(MIN_RANDOM2,MAX_RANDOM2);
      if (dbtsk)   {
        Serial.print("-->ENVIO FALLIDO. Reintento en: "); Serial.print(INTERVAL_RANDOM); Serial.println(" segundos ");
        if (nodo.modo_wifi) {
          char buffers[39];
          sprintf(buffers,"-->ENVIO FALLIDO. Reintento en: %d segundos \r",INTERVAL_RANDOM);
          telnet.println(buffers);telnet.print("\r");
        }
      }
    }
    first_time=0;    
  }

  //ENVIO DEL PAQUETE CADA 8 HS
  if (!nodo.stdby && trans_time && liberar_bloqueo && nodo.is_identify) {
    liberar_bloqueo=0;
    nodo.cont_intentos_uplink=0;
    nodo.bloqueo_reintentos=0;
    bool sisepudo=enviarTodo();
    Serial.printf("enviarTodo: %d \n",sisepudo);
  }

  //REINTENTO DE ENVIO DE PAQUETE de CONTADORES
  if (cont_cont >= INTERVAL_RANDOM && nodo.bloqueo_reintentos==0) {
    cont_cont = 0;
    if (!nodo.stdby && nodo.is_identify) {
      int dir_para_enviar=dirParaEnviar();
      if (dir_para_enviar>=0) nodo.hay_paquetes_pendientes=1;
      else nodo.hay_paquetes_pendientes=0;
      bool bloqueo_uplink;
      if (nodo.cont_intentos_uplink < MAX_INTENTOS_UPLINK) bloqueo_uplink=0;
      else {
        bloqueo_uplink=1;
        nodo.is_deaf=1;
      }

      //vemos si hay paquete pendiente y el contador es menor que la cant de reintentos del mismo paquete
      if (nodo.hay_paquetes_pendientes==1 && nodo.cont_reintentos_paquete < MAX_REINTENTO_SAME_PACKAGE && bloqueo_uplink==0) {
        if (dbtsk) {
          Serial.println("-->Paquetes pendientes: reenviando");
          if (nodo.modo_wifi) telnet.println("-->Paquetes pendientes: reenviando\r");
        }
        if (enviarTodo()) {
          nodo.hay_paquetes_pendientes = 0;
          if (dbtsk)   {
            Serial.println("-->Reintento ok. Se enviaron todos los paquetes");
            if (nodo.modo_wifi) telnet.println("-->Reintento ok. Se enviaron todos los paquetes\r");
          }
        }
        else {
          nodo.cont_reintentos_paquete++;
          nodo.hay_paquetes_pendientes = 1;
          INTERVAL_RANDOM=random_time(MIN_RANDOM2,MAX_RANDOM2);
          if (dbtsk)   {
            Serial.print("-->Reintento FALLIDO. Reintento en: "); Serial.print(INTERVAL_RANDOM); Serial.println(" segundos ");
            Serial.printf("intentos uplink: %d\n",nodo.cont_intentos_uplink);
            if (nodo.modo_wifi) {
              char buffers[39];
              sprintf(buffers,"-->Reintento FALLIDO. Reintento en: %d segundos \r",INTERVAL_RANDOM);
              telnet.println(buffers);telnet.print("\r");
            }
          }
        }
      }
      else if (nodo.cont_reintentos_paquete >= MAX_REINTENTO_SAME_PACKAGE) {
        INTERVAL_RANDOM=random(MIN_RANDOM_LARGO2, MAX_RANDOM_LARGO2);
        nodo.cont_reintentos_paquete = 0; //vuelvo a cero para la proxima
        if (dbtsk) {
          Serial.print("-->Máximo de reintentos realizados. Reintentar en: "); Serial.print(INTERVAL_RANDOM); Serial.println(" segundos ");
          if (nodo.modo_wifi) {
              char buffers[39];
              sprintf(buffers,"-->Máximo de reintentos realizados. Reintentar en: %d segundos\r",INTERVAL_RANDOM);
              telnet.println(buffers);telnet.print("\r");
            }
        }
      }
      else if (nodo.hay_paquetes_pendientes==0) {
        if (dbtsk) {
          Serial.println("-->No hay paquetes pendientes");
          if (nodo.modo_wifi) telnet.println("-->No hay paquetes pendientes\r");
        }
        nodo.hay_paquetes_pendientes = 0; //le digo que deje de intentar
      }

    }
  }
}

//Verificar que se escucha el canal de recepcion de comandos CH0
void defCHSF_function() {
  cont_dc++;
  if (cont_dc >= INTERVAL05) {
    cont_dc=0;
    unsigned char ch=lora.getChannel();
    unsigned char dr=lora.getDataRate();
    //Serial.print("Canal "); Serial.println(ch);
    //Serial.print("Data Rate "); Serial.println(dr);
    if (((ch != DEFAULT_CHANNEL) || (dr != DEFAULT_SF)) && nodo.cont_paq_nulos<MAX_REINTENTO_NULO) {
      char uncero[1]={0};
      if (!nodo.stdby) sendPackage(uncero, 1, no_espera_ACK, 1);
      nodo.cont_paq_nulos++;
      if (dbspk) {
        Serial.print("Canal y Data Rate cambiado por defecto");
        if (nodo.modo_wifi) telnet.println("Canal y Data Rate cambiado por defecto\r");
      }
    }
    else if ((ch == DEFAULT_CHANNEL)&&(dr == DEFAULT_SF)) nodo.cont_paq_nulos=0;
  }
}

//Envio de paquetes en estado Stand By
void stdby_function() { 
  if (nodo.stdby) {
    cont_stdby++;
    //if (cont_stdby%600==0) Serial.printf(">cont_stdby=%d<",cont_stdby);
    if (cont_stdby >= UN_DIA) {
      cont_stdby=0;
      char uncmd[1]={CMD_STDBY_ON};
      sendPackage(uncmd, 1, no_espera_ACK, 1);
      Serial.println("============= SEND STDBY ON CMD =============");
      if (nodo.modo_wifi) telnet.println("============= SEND STDBY ON CMD =============\r");
    }   
  }
}

//Prueba de Red
void pdr_function() {

  if (nodo.stdby == 0 && nodo.is_activated == 1 && nodo.pdr_ok == 0) {
    if (nodo.t_wait == 0) {
      char uncero[1]={0};
      if (sendPackage(uncero, 1, espera_ACK, 0)) {
        nodo.pdr_ok = 1;
        if (DEBUG) nodo.t_wait=1;
        else nodo.t_wait = random_time(0,MAX_RANDOM_LARGO2);
        nodo.pausa_larga = 0;
        nodo.cont_pausas_largas=0;
        nodo.cont_reintento_corto = 0;
        Serial.println("-->PRUEBA DE RED: OK"); Serial.printf("-->Esperando t = %d s para sincronizar. \r\n",nodo.t_wait);
        if (nodo.modo_wifi) { telnet.println("-->PRUEBA DE RED: OK\r"); telnet.print("-->Esperando t = "); telnet.print((String) nodo.t_wait); telnet.println(" s para sincronizar.\r");}
      }
      else {
        nodo.pdr_ok = 0;
        nodo.cont_reintento_corto++;
        if (nodo.cont_reintento_corto >= MAX_REINTENTOS) {
          nodo.cont_reintento_corto = 0;
          nodo.pausa_larga = 1;
          nodo.cont_pausas_largas++;
          if (nodo.cont_pausas_largas >= MAX_PAUSAS_LARGAS) {
            nodo.cont_pausas_largas = 0;
            nodo.t_wait = UN_DIA; //esperar un dia completo
          }
          else nodo.t_wait = LONG_TIME_TO_WAIT; //este es un tiempo largo y fijo
          Serial.println("-->Cant Max de reintentos para PDR excedido, pausa larga");
          if (nodo.modo_wifi) telnet.println("-->Cant Max de reintentos para PDR excedido, pausa larga\r");
          Serial.print("-->Esperando t = "); Serial.print(nodo.t_wait); Serial.println(" s para reintentar PDR...");
          if (nodo.modo_wifi) { telnet.print("-->Esperando t = "); telnet.print((String) nodo.t_wait); telnet.println(" s para reintentar PDR...\r");}
        }
        else {
          nodo.t_wait = random_time(MIN_RANDOM2,MAX_RANDOM2); //devuelve en segundos
          Serial.println("-->PRUEBA DE RED: FALLO");
          if (nodo.modo_wifi) telnet.println("-->PRUEBA DE RED: FALLO\r");
          Serial.print("-->Esperando t = "); Serial.print(nodo.t_wait); Serial.println(" s para reintentar PDR...");
          if (nodo.modo_wifi) { telnet.print("-->Esperando t = "); telnet.print((String) nodo.t_wait); telnet.println(" s para reintentar PDR...\r");}
        }
      }
    }
    else if (nodo.t_wait > 0) { //tiempo de espera 
      nodo.t_wait--; //vamos decrementando el t_wait
      
    }
  }
}

uint8_t sync_function( void ) {
  if (nodo.stdby == 0 && nodo.is_activated == 1 && nodo.pdr_ok == 1 && nodo.is_sync == 0) {
    if (nodo.t_wait == 0) {
      if (sendPackage((char*)payloadST, PAYLOADST_SIZE, espera_Dato, 0)) {
        nodo.is_sync = 1;
        nodo.t_wait = 0;
        nodo.pausa_larga = 0;
        nodo.cont_pausas_largas=0;
        nodo.cont_reintento_corto = 0;
        Serial.println("-->Nodo SINCRONIZADO");
        if (nodo.modo_wifi) telnet.println("-->Nodo SINCRONIZADO\r");
      }
      else {
        nodo.cont_reintento_corto++;
        if (nodo.cont_reintento_corto >= MAX_REINTENTOS)
        {
          nodo.cont_reintento_corto = 0;
          nodo.pausa_larga = 1;
          nodo.cont_pausas_largas++;
          if (nodo.cont_pausas_largas >= MAX_PAUSAS_LARGAS) {
            nodo.cont_pausas_largas = 0;
            nodo.t_wait = UN_DIA; //esperar un dia completo
          }
          else nodo.t_wait = LONG_TIME_TO_WAIT; //este es un tiempo largo y fijo
          Serial.println("-->Cant Max de reintentos para sincronizar excedido, pausa larga");
          if (nodo.modo_wifi) telnet.println("-->Cant Max de reintentos para sincronizar excedido, pausa larga\r");
          Serial.print("-->Esperando t = "); Serial.print(nodo.t_wait); Serial.println(" s para reintentar sync...");
          if (nodo.modo_wifi) { telnet.print("-->Esperando t = "); telnet.print((String) nodo.t_wait); telnet.println(" s para reintentar sync...\r");}
        }
        else
        {
          nodo.is_sync = 0;
          nodo.t_wait = random_time(MIN_RANDOM2,MAX_RANDOM2); //devuelve en segundos
          Serial.println("-->Nodo NO SINCRONIZADO");
          if (nodo.modo_wifi) telnet.println("-->Nodo NO SINCRONIZADO\r");
          Serial.print("-->Esperando t = "); Serial.print(nodo.t_wait); Serial.println(" s para reintentar sincronizacion...");
          if (nodo.modo_wifi) { telnet.print("-->Esperando t = "); telnet.print((String) nodo.t_wait); telnet.println(" s para reintentar sincronizacion...\r");}
        }
      }
    }
    else if (nodo.t_wait > 0) { //tiempo de espera 
      nodo.t_wait--; //vamos decrementando el t_wait
       
    }
  }
}

void identify_function( void ) {
  if ( (nodo.stdby == 0) && (nodo.is_sync == 1) && (nodo.is_identify == 0) ) {  
  //entra a enviar el id del medidor SI esta sincronizado y NO esta identificado aun y el tiempo de espera esta en cero
    if (nodo.t_wait == 0) {
      //para saber si los datos estan listos para usar, consultamos: lect_med_ok
      if (muestrearMedidor(1)) { //Muestrea y carga payloadID
        if (sendPackage((char*)payloadID, sizeof(payloadID), espera_ACK, 0)) {
          nodo.is_identify = 1;
          nodo.t_wait = 0;
          nodo.pausa_larga = 0;
          nodo.cont_pausas_largas=0;
          nodo.cont_reintento_corto = 0;
          Serial.println("-->Nodo IDENTIFICADO");
          if (nodo.modo_wifi) telnet.println("-->Nodo IDENTIFICADO\r");
          writeResetCountToEEPROM(0); //Pone a cero el contador de reinicios
        }
        else {
          nodo.cont_reintento_corto++;
          if (nodo.cont_reintento_corto >= MAX_REINTENTOS) {
            nodo.cont_reintento_corto = 0;
            nodo.pausa_larga = 1;
            nodo.cont_pausas_largas++;
            if (nodo.cont_pausas_largas >= MAX_PAUSAS_LARGAS) {
              nodo.cont_pausas_largas = 0;
              nodo.t_wait = UN_DIA; //esperar un dia completo
            }
            else nodo.t_wait = LONG_TIME_TO_WAIT; //este es un tiempo largo y fijo
            Serial.println("-->Cant Max de reintentos para identificacion excedido, pausa larga");
            if (nodo.modo_wifi) telnet.println("-->Cant Max de reintentos para identificacion excedido, pausa larga\r");
            Serial.print("-->Esperando t = "); Serial.print(nodo.t_wait); Serial.println(" s para reintentar identificacion...");
            if (nodo.modo_wifi) { telnet.print("-->Esperando t = "); telnet.print((String) nodo.t_wait); telnet.println(" s para reintentar identificacion...\r");}
          }
          else {
            nodo.is_identify = 0;
            nodo.t_wait = random_time(MIN_RANDOM2,MAX_RANDOM2);
            Serial.println("-->Nodo NO IDENTIFICADO");
            if (nodo.modo_wifi) telnet.println("-->Nodo NO IDENTIFICADO\r");
            Serial.print("-->Esperando t = "); Serial.print(nodo.t_wait); Serial.println(" s para reintentar identificacion...");
            if (nodo.modo_wifi) { telnet.print("-->Esperando t = "); telnet.print((String) nodo.t_wait); telnet.println(" s para reintentar identificacion...\r");}
          }
        }
      }
    }
    else if (nodo.t_wait > 0 ) { //tiempo de espera
      nodo.t_wait--; //vamos decrementando el t_wait
       
    }
  }
}

uint8_t falso_tamper( void ) {
  if (!inhibir_tamper) cont_1m_t++;
  //Verificar si se perdio la señal irda (falso tamper)
  if (cont_1m_t >= INTERVAL1 && nodo.irda_OK) {
    cont_1m_t=0;
    Serial.println("-->SE PERDIO LA SEÑAL IRDA");
    if (nodo.modo_wifi) telnet.println("-->SE PERDIO LA SEÑAL IRDA\r");
    nodo.irda_OK=0;
    trama150ok = 0;
    trama102ok = 0;
    trama103ok = 0;
    trama1052ok = 0;
    falso_tamper_disparado=1;
    nodo.falso_tamper_act=1;
  }
  if (!nodo.stdby && nodo.is_identify) {
    if (nodo.t_wait == 0) {
      if (nodo.falso_tamper_act == 1) {
        Serial.println("============= ENVIANDO ALARMA DE TAMPER ACTIVADO =============");
        if (nodo.modo_wifi) telnet.println("=============  ENVIANDO ALARMA DE TAMPER ACTIVADO =============\r");
        char comando[1]={CMD_TAMPER_ACT};
        if (sendPackage(comando, 1, espera_ACK, 0)) {
          //if (cmd_rcv==comando[0]) {
            nodo.falso_tamper_act = 0;
            nodo.t_wait = 0;
            nodo.cont_reintentos_alarma=0;
            Serial.println("-->Alarma de Falso Tamper Activado ENVIADA");
            if (nodo.modo_wifi) telnet.println("-->Alarma de Falso Tamper Activado ENVIADA\r");
          //}
          //else Serial.println("Comando de respuesta incorrecto");
        }
        else {
          nodo.cont_reintentos_alarma++;
          if (nodo.cont_reintentos_alarma >= MAX_REINTENTOS) {
            nodo.cont_reintentos_alarma = 0;
            nodo.falso_tamper_act=0;
            nodo.t_wait = 0;
            Serial.println("-->Cant Max de reintentos para Alarma excedido, descartar");
            if (nodo.modo_wifi) telnet.println("-->Cant Max de reintentos para Alarma excedido, descartar\r");
          }
          else {
            nodo.falso_tamper_act = 1;
            nodo.t_wait = random_time(MIN_RANDOM2,MAX_RANDOM2); //devuelve en segundos
            Serial.println("-->Alarma de Falso Tamper Activado NO ENVIADA");
            if (nodo.modo_wifi) telnet.println("-->Alarma de Falso Tamper Activado NO ENVIADA\r");
            Serial.print("-->Esperando t = "); Serial.print(nodo.t_wait); Serial.println(" s para reintentar ...");
            if (nodo.modo_wifi) { telnet.print("-->Esperando t = "); telnet.print((String) nodo.t_wait); telnet.println(" s para reintentar ...\r");}
          }
        }
      }
      else if (nodo.falso_tamper_des == 1) {
        Serial.println("============= ENVIANDO ALARMA DE TAMPER DESACTIVADO =============");
        if (nodo.modo_wifi) telnet.println("=============  ENVIANDO ALARMA DE TAMPER DESACTIVADO =============\r");
        char comando[1]={CMD_TAMPER_DES};
        if (sendPackage(comando, 1, espera_ACK, 0)) {
          //if (cmd_rcv==comando[0]) {
            nodo.falso_tamper_des = 0;
            nodo.t_wait = 0;
            nodo.cont_reintentos_alarma=0;
            Serial.println("-->Alarma de Falso Tamper Desactivado ENVIADA");
            if (nodo.modo_wifi) telnet.println("-->Alarma de Falso Tamper Desactivado ENVIADA\r");
          //}
          //else Serial.println("Comando de respuesta incorrecto");
        }
        else {
          nodo.cont_reintentos_alarma++;
          if (nodo.cont_reintentos_alarma >= MAX_REINTENTOS)
          {
            nodo.cont_reintentos_alarma = 0;
            nodo.falso_tamper_des=0;
            nodo.t_wait = 0;
            Serial.println("-->Cant Max de reintentos para Alarma excedido, descartar");
            if (nodo.modo_wifi) telnet.println("-->Cant Max de reintentos para Alarma excedido, descartar\r");
          }
          else
          {
            nodo.falso_tamper_des = 1;
            nodo.t_wait = random_time(MIN_RANDOM2,MAX_RANDOM2); //devuelve en segundos
            Serial.println("-->Alarma de Falso Tamper Desactivado NO ENVIADA");
            if (nodo.modo_wifi) telnet.println("-->Alarma de Falso Tamper Desactivado NO ENVIADA\r");
            Serial.print("-->Esperando t = "); Serial.print(nodo.t_wait); Serial.println(" s para reintentar ...");
            if (nodo.modo_wifi) { telnet.print("-->Esperando t = "); telnet.print((String) nodo.t_wait); telnet.println(" s para reintentar ...\r");}
          }
        }
      }
    }
    else if (nodo.t_wait > 0) { //tiempo de espera 
      nodo.t_wait--; //vamos decrementando el t_wait
    }
  }
}

void get_timestamp( unsigned long CMt_ ) {
  //Descomponer CMt
  for (byte i=0; i<4; i++) {
    CMt_array[3-i] = lowByte(CMt_);
    CMt_ >>= 8;   //desplazar aqui
  }

  if (dbtstm) {
    Serial.println("-------------get_timestamp--------------");
    Serial.printf("CMt: %d, CMt_array: {",CMt);
    for (int i = 0; i < 4; i++) {
      Serial.print(CMt_array[i], HEX); Serial.print(" ");
    }
    Serial.println("}");
  }
}

int32_t random_time(unsigned int MIN_,unsigned int MAX_) {
  //calcula un nuevo tiempo
  return random(MIN_, MAX_);
}


byte asignaCiclo(int32_t time_) { //Ingresar valor de hora del dia en segundos (0 a 86400 segundos)
  byte ciclo_;
  if (time_>=0 && time_<3600) { //Entre las 00:00:00 y las 00:59:59
    ciclo_=0;
  }
  else if (time_>=3600 && time_<7200) { //Entre las 01:00:00 y las 01:59:59
    ciclo_=1;
  }
  else if (time_>=7200 && time_<10800) { //Entre las 02:00:00 y las 02:59:59
    ciclo_=2;
  }
  else if (time_>=10800 && time_<14400) { //Entre las 03:00:00 y las 03:59:59
    ciclo_=3;
  }
  else if (time_>=14400 && time_<18000) { //Entre las 04:00:00 y las 04:59:59
    ciclo_=4;
  }
  else if (time_>=18000 && time_<21600) { //Entre las 05:00:00 y las 05:59:59
    ciclo_=5;
  }
  else if (time_>=21600 && time_<25200) { //Entre las 06:00:00 y las 06:59:59
    ciclo_=6;
  }
  else if (time_>=25200 && time_<28740) { //Entre las 07:00:00 y las 07:58:59
    ciclo_=7;
  }
  else if (time_>=28740 && time_<28799) { //Entre las 07:59:00 y las 07:59:58
    ciclo_=8;
  }
  else if (time_>=28800 && time_<32400) { //Entre las 08:00:00 y las 08:59:59
    ciclo_=0;
  }
  else if (time_>=32400 && time_<36000) { //Entre las 09:00:00 y las 09:59:59
    ciclo_=1;
  }
  else if (time_>=36000 && time_<39600) { //Entre las 10:00:00 y las 10:59:59
    ciclo_=2;
  }
  else if (time_>=39600 && time_<43200) { //Entre las 11:00:00 y las 11:59:59
    ciclo_=3;
  }
  else if (time_>=43200 && time_<46800) { //Entre las 12:00:00 y las 12:59:59
    ciclo_=4;
  }
  else if (time_>=46800 && time_<50400) { //Entre las 13:00:00 y las 13:59:59
    ciclo_=5;
  }
  else if (time_>=50400 && time_<54000) { //Entre las 14:00:00 y las 14:59:59
    ciclo_=6;
  }
  else if (time_>=54000 && time_<57540) { //Entre las 15:00:00 y las 15:58:59
    ciclo_=7;
  }
  else if (time_>=57540 && time_<57599) { //Entre las 15:59:00 y las 15:59:58
    ciclo_=8;
  }
  else if (time_>=57600 && time_<61200) { //Entre las 16:00:00 y las 16:59:59
    ciclo_=0;
  }
  else if (time_>=61200 && time_<64800) { //Entre las 17:00:00 y las 17:59:59
    ciclo_=1;
  }
  else if (time_>=64800 && time_<68400) { //Entre las 18:00:00 y las 18:59:59
    ciclo_=2;
  }
  else if (time_>=68400 && time_<72000) { //Entre las 19:00:00 y las 19:59:59
    ciclo_=3;
  }
  else if (time_>=72000 && time_<75600) { //Entre las 20:00:00 y las 20:59:59
    ciclo_=4;
  }
  else if (time_>=75600 && time_<79200) { //Entre las 21:00:00 y las 21:59:59
    ciclo_=5;
  }
  else if (time_>=79200 && time_<82800) { //Entre las 22:00:00 y las 22:59:59
    ciclo_=6;
  }
  else if (time_>=82800 && time_<86340) { //Entre las 23:00:00 y las 23:58:59
    ciclo_=7;
  }
  else if (time_>=86340 && time_<86399) { //Entre las 23:59:00 y las 23:59:58
    ciclo_=8;
  }
  return ciclo_;
}

bool enviarTodo() { //Trata de enviar todos los paquetes pendientes, si lo logra devuelve 1 sino 0
  if (nodo.stdby) return 0;
  int dir_para_enviar=dirParaEnviar();
  char payload_para_enviar[PL_CONT_MONO_SIZE]={0};
  bool ondaVerde=1;
  if (dir_para_enviar>=0) nodo.hay_paquetes_pendientes=1;
  else {
    nodo.hay_paquetes_pendientes=0;
    return 1;
  }

  while (nodo.hay_paquetes_pendientes && ondaVerde) { //Mientras hay paquetes para enviar y se mantiene la onda verde
    if (nodo.cont_intentos_uplink >= MAX_INTENTOS_UPLINK) {
      if (dbtsk) {
        nodo.is_deaf=1;
        Serial.println("-->Maximo de intentos de subida alcanzado");
        if (nodo.modo_wifi) telnet.println("-->Maximo de intentos de subida alcanzado\r");
      }
      return 0;
    }
    readPLFromEEPROM(dir_para_enviar,payload_para_enviar);

    if (dbtsk) {
      Serial.println("-->Enviando payloadContadores"); Serial.println("=============PAYLOAD CONTADORES to send (HEX)=============");
      for (int i = 0; i < PL_CONT_MONO_SIZE; i++) {
        Serial.print(payload_para_enviar[i], HEX); Serial.print(" ");
      }
      Serial.println(""); Serial.println("=======================================");
      if (nodo.modo_wifi) {
        telnet.println("-->Enviando payloadContadores\r"); telnet.println("=============PAYLOAD CONTADORES to send (HEX)=============\r");
        for (int i = 0; i < PL_CONT_MONO_SIZE; i++) {
          telnet.print(String(payload_para_enviar[i], HEX)); telnet.print(" ");
        }
        telnet.println("\r"); telnet.println("=======================================\r");
      }
    }
    nodo.cont_intentos_uplink++;
    if (sendPackage((char*)payload_para_enviar, PL_CONT_MONO_SIZE, espera_ACK, 0)) {
      ondaVerde=1;
      nodo.is_deaf=0;
      //nodo.cont_intentos_uplink=0;
      if (dbtsk) {
        Serial.println("-->Envio correcto");
        if (nodo.modo_wifi) telnet.println("-->Envio correcto\r");
      }
      marcarPaquete(dir_para_enviar,1); //Marcar como enviado
      cambio_bloque_PL=1;
      dir_para_enviar=dirParaEnviar();
      if (dir_para_enviar>=0) nodo.hay_paquetes_pendientes=1;
      else {
        nodo.hay_paquetes_pendientes=0;
        return 1;
      }
    }
    else {
      ondaVerde=0;
      if (slot_para_enviar>0) slot_para_enviar--; //Decrementa para que se envíe los paquetes de más reciente a menos reciente
      else if (slot_para_enviar == 0) slot_para_enviar=SLOTS_CANTIDAD_MAX-1;
      if (dbtsk) {
        Serial.println("-->Envio incorrecto");
        if (nodo.modo_wifi) telnet.println("-->Envio incorrecto\r");
      }
      return 0;
    }
  }
}

bool getCredentials() {
  //Envia un paquete de pedido de activacion automatica, devuelve 1
  byte macAddr_[6];
  int reintento_act=0;
  WiFi.macAddress(macAddr_);      //obtiene macadress de la placa wifi y la guarda en macAddr_
  //char MAC[6]={0x98,0xCD,0xAC,0x1A,0x9E,0x1C};
  char MAC[6]={0};
  for (int i = 0; i < 6; ++i)  {    //arma la MAC
    MAC[i]= (char) macAddr_[i];
  }
  Serial.printf("mac address: %02x:%02x:%02x:%02x:%02x:%02x\n", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
  loadPayloadACT(MAC);
  while (!nodo.stdby && !nodo.is_activated && reintento_act<MAX_REINTENTO_ACT) {
    if (sendPackage((char*)payloadACT, PL_ACT_SIZE, espera_Dato, 1)) {
      lora.update(); 
      if (lora.readData(datoEntrante) > 1) {
        procesarDatoEntrante();
      }
      char comando[1]={CMD_ACTIVACION};
      if (cmd_rcv==comando[0] && mac_correcta==1) {
        nodo.is_activated = 1;
        Serial.println("-->Proceso de activacion realizado con exito");
        break;
      }
      else Serial.println("Comando de respuesta incorrecto. No es mi MAC?");
    }
    else {
      Serial.print("Error enviando pedido de activacion. Reintento en 5 seg. Reintento #"); Serial.println(reintento_act++);
      delay(5000);
    }
  }
  if (nodo.is_activated) return 1;
  else return 0;
}

void aa_function() {
  if (!nodo.is_activated) {     //Si el bit de activacion está en 0, se solicitaran las credenciales
    if ( getCredentials() ) {    //si se activó, reiniciar
      Serial.print("Reiniciando... ");
      delay(2000);
      ESP.restart();
      delay(2000);
    }
    else {
      //si no se activó, encender y apagar las luces cada 1 seg
      led_seq(6);
      int cont_s=0;
      while(1) {
        delay(1000);
        cont_s++;
        if (cont_s>=ESPERA_FALLO_AA) {
          //Luego de un tiempo reiniciar
          Serial.print("Se supero el tiempo de espera. Reiniciando... ");
          delay(2000);
          ESP.restart();
          delay(2000);
        }
      }
    }
  }
}

int getOffsetID(unsigned int devID_) {
  int mod=devID_%100; //0 a 99
  mod=mod*216; //0min a 356min (6hs)
  return mod;
}

void printHora(uint8_t hora, uint8_t minutos, uint8_t segundos ) {
  char buffers[39];
  if (nodo.stdby && dbtsk<4) {
    Serial.print("<STDBY_ON> ");
    if (nodo.modo_wifi) telnet.print("<STDBY_ON> ");
  }
  switch (dbtsk) {
    case 0: return;
      break;

    case 1: 
      Serial.printf("-->Hora: %d:%d:%d \n",hora,minutos,segundos);
      if (nodo.modo_wifi) {
        sprintf(buffers,"-->Hora: %d:%d:%d ",hora,minutos,segundos);
        telnet.println(buffers);telnet.print("\r");
      } 
      break;
    case 2: 
      Serial.printf("-->Hora: %d:%d:%d, CMt=%d \n",hora,minutos,segundos,CMt);
      if (nodo.modo_wifi) {
        sprintf(buffers,"-->Hora: %d:%d:%d, CMt=%d \r",hora,minutos,segundos,CMt);
        telnet.println(buffers);telnet.print("\r");
      } 
      break;
    case 3: 
      Serial.printf("-->Hora: %d:%d:%d, CMt=%d, %d\n",hora,minutos,segundos,CMt,master_time);
      if (nodo.modo_wifi) {
        sprintf(buffers,"-->Hora: %d:%d:%d, CMt=%d, %d\r",hora,minutos,segundos,CMt,master_time);
        telnet.println(buffers);telnet.print("\r");
      } 
      break;
    case 4: 
      if (master_time%5==0) {
      //if (master_time%60==0) {
        if (nodo.stdby) {
          Serial.print("<STDBY_ON> ");
          if (nodo.modo_wifi) telnet.print("<STDBY_ON> ");
        }
        Serial.printf("-->Hora: %d:%d:%d \n",hora,minutos,segundos);
        if (nodo.modo_wifi) {
          sprintf(buffers,"-->Hora: %d:%d:%d ",hora,minutos,segundos);
          telnet.println(buffers);telnet.print("\r");
        }
      }
      else return;
      break;
  }
}


void falla_nodo(){          //cada cierto intervalo verifica si el nodo esta en falla y envia paquete de comando segun su falla
  cont_falla++;

  if (cont_falla > falla_interval)   //es hora de ver si el nodo esta en falla ?
  {
    Serial.println("es hora de ver si el nodo falla : ");
    cont_falla=0;
    
    //Verificar banderas de falla
    if (nodo.pdr_ok == 0)   //falla de red
        {
          Serial.println("falla de red");
          nodo.ff_pdr = 1;
          char Falla_red[1]={CMD_FRED};
          sendPackage(Falla_red, 1, no_espera_ACK, 1);
        }
    else if(nodo.is_sync == 0){   //falla de sincronizacion
      Serial.println("falla de sincronizacion");
      nodo.ff_sync = 1;
      char Falla_sync[1]={CMD_FSYNC};
      sendPackage(Falla_sync, 1, no_espera_ACK, 1);

    }
    else if(nodo.is_identify == 0){   //falla de identificacion
      Serial.println("falla de identificacion");
      nodo.ff_identify = 1;
      char Falla_iden[1]={CMD_FIDEN};
      sendPackage(Falla_iden, 1, no_espera_ACK, 1);

    }
    else if(nodo.is_activated==0){      //falla activacion
       Serial.println("falla de activacion");
       nodo.ff_activated = 1;
       char Falla_act[1]={CMD_FACT};
       sendPackage(Falla_act, 1, no_espera_ACK, 1);

    }


    }
  }
