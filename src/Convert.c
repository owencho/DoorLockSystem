#include "Convert.h"
#include <stdio.h>



char * getSolenoidTurnString(OnOff state){
  switch(state){
    case ON:
        return "ON";
        break;
    case OFF:
        return "OFF";
        break;
    default : return NULL;
  }
}

char * getBeepActionString(StartStop action){
  switch(action){
    case START:
        return "START";
        break;
    case STOP:
        return "STOP";
        break;
    default : return NULL;
  }
}
