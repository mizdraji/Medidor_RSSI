
// variables para timeout
#define tick_time 100  // base de tiempo para el delay
#define timeout 150    // timeout * tick_time = tiempo de time out = 8 segundos

Scheduler PDR;  // Tareas relacionadas a la prueba de red PDR

bool resultado_envio = false;

// INTERVALOS (en segundos)
#define delta_t 1              // 1 segundo
#define INTERVAL1 60           // 60 seg = 1 min
#define INTERVAL8 28800        // 28800 seg = 8 hs
#define INTERVAL05 300         // 5 min, se usa para verificar canal por defecto
#define LONG_TIME_TO_WAIT 600  // son 10 minutos, en s
#define UN_DIA 86400           // 24hs en s
#define ESPERA_FALLO_AA 7200   // 7200s: 2h. Tiempo de espera para reiniciar por fallo en el proceso de activacion auto

#define DEBUG 1       // PARA BAJAR TIEMPOS, debug = 1
#if DEBUG == 1
#define MIN_RANDOM 10 //s, 10 segundos
#define MAX_RANDOM 60 //s, 60 segundos
#define MIN_RANDOM_LARGO 120 //s, 2 minutos
#define MAX_RANDOM_LARGO 600 //s, 10 minutos
#else
#define MIN_RANDOM 60 //s, 1 min
#define MAX_RANDOM 300 //s, 5 min
#define MIN_RANDOM_LARGO 600 //s, 10 minutos
#define MAX_RANDOM_LARGO 3000 //s, 50 minutos
#endif

//REINTENTOS
#define MAX_REINTENTOS 6    //es la cantidad  maxima de reintentos que hace con un intervalo de tiempo pequeño, luego espera un tiempo mas largo y vuelve a intentar. Min 6 para que use todos los SF
#define MAX_REINTENTO_SAME_PACKAGE 3 //cantidad maxima de reintentos cortos en el envio de paquetes de contadores
#define MAX_REINTENTO_ACT 10    //Maximo de pedidos de activacion automatica
#define MAX_REINTENTO_NULO 10   //Maximo de paquetes nulos (paquetes de cambio de canal por defecto)
#define MAX_INTENTOS_UPLINK 10  //Maximo de reintentos por periodo de transmision
#define MAX_PAUSAS_LARGAS 3 //Maximo de pausas largas que se realizan cuando los reintentos cortos fallan, superado este numero se espera un dia completo

void procesarExitoPDR();
void procesarFalloPDR();

// estructura para guardar el estado del nodo
struct str {
  bool pdr_ok = false;                  // =1 cuando la prueba de red dio ok, sino =0
  bool is_sync = false;                 //=1 cuando la sincronizacion dio ok, sino =0
  int32_t t_wait = 0;                   // tiempo de espera
  uint8_t pausa_larga = 0;              //=1 esperar tiempo largo
  bool is_identify = false;             //=1 cuando la identificacion dio ack_ok, sino =0
  bool hay_paquetes_pendientes = 0;     //=1 cuando hay al menos un paquete pendiente de envio, sino =0
  uint8_t cont_reintentos_paquete = 0;  // usado para contar los intentos uplink de paquetes de contadores
  uint8_t cont_reintento_corto = 0;     // usado para contar los intentos uplink de las funciones pdr, sync e ident
  uint8_t cont_pausas_largas = 0;       // cuenta las pausas largas que se realizan cuando los reintentos cortos fallan
  uint8_t irda_OK = 0;

  uint8_t is_activated = 0;
  uint8_t falso_tamper_act = 0;
  uint8_t falso_tamper_des = 0;
  uint8_t cont_reintentos_alarma = 0;
  uint8_t cont_paq_nulos = 0;        // Usado para contar los paquetes de cambio de canal por defecto
  uint8_t cont_intentos_uplink = 0;  // Usado para limitar la cantidad de reintentos por periodo de transmision
  uint8_t bloqueo_reintentos = 1;    // Usado para bloquear el envio de paquetes de contadores hasta la hora de transmitir
  uint8_t is_deaf = 0;               //=1 cuando los intentos uplink alcanzan el maximo permitido, =0 cuando se recibe ack
};  str nodo;

// --------------------------------------------------------------------------
// Prototipos de funciones
// --------------------------------------------------------------------------
void config_task();
void task_decrementarEspera();
void task_intentarEnvio();
void tareaEsperarAck();
void procesarExitoPDR();
void tareaFinalizar();
void intentarEnvioPDR();
int32_t random_time(unsigned int MIN_, unsigned int MAX_);

// Tareas de PDR
inline Task tDecrementarEspera(1000, TASK_FOREVER, &task_decrementarEspera, &PDR, true);
inline Task tIntentarEnvio(0, TASK_ONCE, &task_intentarEnvio, &PDR, false);
inline Task tEsperarAck(tick_time, timeout, &tareaEsperarAck, &PDR, false);
inline Task tFinalizar(TASK_IMMEDIATE, TASK_ONCE, &tareaFinalizar, &PDR, false);
