
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