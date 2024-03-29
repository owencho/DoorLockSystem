#ifndef _DOOR_H
#define _DOOR_H
#include <stdint.h>
#include "OnOff.h"
#include "Event.h"


typedef struct DoorInfo DoorInfo ;

typedef enum {
    DOOR_INIT_STATE,
    DOOR_CLOSED_AND_LOCKED_STATE,
    DOOR_CLOSED_AND_LOCKED_BEEP_STATE,
    DOOR_CLOSED_AND_UNLOCKED_STATE,
    DOOR_OPENED_STATE,
} DoorState ;

struct DoorInfo{
    DoorState state;
    uint32_t time;
    uint32_t timeDiff;
    uint32_t previousTime;
};

void handleDoor(Event *evt,DoorInfo * dInfo);



#endif // _DOOR_H
