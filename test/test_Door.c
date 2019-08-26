#include <stdio.h>
#include "unity.h"
#include "Door.h"
#include "OnOff.h"
#include "Event.h"
#include "DoorLock.h"
#include "CustomUnityMsg.h"
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
    TEST_FAIL_MESSAGE("getTime() called too many time than expected ");
    return 0;
}

void fake_solenoidTurn(OnOff state,int NumCalls){
  char* generatedState = NULL;
  char* expectedState = NULL;
    if(solenoidScript[NumCalls]==-1)
        TEST_FAIL_MESSAGE("solenoidTurn() called too many time than expected ");

    if(state == ON){
      printf("Door is locked \n");
    }

    else{
      printf("Door is unlocked \n");
    }

    if(solenoidScript[NumCalls] != state){
      generatedState = getSolenoidTurnString(state);
      expectedState = getSolenoidTurnString(solenoidScript[NumCalls]);
      testFailMessage("Solenoid state is incorrect at %d ,expected %s but detected %s"
                      ,NumCalls,expectedState,generatedState);
    }

}

void fake_beeping(StartStop action,int NumCalls){
    char* generatedAction = NULL;
    char* expectedAction = NULL;
    if(beepingScript[NumCalls]==-1)
        TEST_FAIL_MESSAGE("beeping() called too many time than expected ");

    if(action == START){
        printf("beep... \n");
    }
    else{
      printf(" No beep \n");
    }

    if(beepingScript[NumCalls] != action){
      generatedAction = getBeepActionString(action);
      expectedAction = getBeepActionString(beepingScript[NumCalls]);
      testFailMessage("beeping action is incorrect at index %d ,expected %s action but detected %s action"
                      ,NumCalls,expectedAction,generatedAction);
    }

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

void test_Door_init_state(void){
    Event evt = {DOOR_INIT_EVENT,NULL};
    DoorInfo doorInfo;
    uint32_t timeline[] ={
    0,1,          // time generated for system to check the time
    };
    OnOff solenoidAction[]={
    ON ,-1                 // Solenoid will remain ON as init state turn on the solenoid
    };
    StartStop beepAction[]={
    STOP,-1      // stop beeping when reached init state
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // init state
    // Expected DOOR_CLOSED_AND_LOCKED_STATE
    handleDoor(&evt,&doorInfo);
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(0,doorInfo.time);
}

// this code test the handleDoor module when it detected as INVALID DOOR access
// this module will start to beep when invalid door access event detected
// the beep will remain for 3 second only
void test_Door_invalid_access_and_beep(void){
    Event evt = {INVALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo;
    uint32_t timeline[] ={
    0,1,2,3,4,5,           // time generated for system to check the time
    };
    OnOff solenoidAction[]={
    ON ,-1                 // Solenoid will remain ON as invalid access detected
    };                      // invalid access wouldnt turn off the solenoid
    StartStop beepAction[]={
    START,STOP,-1
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // in closed and locked state with invalid access card detected
    // Expected DOOR_CLOSED_AND_LOCKED_BEEP_STATE
    handleDoor(&evt,&doorInfo); //invalid access detected , start beep
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_BEEP_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(0,doorInfo.time);
    handleDoor(&evt,&doorInfo); //1 sec detected , keep beeping
    TEST_ASSERT_EQUAL(1,doorInfo.time);
    handleDoor(&evt,&doorInfo); //2 sec detected , keep beeping
    TEST_ASSERT_EQUAL(2,doorInfo.time);
    handleDoor(&evt,&doorInfo); //3 sec detected , keep beeping
    TEST_ASSERT_EQUAL(3,doorInfo.time);
    handleDoor(&evt,&doorInfo); //4 sec detected , stop beeping
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_BEEP_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(4,doorInfo.time);
    evt.type = IDLE_EVENT;
    handleDoor(&evt,&doorInfo); //returned to DOOR_CLOSED_AND_LOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_STATE,doorInfo.state);
}

void test_Door_validaccess_and_open_until_door_beep(void){
    Event evt = {DOOR_CLOSED_AND_LOCKED_STATE,NULL};
    DoorInfo doorInfo;
    uint32_t timeline[] ={
    0,10,10,16,16
    };
    OnOff solenoidAction[]={
    OFF ,ON,-1
    };
    StartStop beepAction[]={
    START ,STOP,-1
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    handleDoor(&evt,&doorInfo); // init the door system
    evt.type = VALID_DOOR_ACCESS_EVENT;
    // in closed and locked state with valid access card detected
    // Expected DOOR_CLOSED_AND_UNLOCKED_STATE
    handleDoor(&evt,&doorInfo);
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(0,doorInfo.time);
    evt.type = DOOR_OPENED_EVENT;
    // door detected open
    // Expected DOOR_OPENED_STATE
    handleDoor(&evt,&doorInfo);
    TEST_ASSERT_EQUAL(DOOR_OPENED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(10,doorInfo.time);
    handleDoor(&evt,&doorInfo); //10 sec detected so no beep
    TEST_ASSERT_EQUAL(10,doorInfo.time);
    handleDoor(&evt,&doorInfo); //16 sec detected and the module beep
    TEST_ASSERT_EQUAL(16,doorInfo.time);
    evt.type = DOOR_CLOSED_EVENT;
    handleDoor(&evt,&doorInfo); //DOOR CLOSED and beep stop
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_STATE,doorInfo.state);
}

void test_Door_validaccess_and_open_then_close(void){
    Event evt = {VALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo;
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
    handleDoor(&evt,&doorInfo);
    evt.type = DOOR_OPENED_EVENT;
    // door detected open
    // Expected DOOR_OPENED_STATE
    handleDoor(&evt,&doorInfo);
    handleDoor(&evt,&doorInfo); //10 sec detected so no beep
    evt.type = DOOR_CLOSED_EVENT;
    handleDoor(&evt,&doorInfo); //DOOR CLOSED
}

void test_Door_validaccess_and_did_not_opened_wait_10_sec_stop(void){
    Event evt = {VALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo;
    uint32_t timeline[] ={
    0,2,3,17
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
    handleDoor(&evt,&doorInfo); // 0 sec detected
    handleDoor(&evt,&doorInfo); // 2 sec detected
    handleDoor(&evt,&doorInfo); // 3 sec detected
    handleDoor(&evt,&doorInfo); // 17 sec detected & DOOR locked
    evt.type = IDLE_EVENT;
    handleDoor(&evt,&doorInfo); // returned to DOOR_CLOSED_AND_LOCKED_STATE
}
