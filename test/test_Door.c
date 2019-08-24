#include <stdio.h>
#include "unity.h"
#include "Door.h"
#include "OnOff.h"
#include "Event.h"
#include "DoorLock.h"
#include "mock_Timer.h"
#include "mock_Solenoid.h"
#include "mock_Beep.h"

uint32_t * timelinePtr;
int numberOfTimelines ;
OnOff * solenoidScript = NULL;
StartStop * beepingScript = NULL;


uint32_t fake_getTime(int NumCalls){
    if(NumCalls < numberOfTimelines){
        printf("Current time get is %d \n",timelinePtr[NumCalls]);
        return timelinePtr[NumCalls];
    }
    TEST_FAIL_MESSAGE("getTime() called too many time than expected \n");
    return 0;
}

void fake_solenoidTurn(OnOff state,int NumCalls){
    if(solenoidScript[NumCalls]==-1)
        TEST_FAIL_MESSAGE("beeping() called too many time than expected \n");

    if(state == ON)
        printf("Door is locked \n");
    else
        printf("Door is unlocked \n");
    if(solenoidScript[NumCalls] != state)
        TEST_FAIL_MESSAGE("Solenoid state is incorrect");
}

void fake_beeping(StartStop action,int NumCalls){
    if(beepingScript[NumCalls]==-1)
        TEST_FAIL_MESSAGE("beeping() called too many time than expected \n");

    if(action == START)
        printf("beep... \n");
    else
        printf(" No beep \n");
    if(beepingScript[NumCalls] != action)
        TEST_FAIL_MESSAGE("beep action is incorrect");
}

void initTimerAndLowLevelHardware(uint32_t timeline[],int timelineNumber,
                                  OnOff * solenoidActionPtr,
                                  StartStop * beepActionPtr){

    timelinePtr = timeline;
    numberOfTimelines = timelineNumber;
    solenoidScript = solenoidActionPtr;
    beepingScript = beepActionPtr;
    getTime_StubWithCallback(fake_getTime);
    beeping_StubWithCallback(fake_beeping);
    solenoidTurn_StubWithCallback(fake_solenoidTurn);
}

void setUp(void){}
void tearDown(void){}

void test_Door_try(void){
    Event evt = {DOOR_INIT_EVENT,NULL};
    uint32_t timeline[] ={
    0,10,10,16,17,50,60,100,120
    };
    OnOff solenoidAction[]={
    ON , OFF ,OFF ,-1
    };
    StartStop beepAction[]={
    STOP ,STOP, START ,STOP,-1
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    handleDoor(&evt);                   // init the door system
    evt.type = VALID_DOOR_ACCESS_EVENT;
    // in closed and locked state with valid access card detected
    //
    handleDoor(&evt);
    evt.type = DOOR_OPENED_EVENT;
    handleDoor(&evt);
    handleDoor(&evt);
    handleDoor(&evt);
    evt.type = DOOR_CLOSED_EVENT;
    handleDoor(&evt);
}
