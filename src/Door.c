#include <stdio.h>
#include "Door.h"
#include "DoorLock.h"
#include "Beep.h"

DoorInfo doorInfo = {DOOR_INIT_STATE};

void handleDoor(Event *evt){
    switch(doorInfo.state){
        case DOOR_INIT_STATE:
            if(evt-> type == DOOR_INIT_EVENT){
                printf("Current state is DOOR_INIT_STATE \n");
                beeping(STOP);
                lockDoor();
                doorInfo.state = DOOR_CLOSED_AND_LOCKED_STATE;
                doorInfo.time = getTime();
            }
            else{
                doorInfo.state = DOOR_INIT_STATE;
            }
            break;
        case DOOR_CLOSED_AND_LOCKED_STATE:
            printf("Current state is DOOR_CLOSED_AND_LOCKED_STATE \n");
            if(evt-> type ==VALID_DOOR_ACCESS_EVENT){
                beeping(STOP);
                unlockDoor();
                doorInfo.state = DOOR_CLOSED_AND_UNLOCKED_STATE;
            }
            else if(evt-> type ==INVALID_DOOR_ACCESS_EVENT){
                beeping(START);
                doorInfo.state = DOOR_CLOSED_AND_LOCKED_BEEP_STATE;
                doorInfo.time = getTime();
            }
            else{
                doorInfo.state = DOOR_CLOSED_AND_LOCKED_STATE;
            }
            break;
        case DOOR_CLOSED_AND_LOCKED_BEEP_STATE:
            printf("Current state is DOOR_CLOSED_AND_LOCKED_BEEP_STATE \n");
            if(doorInfo.time < 3){
                doorInfo.time = getTime();
                beeping(START);
            }
            else{
                beeping(STOP);
                doorInfo.state = DOOR_CLOSED_AND_LOCKED_STATE;
            }
            break;
        case DOOR_CLOSED_AND_UNLOCKED_STATE:
            printf("Current state is DOOR_CLOSED_AND_UNLOCKED_STATE \n");
            if(evt-> type ==DOOR_OPENED_EVENT){
                doorInfo.state = DOOR_OPENED_STATE;
                doorInfo.time = getTime();
            }
            else{
                doorInfo.state = DOOR_CLOSED_AND_LOCKED_STATE;
            }
            break;
        case DOOR_OPENED_STATE:
            printf("Current state is DOOR_OPENED_STATE \n");
            doorInfo.time = getTime();
            if(evt-> type ==DOOR_CLOSED_EVENT){
                beeping(STOP);
                doorInfo.state = DOOR_CLOSED_AND_LOCKED_STATE;
            }
            else if(doorInfo.time > 15){
                beeping(START);
                doorInfo.state = DOOR_OPENED_STATE;
            }
            else{
                doorInfo.state = DOOR_OPENED_STATE;
            }
            break;
    }
}
