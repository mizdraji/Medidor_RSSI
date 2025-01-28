//Version
#define VERSION "V2.4.3_RSSI"

//Serial
#define SERIAL_SPEED 115200

//WiFi
//#define WIFI_SSID       "MI-programacion"
//#define WIFI_PASSWORD   "Macro2021"
//#define WIFI_SSID       "Fibertel WiFi640 2.4GHz"
//#define WIFI_PASSWORD   "00423742291"
//#define WIFI_SSID       "laboratorio macrointell" //"Android de muris"
//#define WIFI_PASSWORD   "desarrollo2022"  //"sipuemihermane"
#define WIFI_SSID       "Android de muris"
#define WIFI_PASSWORD   "sipuemihermane"

//LoraTec
#define tipo_modulo 1     //Es para definir el pinout de SPI con el modulo Lora. 0: ESP12, 1: ESP32
#define RANDOM_CHANNEL 1
#define DEFAULT_CHANNEL CH0
#define DEFAULT_SF SF7BW125
#define DEBUG 0 
#define dbspk 1 //lora send pakage

//Lector infrarrojos
#define dbirda 0

//Hardware
#define dbpdt 1 //procesar dato
#define dbplact 0 //muestra payload activacion
#define dbplst 1 //muestra payload inicio
#define dbplid 1 //muestra payload id
#define dbplcm 0 //muestra payload contadores mono
#define dbrmraw 0 //muestra la trama completa del medidor
#define dbtstm 0 //muestra time stamp
#define dbmem 0 //muestra mensajes relacionados a la memoria EEPROM

//Configurable por Telnet
byte dbtsk=0; //Hora del RTC. 0: Desactivar. 1: Muestra Hora (cada 1s). 2: Muestra Hora, CMt. 3: Muestra Hora, CMt, master_time. 4: Muestra Hora (cada 5s)
bool dbint=1; //muestra la flechita que indica una nueva trama irda
bool dbcmtc=0; //muestra CMt corte guardados
bool dbplg=0; //muestra payloads guardadas
bool dbprm=1; //muestra parametros de trama irda
