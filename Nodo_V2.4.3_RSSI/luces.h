#include <Ticker.h>

Ticker blinker1;
Ticker blinker2;

uint8_t secuenciaLed=255;

void flip3() {
  digitalWrite(pin_led, !digitalRead(pin_led));
}
void flip2() {
  // cambia / 100ms (flip3) / cambia (1 parpadeo)
  // si inicia con LOW: HIGH / 100ms / LOW  
  digitalWrite(pin_led, !digitalRead(pin_led));
  blinker1.once_ms(100, flip3); 
}
void flip1() {
  digitalWrite(pin_led, !digitalRead(pin_led));
  blinker1.once_ms(100, flip2); 
}
void flip0() {
  // cambia / 100ms (flip1) / cambia  / 100ms (flip2) / cambia / 100ms (flip3) / cambia (2 parpadeos)
  // si inicia con LOW: HIGH / 100ms / LOW / 100ms / HIGH / 100ms / LOW 
  digitalWrite(pin_led, !digitalRead(pin_led));
  blinker1.once_ms(100, flip1); 
}

void toggleLed2(void) {
  digitalWrite(pin_led, !digitalRead(pin_led));
  digitalWrite(pin_led_2, !digitalRead(pin_led_2));
}

//Esta función define la secuencia del led 2, ingresar el numero de secuencia que desee implementar
void led_seq(uint8_t seqnum) {
  switch (seqnum) {
      case 0:
      //Estado "Prueba de Red" (un pulso cada 1620ms)
        blinker2.detach();
        digitalWrite(pin_led,LOW);
        blinker2.attach_ms(1620, flip2);
      break;

      case 1:
      //Estado "Sincronizando" (dos pulsos cada 1620ms)
        blinker2.detach();
        digitalWrite(pin_led,LOW);
        blinker2.attach_ms(1620, flip0);
      break;

      case 2:
      //Estado "Identificando" (onda cuadrada, T/2=405ms)
        blinker2.detach();
        digitalWrite(pin_led,HIGH);
        blinker2.attach_ms(405, flip3);
      break;
      
      case 3:
      //Estado "Operativo" (un parpadeo cada 1620ms)
        blinker2.detach();
        digitalWrite(pin_led,HIGH);
        blinker2.attach_ms(1620, flip2);
      break;

      case 4:
      //Estado "Sin Respuesta" (dos parpadeos cada 1620ms)
        blinker2.detach();
        digitalWrite(pin_led,HIGH);
        blinker2.attach_ms(1620, flip0);
      break;

      case 5:
      //Estado "Buscando red WiFi" (onda cuadrada, T/2=100ms)
        blinker2.detach();
        blinker2.attach(0.1, toggleLed2);
      break;

      case 6:
      //Estado "Fallo en Activación" (onda cuadrada, T/2=2s)
        blinker2.detach();
        blinker2.attach(2, toggleLed2);
      break;
    }
}

void infoLED() {
  uint8_t led_seq_actual;
  if (nodo.irda_OK) {
    digitalWrite(pin_led_2, HIGH);
  }
  else {
    digitalWrite(pin_led_2, LOW);
  }
  if (nodo.is_deaf){
    led_seq_actual=4; //Sin respuesta
  }
  else if (nodo.is_identify){
    led_seq_actual=3; //Estado operativo
  }
  else if (nodo.is_sync) {
    led_seq_actual=2; //Identificando
  }
  else if (nodo.pdr_ok) {
    led_seq_actual=1; //Sincronizando
  }
  else {
    led_seq_actual=0; //Probando red
  }
  //Serial.printf("infoLed: %d",led_seq_actual);
  if (led_seq_actual != secuenciaLed) {
    led_seq(led_seq_actual);
    secuenciaLed=led_seq_actual;
  }
  
}
