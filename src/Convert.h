#ifndef _CONVERT_H
#define _CONVERT_H

#include "OnOff.h"
#include "Event.h"
char * getSolenoidTurnString(OnOff state);
char * getBeepActionString(StartStop action);

#endif // _CONVERT_H
