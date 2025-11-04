void config_task() {
  Serial.println("Initialized scheduler");

  PDR.addTask(tIntentarEnvio);
  PDR.addTask(tDecrementarEspera);
  PDR.addTask(tEsperarAck);
  PDR.addTask(tFinalizar);

  tIntentarEnvio.enable();
  tDecrementarEspera.disable();
  tEsperarAck.disable();
  tFinalizar.disable();
}

void task_decrementarEspera() {
  if (!nodo.pdr_ok && nodo.t_wait > 0) {
    nodo.t_wait--;
    if (nodo.t_wait == 0) {
      tIntentarEnvio.restartDelayed(0);
    }
  }
}

void task_intentarEnvio() {
  if (!nodo.pdr_ok && nodo.t_wait == 0) {
    Serial.println("intentar envio");
    intentarEnvioPDR();
  }
}

void tareaEsperarAck() {
  static uint8_t cont_timeout = 0;
  if (lora.readAck()) {
    Serial.println("--> ACK recibido");
    resultado_envio = true;
    tEsperarAck.disable();
    tFinalizar.enable();
  } else {
    cont_timeout++;
    if (cont_timeout >= timeout) {
      Serial.println("--> NO ACK, time out");
      resultado_envio = false;
      cont_timeout = 0;
      tEsperarAck.disable();
      tFinalizar.enable();
    }
  }
}

void intentarEnvioPDR() {
  char uncero[1] = { 0 };
  lora.sendUplink(uncero, strlen(uncero), 1, 1);
  tEsperarAck.enable();
}

void tareaFinalizar() {
  if (resultado_envio) {
    Serial.println("Paquete enviado con exito");
  } else
    Serial.println("Fallo el envío");
}

// ==================== FUNCIONES DE PDR ====================

void procesarExitoPDR() {
  nodo.pdr_ok = true;
  nodo.t_wait = random_time(0, MAX_RANDOM_LARGO);
  nodo.pausa_larga = false;
  nodo.cont_pausas_largas = 0;
  nodo.cont_reintento_corto = 0;

  Serial.println("-->PRUEBA DE RED: OK");
}

void procesarFalloPDR() {
  nodo.pdr_ok = false;
  nodo.cont_reintento_corto++;

  if (nodo.cont_reintento_corto >= MAX_REINTENTOS) {
    nodo.cont_reintento_corto = 0;
    nodo.pausa_larga = true;
    nodo.cont_pausas_largas++;

    if (nodo.cont_pausas_largas >= MAX_PAUSAS_LARGAS) {
      nodo.cont_pausas_largas = 0;
      nodo.t_wait = UN_DIA;  // esperar un día completo
    } else {
      nodo.t_wait = LONG_TIME_TO_WAIT; // tiempo largo y fijo
    }

    Serial.println("-->Cant Max de reintentos para PDR excedido, pausa larga");
    Serial.print("-->Esperando t = "); Serial.print(nodo.t_wait); Serial.println(" s para reintentar PDR...");
  } else {
    nodo.t_wait = random_time(MIN_RANDOM, MAX_RANDOM); // tiempo aleatorio
    Serial.println("-->PRUEBA DE RED: FALLO");
    Serial.print("-->Esperando t = "); Serial.print(nodo.t_wait); Serial.println(" s para reintentar PDR...");
  }
}


int32_t random_time(unsigned int MIN_,unsigned int MAX_) {
  //calcula un nuevo tiempo
  return random(MIN_, MAX_);
}