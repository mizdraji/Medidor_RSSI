//ABP Credentials

unsigned int devID_AA=0;   //devID para Activacion Automatica
char devAddr_AA[9] = "0118e299"; //devAddr para Activacion Automatica

//Activacion
#define activacion_manual 0 // 1: manual, 0: automati


#if activacion_manual == 0
  const char *nwkSKey = "d8e5a8e01bddfaf48e059992119be745";
  const char *appSKey = "3f19e1ef0da5286e178c5b84fc9e23ae";

//////////////////////////////////////////////////////////

#elif activacion_manual == 1

  #define devID 1

  #if devID == 0 //
  const char *devAddr = "009b7852";
  const char *nwkSKey = "07e7e68ce894f662d84ed54d0c0c23fa";
  const char *appSKey = "db62bbff43b4bf4f0f73e707076a14b5";
  #endif

  #if devID == 1 //
  const char *devAddr = "01d3d25a";
  const char *nwkSKey = "e86a226f3d1091dd22e7885366c8f840";
  const char *appSKey = "e839e5710602d6a913da02a41fdb34ba";
  #endif
  
  #if devID == 3900 //USAR ESTE PARA PRUEBAS
  const char *devAddr = "0102150b";
  const char *nwkSKey = "1a0b4e5246e55ff17f405edc5dd8e65e";
  const char *appSKey = "ea520a0fac3c44a80d04ea5f4bfadfce";
  #endif

  #if devID == 13338 //
  const char *devAddr = "01bb7bbc";
  const char *nwkSKey = "07e7e68ce894f662d84ed54d0c0c23fa";
  const char *appSKey = "db62bbff43b4bf4f0f73e707076a14b5";
  #endif

#endif
