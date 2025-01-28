bool isConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

/* ------------------------------------------------- */

bool connectToWiFi(const char* ssid, const char* password, int max_tries, int pause) {
  int i = 0;
  WiFi.mode(WIFI_STA);
  #if defined(ARDUINO_ARCH_ESP8266)
    WiFi.forceSleepWake();
    delay(200);
  #endif
  WiFi.begin(ssid, password);
  do {
    delay(pause);
    Serial.print(".");
  } while (!isConnected() && i++ < max_tries);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  return isConnected();
}

/* ------------------------------------------------- */

void errorMsg(String error, bool restart) {
  Serial.println(error);
  if (restart) {
    Serial.println("Rebooting now...");
    delay(5000);
    ESP.restart();
    delay(2000);
  }
}

/* ------------------------------------------------- */

void setupTelnet() {  
  // passing on functions for various telnet events
  telnet.onConnect(onTelnetConnect);
  telnet.onConnectionAttempt(onTelnetConnectionAttempt);
  telnet.onReconnect(onTelnetReconnect);
  telnet.onDisconnect(onTelnetDisconnect);
  
  // passing a lambda function
  telnet.onInputReceived([](String str) {
    // checks for a certain command
    if (str.indexOf("help") != -1) {
      telnet.print("\r         \n\rComandos disponibles: \n\r         help\n\r         ping\n\r         plg\n\r         dbplg 0/1\n\r         cmtc\n\r         dbcmtc 0/1\n\r         dbtsk 0/1/2/3/4\n\r         dbint 0/1\n\r         dbprm 0/1\n\r         modo_wifi 0/1\n\r         reset\n\r         showid\r\n         clear\r\n         version\r\n         stdby 0/1\r\n         last_pkt\r\n         dir_actual\r\n");    }
    else if (str.indexOf("ping") != -1) {
      telnet.println("pong\r");
      Serial.println("- Telnet: pong");
    }
    else if (str.indexOf("dbtsk 0") != -1) {
      telnet.println("No se muestra la hora del RTC\r");
      Serial.println("- Telnet: dbtsk 0");
      dbtsk=0;
    }
    else if (str.indexOf("dbtsk 1") != -1) {
      telnet.println("Se muestra la hora del RTC cada 1s\r");
      Serial.println("- Telnet: dbtsk 1");
      dbtsk=1;
    }
    else if (str.indexOf("dbtsk 2") != -1) {
      telnet.println("Se muestra la hora del RTC y CMT cada 1s\r");
      Serial.println("- Telnet: dbtsk 2");
      dbtsk=2;
    }
    else if (str.indexOf("dbtsk 3") != -1) {
      telnet.println("Se muestra la hora del RTC, CMT y master_time cada 1s\r");
      Serial.println("- Telnet: dbtsk 3");
      dbtsk=3;
    }
    else if (str.indexOf("dbtsk 4") != -1) {
      telnet.println("Se muestra la hora del RTC cada 5s\r");
      Serial.println("- Telnet: dbtsk 4");
      dbtsk=4;
    }
    else if (str.indexOf("dbint 0") != -1) {
      telnet.println("No se muestra indicador de interrupciones\r");
      Serial.println("- Telnet: dbint 0");
      dbint=0;
    }
    else if (str.indexOf("dbint 1") != -1) {
      telnet.println("Se muestra indicador de interrupciones\r");
      Serial.println("- Telnet: dbint 1");
      dbint=1;
    }
    else if (str.indexOf("dbcmtc 0") != -1) {
      telnet.println("No se muestra CMt corte guardados\r");
      Serial.println("- Telnet: dbcmtc 0");
      dbcmtc=0;
    }
    else if (str.indexOf("dbcmtc 1") != -1) {
      telnet.println("Se muestra CMt corte guardados\r");
      Serial.println("- Telnet: dbcmtc 1");
      dbcmtc=1;
    }
    else if (str.indexOf("dbplg 0") != -1) {
      telnet.println("No se muestra payloads guardadas\r");
      Serial.println("- Telnet: dbplg 0");
      dbplg=0;
    }
    else if (str.indexOf("dbplg 1") != -1) {
      telnet.println("Se muestra payloads guardadas\r");
      Serial.println("- Telnet: dbplg 1");
      dbplg=1;
    }
    else if (str.indexOf("modo_wifi 0") != -1) {
      telnet.println("Se desactivara el WiFi al iniciar\r");
      Serial.println("- Telnet: modo_wifi 0");
      modoWifi(0);
    }
    else if (str.indexOf("modo_wifi 1") != -1) {
      telnet.println("Se activara el WiFi al iniciar\r");
      Serial.println("- Telnet: modo_wifi 1");
      modoWifi(1);
    }
    else if (str.indexOf("reset") != -1) {
      telnet.println("Reseteando...");
      Serial.println("- Telnet: reset");
      delay(2000);
      ESP.restart();
      delay(2000);
    }
    else if (str.indexOf("showid") != -1) {
      telnet.print("devID: ");
      #if defined devID
        telnet.println(String(devID)+"\r");
      #else
       telnet.println(String(devID_AA)+"\r");
      #endif
      Serial.println("- Telnet: showid");
    }
    else if (str.indexOf("plg") != -1) {
      telnet.println("Mostrar Payloads Guardadas\r");
      Serial.println("- Telnet: plg");
      leerPLGuardadas();
    }
    else if (str.indexOf("cmtc") != -1) {
      telnet.println("Mostrar CMt Corte Guardados\r");
      Serial.println("- Telnet: cmtc");
      leerCMtGuardados();
    }
    else if (str.indexOf("dbprm 0") != -1) {
      telnet.println("No se muestra parametros de trama irda\r");
      Serial.println("- Telnet: dbprm 0");
      dbprm=0;
    }
    else if (str.indexOf("dbprm 1") != -1) {
      telnet.println("Se muestra parametros de trama irda\r");
      Serial.println("- Telnet: dbprm 1");
      dbprm=1;
    }
    else if (str.indexOf("clear") != -1) {
      telnet.println("Se limpiara la memoria EEPROM");
      Serial.println("- Telnet: clear");
      clearEEPROM();
      delay(2000);
      ESP.restart();
      delay(2000);
    }
    else if (str.indexOf("version") != -1) {
      telnet.print("Version: "); telnet.print(VERSION); telnet.println("\r");
      Serial.println("- Telnet: version");
    }
    else if (str.indexOf("stdby 0") != -1) {
      telnet.println("STANDBY: OFF\r");
      Serial.println("- Telnet: stdby 0");
      if (nodo.stdby == 1) {
        telnet.println("No se silenciara el nodo\r");
        nodo.stdby = 0;
        cont_stdby = 0;
        writeStdby(0); //Escribir en memoria el byte stdby
      }
    }
    else if (str.indexOf("stdby 1") != -1) {
      telnet.println("STANDBY: ON\r");
      Serial.println("- Telnet: stdby 1");
      if (nodo.stdby == 0) {
        telnet.println("El nodo se encuentra silenciado ahora\r");
        nodo.stdby = 1;
        cont_stdby = UN_DIA;
        writeStdby(1); //Escribir en memoria el byte stdby
      }
    }
    else if (str.indexOf("last_pkt") != -1) {
      byte slot_last_pkt = lastPL(); //Slot del paquete mas reciente
      Serial.println("- Telnet: last_pkt");
      if (slot_last_pkt<SLOTS_CANTIDAD_MAX) {
        telnet.print("Ultimo paquete en slot: "); 
        telnet.print(String(slot_last_pkt)); telnet.print(", dir: "); 
        telnet.print(String(pos_mem[slot_last_pkt])); telnet.println("\r");
      }
      else telnet.println("No hay paquetes guardados\r");
    }
    else if (str.indexOf("dir_actual") != -1) {
      Serial.println("- Telnet: dir_actual");
      telnet.print("dir_actual: "); telnet.print(String(dir_actual)); telnet.println("\r");
    }
    else {
      telnet.println("Comando desconocido\r");
      Serial.print("- Telnet: ");Serial.println(str);
    }
  });

  Serial.print("- Telnet: ");
  if (telnet.begin()) {
    Serial.println("running");
  } else {
    Serial.println("Telnet error.");
    errorMsg("Will reboot...");
  }
}

/* ------------------------------------------------- */

// (optional) callback functions for telnet events
void onTelnetConnect(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" conectado");
  telnet.print("********************************************************************************\n\r* NODO TELE LECTOR LORAWAN "+String(VERSION)+"\n\r*\n\r* Desarrollado por Macro Intell para DPEC \n\r* Autor: Hugo Ferrari \n\r* Año: 2023\n\r********************************************************************************\r\n\nBienvenido "+ telnet.getIP() +"\n\r");
  telnet.print("\r         \n\rPara ver los comandos disponibles: \n\r   help\n\r");
}

void onTelnetDisconnect(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" disconnected");
}

void onTelnetReconnect(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" reconnected");
}

void onTelnetConnectionAttempt(String ip) {
  Serial.print("- Telnet: ");
  Serial.print(ip);
  Serial.println(" tried to connected");
}
