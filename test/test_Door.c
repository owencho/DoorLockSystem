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

void test_Door_validaccess_and_open_until_door_beep(void){
    Event evt = {DOOR_INIT_EVENT,NULL};
    uint32_t timeline[] ={
    0,10,10,16,16,17,50,60,100,120
    };
    OnOff solenoidAction[]={
    ON , OFF ,ON,-1
    };
    StartStop beepAction[]={
    STOP , START ,STOP,-1
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    handleDoor(&evt);                   // init the door system
    evt.type = VALID_DOOR_ACCESS_EVENT;
    // in closed and locked state with valid access card detected
    // Expected DOOR_CLOSED_AND_UNLOCKED_STATE
    handleDoor(&evt);
    evt.type = DOOR_OPENED_EVENT;
    // door detected open
    // Expected DOOR_OPENED_STATE
    handleDoor(&evt);
    handleDoor(&evt); //10 sec detected so no beep
    handleDoor(&evt); //16 sec detected and the module beep
    evt.type = DOOR_CLOSED_EVENT;
    handleDoor(&evt); //DOOR CLOSED and beep stop
}

void test_Door_validaccess_and_open_then_close(void){
    Event evt = {VALID_DOOR_ACCESS_EVENT,NULL};
    uint32_t timeline[] ={
    0,10,10,16,16,17,50,60,100,120
    };
    OnOff solenoidAction[]={
    OFF ,ON,-1
    };
    StartStop beepAction[]={
    STOP , START ,STOP,-1
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // in closed and locked state with valid access card detected
    // Expected DOOR_CLOSED_AND_UNLOCKED_STATE
    handleDoor(&evt);
    evt.type = DOOR_OPENED_EVENT;
    // door detected open
    // Expected DOOR_OPENED_STATE
    handleDoor(&evt);
    handleDoor(&evt); //10 sec detected so no beep
    evt.type = DOOR_CLOSED_EVENT;
    handleDoor(&evt); //DOOR CLOSED
}

void test_Door_validaccess_and_did_not_opened_wait_10_sec_stop(void){
    Event evt = {VALID_DOOR_ACCESS_EVENT,NULL};
    uint32_t timeline[] ={
    0,2,3,17,50,60,100,120
    };
    OnOff solenoidAction[]={
    OFF ,ON,-1
    };
    StartStop beepAction[]={
    STOP , START ,STOP,-1
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // in closed and locked state with valid access card detected
    // Expected DOOR_CLOSED_AND_UNLOCKED_STATE
    handleDoor(&evt); // 0 sec detected
    handleDoor(&evt); // 2 sec detected
    handleDoor(&evt); // 3 sec detected
    handleDoor(&evt); // 17 sec detected & DOOR locked
    handleDoor(&evt); // returned to DOOR_CLOSED_AND_LOCKED_STATE
}

void test_Door_invalid_access_and_beep(void){
    Event evt2 = {INVALID_DOOR_ACCESS_EVENT,NULL};
    uint32_t timeline[] ={
    0,1,2,3,4,5,
    };
    OnOff solenoidAction[]={
    ON ,-1
    };
    StartStop beepAction[]={
     START ,STOP,-1
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // in closed and locked state with invalid access card detected
    // Expected DOOR_CLOSED_AND_LOCKED_BEEP_STATE
    handleDoor(&evt2); //start beeping
    handleDoor(&evt2); //1 sec detected , keep beeping
    handleDoor(&evt2); //2 sec detected , keep beeping
    handleDoor(&evt2); //3 sec detected , keep beeping
    handleDoor(&evt2); //4 sec detected , stop beeping
    evt2.type = IDLE_EVENT;
    handleDoor(&evt2); //returned to DOOR_CLOSED_AND_LOCKED_STATE
}
