const unsigned long interval = 15000;      // 15 s interval to send message
unsigned long previousMillis = 0;          // will store last time message sent
unsigned int counter = 0;                  // message counter
int rcv_count = 0;                         // contador de mensajes recibidos
int send_count = 0;                        // contador de mensajes enviados
int diferencia = 3;                        //diferencia para comparar send_count y rcv_count

int8_t rssi_rcv = 0;
int rssiValue=0;
char rssiSend[20]={0};                     //variable para guardar dato a enviar en forma de string
String get_name;                           //variable para guardar gatewayname

#define INPUTBUFF 100
char datoEntrante[INPUTBUFF]={0};          //variable para guardar dato recibido
              

byte recvStatus = 0;
bool packetReceived = false;

// variables para timeout
#define tick_time 100  // base de tiempo para el delay
#define timeout 150    // timeout * tick_time = tiempo de time out = 8 segundos



bool resultado_envio = false;

// INTERVALOS (en segundos)
#define LONG_TIME_TO_WAIT 600    // son 10 minutos, en s
#define UN_DIA            86400  // 24hs en s

#define MIN_RANDOM        5     //s, 10 segundos
#define MAX_RANDOM        10     //s, 60 segundos
#define MIN_RANDOM_LARGO  120    //s, 2 minutos
#define MAX_RANDOM_LARGO  600    //s, 10 minutos

//REINTENTOS
#define MAX_REINTENTOS 6    //es la cantidad  maxima de reintentos que hace con un intervalo de tiempo pequeño, luego espera un tiempo mas largo y vuelve a intentar. Min 6 para que use todos los SF

#define MAX_PAUSAS_LARGAS 3 //Maximo de pausas largas que se realizan cuando los reintentos cortos fallan, superado este numero se espera un dia completo

// estructura para guardar el estado del nodo
struct str {
  bool pdr_ok = false;                  // =1 cuando la prueba de red dio ok, sino =0
  
  int32_t t_wait = 0;                   // tiempo de espera
  bool pausa_larga = false;              //=1 esperar tiempo largo
  
  uint8_t cont_reintento_corto = 0;     // usado para contar los intentos uplink de las funciones pdr, sync e ident
  uint8_t cont_pausas_largas = 0;       // cuenta las pausas largas que se realizan cuando los reintentos cortos fallan
};  str nodo;

void IRAM_ATTR onReceive();
void ProcesarDatoEntrante(char *inputData, int8_t *rssi_out, String *name_out);