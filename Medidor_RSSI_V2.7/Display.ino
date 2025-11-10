//Función Setup OLED
void config_OLED()
{
  pinMode(OLED_RST,OUTPUT);
  digitalWrite(OLED_RST, LOW);              // set GPIO16 low to reset OLED
  delay(20); 
  digitalWrite(OLED_RST, HIGH);
  
  //initialize OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR, false, false)) {       // Address 0x3C for 128x32
     Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.display();
  delay(2000); // Pause for 2 seconds
}

//Inicialización OLED
void init_OLED()
{
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print(VERSION);
  display.display();
  display.setTextSize(1);
  display.setCursor(0,40);
  display.print("Desarrollado por Macro Intell.");
  display.display();
  delay(1000);
  display.clearDisplay();
}

void mostrarDisplay(int16_t rssiTX, int16_t rssiRX, String *gatewayname){
    Serial.print("Tx: ");Serial.println(rssiTX);               
    display.setCursor(0,0);
    display.setTextSize(2);
    display.print("Tx: ");display.print(rssiTX);              // display.print(" dbm ");

    display.setTextSize(2);
    display.setCursor(0,35);
    
    Serial.print("Rx: ");Serial.println(rssiRX);
    display.print("Rx: ");display.print(rssiRX);
    display.setCursor(0,50);
    display.print(*gatewayname);
    Serial.print("gatewayname: ");Serial.println(*gatewayname);
    display.display();
    display.clearDisplay();
}