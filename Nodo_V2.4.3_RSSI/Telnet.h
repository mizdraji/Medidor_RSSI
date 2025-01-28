#include "ESPTelnet.h"
ESPTelnet telnet;
IPAddress ip;
bool isConnected();
bool connectToWiFi(const char* ssid, const char* password, int max_tries = 200, int pause = 500);
void errorMsg(String error, bool restart = true);
void setupTelnet();
