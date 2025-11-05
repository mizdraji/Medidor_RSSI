void config_task() {
  Serial.println("Initialized scheduler");

  //PDR.addTask(tIntentarEnvio);
  PDR.addTask(tDecrementarEspera);
  PDR.addTask(tEsperarAck);
}

// ==================== FUNCIONES DE PDR ==================== //
void decrementarEspera() {
  if (!nodo.pdr_ok) {
    if (nodo.t_wait > 0) {
      nodo.t_wait--;
    }
    else {
      tDecrementarEspera.disable();       //---> ejecuta la funcion intentarEnvio, cuando tDecrementarEspera es deshabilitada.
    }
  }
}

void intentarEnvioPDR() {
  if (!nodo.pdr_ok && nodo.t_wait == 0) {
    nodo.t_wait = random(MIN_RANDOM, MAX_RANDOM);
    char uncero[1] = { 0 };
    lora.sendUplink(uncero, strlen(uncero), 1, 1);
    tEsperarAck.enable();
  }
}

void EsperarAck() {
  static uint8_t cont_timeout = 0;
  if (lora.readAck()) {
    Serial.println("--> ACK recibido");
    nodo.pdr_ok = true;
    tEsperarAck.disable();
  } else {
    cont_timeout++;
    if (cont_timeout >= timeout) {
      Serial.println("--> NO ACK, time out");
      nodo.pdr_ok = false;
      cont_timeout = 0;
      tEsperarAck.disable();
      nodo.t_wait = random(MIN_RANDOM,MAX_RANDOM);
      tDecrementarEspera.enable();
    }
  }
}