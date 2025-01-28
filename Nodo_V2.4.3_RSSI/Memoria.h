/*
-  Manejo de memoria EEPROM (Flash)

*/
#include <EEPROM.h>
#include "payloads.h" 
#define EEPROM_SIZE 3072
byte cantSlot;       //Cantidad de slots guardados en memoria
#define SLOTS_CANTIDAD_MAX 16 //Cantidad maxima de payloads que se pueden almacenar
#define CMT_CANTIDAD_MAX 100 //Cantidad maxima de marcas de tiempo que se pueden almacenar
int pos_mem[SLOTS_CANTIDAD_MAX]={0};    //Array donde se guardan las posiciones de memoria de los slots almacenados
const int pos_modo_wifi=(SLOTS_CANTIDAD_MAX*(PL_CONT_MONO_SIZE+2))+(CMT_CANTIDAD_MAX*5); //2724 direccion del byte para conectar o no al wifi al inicio (1 byte)
const int pos_bit_act=pos_modo_wifi+1; //2725 bit de activacion (1 byte) 
const int pos_devID=pos_bit_act+1;	//2726 devID (4 + 1 bytes) 
const int pos_devAddr=pos_devID+5;	//2731	(8+1 bytes)
const int pos_reset_count=pos_devAddr+9; //2740 contador de reseteos (1 byte)
const int pos_stdby=pos_reset_count+1; //2741 byte stdby (1 byte)
byte slot_actual=0;	//Numero de bloque de memoria (0-15) donde se almacenan las payloads
int dir_actual=0;	//Dirección de memoria donde se almacena la payload actual
int dir_CMt_actual=0;	//Direccion de memoria donde se almacena la marca de tiempo
unsigned long CMt_corte=0; //Ultima marca de tiempo registrada antes de que se reinicie el dispositivo
bool cambio_bloque_PL=1; //0 si no cambio la memoria en el bloque de payloads, 1 si cambio algo
byte slot_para_enviar=0; //Numero de bloque de memoria (0-15) donde se almacena el paquete proximo a ser enviado

void initEEPROM();
void updateEEPROM(int addr, byte dato_in);
int writePLToEEPROM(int addrOffset, char data_in_[], byte length_);
int writeCMtToEEPROM(int addrOffset, char data_in_[], byte length_);
byte readPLFromEEPROM(int addrOffset, char * data_out_);
void readCmtFromEEPROM(int addrOffset, char * data_out_);
void overWriteEEPROM(int addr, char data_in_[], byte length_);
byte leerPLGuardadas();
int leerCMtGuardados();
bool sePuedeEscribirSlot(int addr);
bool memoriaLlena();
void marcarPaquete(int addr, byte marca);
void checkDir();
int dirParaEnviar();
void modoWifi(bool in);
void clearEEPROM();
void writeCredToEEPROM(unsigned long devID_in, char devAddr_in[]);
byte readCredFromEEPROM();
void writeResetCountToEEPROM(byte reset_count_);
void writeStdby(bool in);
byte lastPL();
