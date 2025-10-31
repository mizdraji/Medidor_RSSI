const unsigned long interval = 15000;      // 15 s interval to send message
unsigned long previousMillis = 0;          // will store last time message sent
unsigned int counter = 0;                  // message counter
int rcv_count = 0;                // contador de mensajes recibidos
int send_count = 0;               // contador de mensajes enviados
int diferencia = 3;               //diferencia para comparar send_count y rcv_count

int rssi_rcv = 0;
int rssiValue=0;
char rssiSend[20]={0};                     //variable para guardar dato a enviar en forma de string
String get_name;                           //variable para guardar gatewayname

char datoEntrante[255]={0};                //variable para guardar dato recibido
char *token;                 

int recvStatus = 0;


void IRAM_ATTR onReceive();