#include "DoorLock.h"
#include "Solenoid.h"
#include "OnOff.h"

void lockDoor(void){
    solenoidTurn(ON);
}

void unlockDoor(void){
    solenoidTurn(OFF);
}
