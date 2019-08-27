#include <stdio.h>
#include "Door.h"
#include "DoorLock.h"
#include "Beep.h"
#include "Timer.h"

DoorInfo doorInfo = {DOOR_INIT_STATE};



void handleDoor(Event *evt, DoorInfo * dInfo){
    switch(doorInfo.state){
        case DOOR_INIT_STATE:
            beeping(STOP);
            lockDoor();
            doorInfo.state = DOOR_CLOSED_AND_LOCKED_STATE;
            doorInfo.time = getTime();
            break;
        case DOOR_CLOSED_AND_LOCKED_STATE:
            if(evt-> type ==VALID_DOOR_ACCESS_EVENT){
                unlockDoor();
                doorInfo.time = getTime();
                doorInfo.state = DOOR_CLOSED_AND_UNLOCKED_STATE;
            }
            else if(evt-> type ==INVALID_DOOR_ACCESS_EVENT){
                beeping(START);
                doorInfo.state = DOOR_CLOSED_AND_LOCKED_BEEP_STATE;
                doorInfo.time = getTime();
            }
            break;
        case DOOR_CLOSED_AND_LOCKED_BEEP_STATE:
            if(evt-> type ==VALID_DOOR_ACCESS_EVENT){
                beeping(STOP);
                unlockDoor();
                doorInfo.state = DOOR_CLOSED_AND_UNLOCKED_STATE;
            }
            else if(doorInfo.time <= 3){
                doorInfo.time = getTime();
            }
            else{
                beeping(STOP);
                doorInfo.state = DOOR_CLOSED_AND_LOCKED_STATE;
            }
            break;
        case DOOR_CLOSED_AND_UNLOCKED_STATE:
            doorInfo.time = getTime();
            if(evt-> type ==DOOR_OPENED_EVENT){
                doorInfo.state = DOOR_OPENED_STATE;
            }
            else if (doorInfo.time > 10){
                doorInfo.state = DOOR_CLOSED_AND_LOCKED_STATE;
                lockDoor();
            }
            break;
        case DOOR_OPENED_STATE:
            doorInfo.time = getTime();
            if(evt-> type ==DOOR_CLOSED_EVENT){
                if(doorInfo.time > 15){
                    beeping(STOP);
                }
                lockDoor();
                doorInfo.state = DOOR_CLOSED_AND_LOCKED_STATE;
            }
            else if(doorInfo.time > 15){
                beeping(START);
                doorInfo.state = DOOR_OPENED_STATE;
            }
            break;
        default: doorInfo.state = DOOR_INIT_STATE;
    }
    *dInfo = doorInfo;
}
