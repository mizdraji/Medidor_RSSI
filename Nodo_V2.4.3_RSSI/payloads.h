#include "MacroIrda.h"

//Tipos de medidor
#define A1052 0
#define A150 1
#define A102 2
#define A103 3

//Longitudes
#define PAYLOADST_SIZE 23 //INICIO V2: 23 bytes
#define PAYLOADID_SIZE 23 //Identificacion V2: 23 bytes
#define PL_CONT_MONO_SIZE 137 //Payload de contadores monofasicos V2: 107 Bytes
//#define B2C_MONO_SIZE 7 //Bloque 2 de la cabecera de la payload monofasico.
#define PL_CONT_TRI_SIZE 147 //Payload de contadores trifasicos V2: 113 Bytes
#define PL_ACT_SIZE 7 //Payload de activacion automatica (V2.3): 7 bytes

//Comandos
#define CMD_SYNC 0x01 //1 (DEC)
#define CMD_SYNC_V2 0x08 //8 (DEC)
#define CMD_IDENT_V2 0x09 //9 (DEC)
#define CMD_LECT_MONO_V2 0x0A //10 (DEC)
#define CMD_LECT_TRI_V2 0x0B //11 (DEC)
#define CMD_MODO_WIFI_ON 0x0C //12 (DEC)
#define CMD_MODO_WIFI_OFF 0x0D //13 (DEC)
#define CMD_CLEAR 0x0E //14 (DEC)
#define CMD_RESET 0x0F //15 (DEC)
//#define CMD_LECT_MONO_V2 0X10 //16 (DEC) (primeros 7 nodos)
#define CMD_TAMPER_ACT 0x11 //17 (DEC)
#define CMD_TAMPER_DES 0x12 //18 (DEC)
#define CMD_STDBY_ON 0x13 //19 (DEC) comando Modo Stdby On
#define CMD_STDBY_OFF 0x14 //20 (DEC) comando Modo Stdby Off
#define CMD_LAST_PKT 0x15 //21 (DEC) comando Enviar último paquete
#define CMD_SPEC_PKT 0x16 //22 (DEC) comando Enviar Paquete especifico
#define CMD_RE_SYNC 0x17 //23 (DEC) comando Ajuste de hora forzado
#define CMD_RE_ACTIV 0x24 //24 (DEC)

#define CMD_FRED 0x1E   //30 dec  comando falla red
#define CMD_FSYNC 0x1F  //31 dec  comando falla sincronizacion
#define CMD_FIDEN 0x20  //32 dec  comando falla identificacion
#define CMD_FACT 0x21   //33 dec  comando falla activacion

#define CMD_ACTIVACION 0x63 //99 (DEC)



//Arrays
char CMtd[1]={60}; //60 minutos entre dos lecturas sucesivas
char CMD[1] = {0};
char payloadST[PAYLOADST_SIZE] = {0};
char payloadID[PAYLOADID_SIZE] = {0};
char payloadACT[PL_ACT_SIZE] = {0};
char payloadContadores[PL_CONT_MONO_SIZE] = {0}; //Aca se almacena la carga util del paquete que se envia periodicamente

//uint8_t lect_med_ok = 0;
bool flag_lectura_med = 0;

bool muestrearMedidor( byte tipoPayload );
byte loadPayloadContadoresMono(strInfo strData, byte ciclo_, int dir);
void loadPayloadST(char payloadGuardada_[], unsigned long CMtCorte_[]);
unsigned long BCDtoDEC(char val_[], byte len_);
