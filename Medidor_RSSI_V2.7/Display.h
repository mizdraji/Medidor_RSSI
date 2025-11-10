//pins OLED
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define OLED_ADDR 0x3C
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

void config_OLED();       //Setup OLED
void init_OLED();         //inicialización OLED
void mostrarDisplay(int16_t rssiTX, int16_t rssiRX, String *gatewayname);     //funcion para visualizar en display valores de rssiTX, rssiRX y gatewayname
void ProcesarFalla();