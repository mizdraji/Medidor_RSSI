
// variables para timeout
#define tick_time 100  // base de tiempo para el delay
#define timeout 150    // timeout * tick_time = tiempo de time out = 8 segundos

Scheduler PDR;  // Tareas relacionadas a la prueba de red PDR

bool resultado_envio = false;

// INTERVALOS (en segundos)
#define LONG_TIME_TO_WAIT 600    // son 10 minutos, en s
#define UN_DIA            86400  // 24hs en s

#define MIN_RANDOM        10     //s, 10 segundos
#define MAX_RANDOM        60     //s, 60 segundos
#define MIN_RANDOM_LARGO  120    //s, 2 minutos
#define MAX_RANDOM_LARGO  600    //s, 10 minutos

//REINTENTOS
#define MAX_REINTENTOS 6    //es la cantidad  maxima de reintentos que hace con un intervalo de tiempo pequeño, luego espera un tiempo mas largo y vuelve a intentar. Min 6 para que use todos los SF

#define MAX_PAUSAS_LARGAS 3 //Maximo de pausas largas que se realizan cuando los reintentos cortos fallan, superado este numero se espera un dia completo

void procesarExitoPDR();
void procesarFalloPDR();

// estructura para guardar el estado del nodo
struct str {
  bool pdr_ok = false;                  // =1 cuando la prueba de red dio ok, sino =0
  
  int32_t t_wait = 0;                   // tiempo de espera
  bool pausa_larga = false;              //=1 esperar tiempo largo
  
  uint8_t cont_reintento_corto = 0;     // usado para contar los intentos uplink de las funciones pdr, sync e ident
  uint8_t cont_pausas_largas = 0;       // cuenta las pausas largas que se realizan cuando los reintentos cortos fallan
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
