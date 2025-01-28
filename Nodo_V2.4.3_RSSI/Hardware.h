#if DEBUG == 1
#define MIN_RANDOM 10 //s, 10 segundos
#define MAX_RANDOM 60 //s, 60 segundos
#define MIN_RANDOM_LARGO 120 //s, 2 minutos
#define MAX_RANDOM_LARGO 600 //s, 10 minutos
#define OFFSET_FIJO 300         //offset de tiempo para empezar a transmitir, 5 minutos
#else
#define MIN_RANDOM 60 //s, 1 min
#define MAX_RANDOM 300 //s, 5 min
#define MIN_RANDOM_LARGO 600 //s, 10 minutos
#define MAX_RANDOM_LARGO 3000 //s, 50 minutos
#define OFFSET_FIJO 1800        //offset de tiempo para empezar a transmitir, 30 minutos
#endif

//variables para definir el tiempo corto
#define interval_TC 300000     //intervalo para tiempos cortos igual a 5 min
uint32_t contador_TC = 0;
bool inicio_TC = 1;           //bandera para saber q es el inicio de tiempos cortos
uint32_t prevMillis_TC = 0;
uint16_t MIN_RANDOM2 = 20;            // 10 segundos
uint16_t MAX_RANDOM2 =40;             // 40 segundos
uint16_t MIN_RANDOM_LARGO2 = 40;      // 40 segundos
uint16_t MAX_RANDOM_LARGO2 = 60;      // 60 segundos
uint16_t OFFSET_FIJO2 = 180;          //offset de tiempo para empezar a transmitir, 3min

unsigned long CMt=0; //segundos desde 01-01-2000 00:00:00
int32_t master_time = 0;  //0 a 86400 segundos
uint32_t next_time_sync = 0;
byte ciclo=0;

int offset_id=0; //offset de tiempo, depende del devID, se suma al offset fijo

char CMt_array[4]={0};

#define pin_led 2 //Led 1 verde de placa y led integrado, logica invertida.  
#define pin_led_2 17 //Led 2 azul de la placa
#define pin_analog A0  // ESP8266 Analog Pin ADC0 = A0
bool estado_led = 0;
bool first_time=1;

//estructura para guardar el estado del nodo
struct str {
  uint8_t pdr_ok = 0;   // =1 cuando la prueba de red dio ok, sino =0
  uint8_t is_sync = 0;                //=1 cuando la sincronizacion dio ok, sino =0
  int32_t t_wait = 0;                 //tiempo de espera
  uint8_t pausa_larga = 0;             //=1 esperar tiempo largo
  uint8_t is_identify = 0;            //=1 cuando la identificacion dio ack_ok, sino =0
  uint8_t hay_paquetes_pendientes = 0;      //=1 cuando hay al menos un paquete pendiente de envio, sino =0
  uint8_t cont_reintentos_paquete=0;  //usado para contar los intentos uplink de paquetes de contadores
  uint8_t cont_reintento_corto = 0; //usado para contar los intentos uplink de las funciones pdr, sync e ident
  uint8_t cont_pausas_largas = 0; //cuenta las pausas largas que se realizan cuando los reintentos cortos fallan
  uint8_t irda_OK =0;
  uint8_t modo_wifi=0;
  uint8_t is_activated=0;
  uint8_t falso_tamper_act=0;
  uint8_t falso_tamper_des=0;
  uint8_t cont_reintentos_alarma = 0;
  uint8_t cont_paq_nulos=0; //Usado para contar los paquetes de cambio de canal por defecto
  uint8_t cont_intentos_uplink=0; //Usado para limitar la cantidad de reintentos por periodo de transmision
  uint8_t bloqueo_reintentos=1; //Usado para bloquear el envio de paquetes de contadores hasta la hora de transmitir
  uint8_t is_deaf=0; //=1 cuando los intentos uplink alcanzan el maximo permitido, =0 cuando se recibe ack
  uint8_t stdby=0; //=1 Modo Standby, se inhiben los uplink, solo envia un paquete de comando stdby cada 24hs 

  bool ff_pdr=0;        //bandera falla red, 1= falla
  bool ff_activated=0;  //bandera falla activacion, 1=falla
  bool ff_identify =0;  //bandera identificacion, 1=falla
  bool ff_sync = 0;     //bandera sincronizacion, 1 = falla 
};
str nodo;

int32_t random_time(unsigned int MIN_,unsigned int MAX_);
void pdr_function();
uint8_t sync_function( void );
void identify_function( void );
void contTask( void );
void defCHSF_function();
void stdby_function();
void config_pines( void );
void RTC( void );
bool getCredentials();
int getOffsetID(unsigned int devID_);
