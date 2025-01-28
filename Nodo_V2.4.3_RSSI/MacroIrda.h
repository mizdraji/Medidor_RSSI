/*
<----------------------------------------------
Macro Irda 1052 150 v1: Lector del puerto serial infrarrojo de medidores Elster A1052 plus, A102 y A150.
Desarrollado por Hugo Ferrari para Macro Intell S.A. en Abril del 2021
Esta versión detecta el tipo de medidor por la transmision del puerto de datos seriales infrarrojos
y se configura para obtener los parámetros de cada uno según corresponda.

Arranca intentando decodificar un A150, si falla 3 veces cambia de medidor. Si vuelve a fallar 3 veces 
cambia de nuevo hasta volver al A150. Tener en cuenta que para que considere erronea una trama de A1052 se necesitan
8 tramas de A150.
<----------------------------------------------
*/
//const uint8_t intPin = 12; //GPIO 12 (D6)
const uint8_t intPin = 39; //GPIO 4 (D2)
#define BIT_PERIOD 416 // us
//The following global variables in the sketch will hold the time elapsed every time an IR signal is detected:
#define BUFF_SIZE 100 //24 es aceptable
int data[BUFF_SIZE];         //“data” array holds up to 100 time measurements, 
volatile uint8_t in;         //“in” pointer for the ISR to know whereto store a new reading,       
volatile uint8_t out;       // “out” points the decoding function where to find the next measurement to be analysed. 
volatile unsigned long last_us;

uint8_t dbug = 0;
byte trama[190]; //Aca se guardan los bytes de la trama
byte idt=0; //contador para la posicion dentro de la trama
uint16_t idx=0;
uint8_t pSum = 0;
int p_buff[10]={0,0,0,0,0,0,0,0,0,0}; //aca voy a guardar los tiempos de bit
uint8_t id=0;
byte err=0; //bandera de error
byte crct=0; //aca voy a guardar los caracteres formados por dos nibbles
byte par=0; //variable para saber el orden (par o impar) en el que llegan los nibbles y ordenarlos
byte hi=0;  //nibble superior
byte lo=0;  //nibble inferior
byte printed=0;
//bool irda_OK=0;
bool trama150ok = 0;
bool trama102ok = 0;
bool trama103ok = 0;
bool trama1052ok = 0;

//bool buffOut [11] = {0,0,0,0,0,0,0,0,0,0,0};
bool buffOut [10] = {0,0,0,0,0,0,0,0,0,0}; //Arreglo de binarios que representa la palabra de bit
byte dato; //dato hexadecimal
int sum = 0;
bool ok;
double x, entero, decimal = 0.00;
char renglon[30];
int k, r;//indice para moverme dentro de renglon, contador de renglones sin reconocer
byte tipoMedidor=0; // Puede ser A1052 (0), A150 (1), A102 (2), A103(3) o indeterminado (248-255)
bool encontrado=0;
byte pausas=0;  //Contador de pausas entre tramas en el lector infrarrojo
bool lectura0=1;  //Bandera para indicar que se realiza la primer lectura Irda, usado para calcular demanda
bool calculardmd=0; //Bandera para indicar que es la hora de calcular la demanda (15min)
unsigned long millis_lectura0=0; //tiempo en milisegundos de la ultima vez que se registro lectura para el calculo de demanda


struct strInfo {
  unsigned long IAacum; //para calcular corriente promedio (numerador)
  unsigned long IBacum; //para calcular corriente promedio (numerador)
  unsigned long ICacum; //para calcular corriente promedio (numerador)
  int nI; //para calcular corriente promedio (denominador)
  unsigned long KWh_0; //para calcular demanda
  unsigned int Dmd_4; // Demanda/4, calculada cada 15 min
  unsigned int Dmd_max; //Demanda maxima de 1 hora (valor maximo de Dmd_4*4 en una hora)
  char Product_Code[12]; //ascii
  char MedidorTipo[2];
  // char Free_text[9];
  // char FT[4];
  char KWh[5]; //viene bcd-->ascii de 10
  char KVArh[5]; //viene bcd
  // char STATUS[1];
  char Tiempo_Servicio[4];
  // char KVA[5]; //Nota 3
  // char KVAr[5];
  // char Cuadrante[1]; //bcd
  // char PF[2];
  // char Meter_type[1];
  // char React_config[1];
  char IntensA[2]; //bcd
  char IntensB[2]; //bcd
  char IntensC[2]; //bcd
  // char Current_B[2];
  char TensionA[2]; //bcd
  char TensionB[2]; //bcd
  char TensionC[2]; //bcd
  char Creep_Time[3]; //bcd
  // char Dmd_Reset_Count[1];
  // char Reverse_Count[1];
  // char Meter_Reset_Count[1];
  char MedidorID[16]; //ascii
  // char Dmd_Prev[3];
  // char Dmd_Acum[3];
  // char Dmd_Max[3];

  void clear() {
    for (int i = 0; i < 12; ++i) Product_Code[i]=0;
    for (int i = 0; i < 2; ++i) MedidorTipo[i]=0;
    // for (int i = 0; i < 9; ++i) Free_text[i]=0;
    // for (int i = 0; i < 4; ++i) FT[i]=0;
    for (int i = 0; i < 5; ++i) KWh[i]=0;
    for (int i = 0; i < 5; ++i) KVArh[i]=0;
    // STATUS[0]=0;
    for (int i = 0; i < 4; ++i) Tiempo_Servicio[i]=0;
    // for (int i = 0; i < 5; ++i) KVA[i]=0;
    // for (int i = 0; i < 5; ++i) KVAr[i]=0;
    // Cuadrante[0]=0;
    // for (int i = 0; i < 2; ++i) PF[i]=0;
    // Meter_type[0]=0;
    // React_config[0]=0;
    for (int i = 0; i < 2; ++i) IntensA[i]=0;
    for (int i = 0; i < 2; ++i) IntensB[i]=0;
    for (int i = 0; i < 2; ++i) IntensC[i]=0;
    //for (int i = 0; i < 2; ++i) Current_B[i]=0;
    for (int i = 0; i < 2; ++i) TensionA[i]=0;
    for (int i = 0; i < 2; ++i) TensionB[i]=0;
    for (int i = 0; i < 2; ++i) TensionC[i]=0;
    for (int i = 0; i < 3; ++i) Creep_Time[i]=0;
    // Dmd_Reset_Count[0]=0;
    // Reverse_Count[0]=0;
    // Meter_Reset_Count[0]=0;
    for (int i = 0; i < 16; ++i) MedidorID[i]=0;
    // for (int i = 0; i < 3; ++i) Dmd_Prev[i]=0;
    // for (int i = 0; i < 3; ++i) Dmd_Acum[i]=0;
    // for (int i = 0; i < 3; ++i) Dmd_Max[i]=0;
  }
};
strInfo strData;

void Elster_start( void );
void Elster_stop( void );
void IRAM_ATTR detectaIrda( void );
void printData();
