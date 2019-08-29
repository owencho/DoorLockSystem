#include <stdio.h>
#include "Door.h"
#include "DoorLock.h"
#include "Beep.h"
#include "Timer.h"

void handleDoor(Event *evt, DoorInfo * doorInfo){
    switch(doorInfo->state){
        case DOOR_INIT_STATE:
            beeping(STOP);
            lockDoor();
            doorInfo->state = DOOR_CLOSED_AND_LOCKED_STATE;
            break;
        case DOOR_CLOSED_AND_LOCKED_STATE:
            if(evt-> type ==VALID_DOOR_ACCESS_EVENT){
                unlockDoor();
                doorInfo->time = getTime();
                doorInfo->previousTime = doorInfo->time;
                doorInfo->state = DOOR_CLOSED_AND_UNLOCKED_STATE;
            }
            else if(evt-> type ==INVALID_DOOR_ACCESS_EVENT){
                beeping(START);
                doorInfo->state = DOOR_CLOSED_AND_LOCKED_BEEP_STATE;
                doorInfo->time = getTime();
                doorInfo->previousTime = doorInfo->time;
            }
            break;
        case DOOR_CLOSED_AND_LOCKED_BEEP_STATE:
            doorInfo->time = getTime();
            doorInfo->timeDiff =(doorInfo->time - doorInfo->previousTime);
            if(evt-> type ==VALID_DOOR_ACCESS_EVENT){
                beeping(STOP);
                unlockDoor();
                doorInfo->previousTime = doorInfo->time;
                doorInfo->state = DOOR_CLOSED_AND_UNLOCKED_STATE;
            }
            else if(doorInfo->timeDiff > 3){
                beeping(STOP);
                doorInfo->state = DOOR_CLOSED_AND_LOCKED_STATE;
            }
            break;
        case DOOR_CLOSED_AND_UNLOCKED_STATE:
            doorInfo->time = getTime();
            doorInfo->timeDiff =(doorInfo->time - doorInfo->previousTime);
            if(evt-> type ==DOOR_OPENED_EVENT){
                doorInfo->state = DOOR_OPENED_STATE;
                doorInfo->previousTime = doorInfo->time;
            }
            else if (doorInfo->timeDiff > 10){
                doorInfo->state = DOOR_CLOSED_AND_LOCKED_STATE;
                lockDoor();
            }
            break;
        case DOOR_OPENED_STATE:
            doorInfo->time = getTime();
            doorInfo->timeDiff =(doorInfo->time - doorInfo->previousTime);
            if(evt-> type ==DOOR_CLOSED_EVENT){
                beeping(STOP);
                lockDoor();
                doorInfo->state = DOOR_CLOSED_AND_LOCKED_STATE;
            }
            else if(doorInfo->timeDiff > 15){
                beeping(START);
            }
            break;
        default: doorInfo->state = DOOR_INIT_STATE;
    }
}
