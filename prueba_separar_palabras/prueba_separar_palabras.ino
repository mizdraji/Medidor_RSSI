#include <Arduino.h>

void setup() {
  Serial.begin(9600);

  char input[] = "-27,gateway";
  char *token;

  // Obtener el primer token (número)
  token = strtok(input, ",");
  if (token != NULL) {
    int numero = atoi(token);
    Serial.print("Número: ");
    Serial.println(numero);
  }

  // Obtener el segundo token (palabra)
  token = strtok(NULL, ",");
  if (token != NULL) {
    String palabra(token);
    Serial.print("Palabra: ");
    Serial.println(palabra);
  }
}

void loop() {
  // Tu código aquí
}
