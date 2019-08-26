#include <stdio.h>
#include "unity.h"
#include "Door.h"
#include "OnOff.h"
#include "Event.h"
#include "DoorLock.h"
#include "Convert.h"
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
    testFailMessage("getTime() called too many time %d than expected %d"
                    ,NumCalls,numberOfTimelines);
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

// it will lock the door and stop beep when it reach INIT_STATE
void test_Door_init_state(void){
    Event evt = {DOOR_INIT_EVENT,NULL};
    DoorInfo doorInfo;
    uint32_t timeline[] ={
    0,          // time generated for system to check the time
    };
    OnOff solenoidAction[]={
    ON ,-1        // Solenoid will remain ON as init state turn on the solenoid
    };
    StartStop beepAction[]={
    STOP,-1      // stop beeping when reached init state
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // init state
    // Expected DOOR_CLOSED_AND_LOCKED_STATE after init state
    handleDoor(&evt,&doorInfo);
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(0,doorInfo.time);
}

// this code test the handleDoor module when it detected as INVALID DOOR access
// this module will start to beep when invalid door access event detected
// the beep will remain for 3 second only and then will return to DOOR_CLOSED_AND_LOCKED_STATE
void test_Door_invalid_access_and_beep(void){
    Event evt = {INVALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo;
    uint32_t timeline[] ={
    0,1,2,3,4,5,           // time generated for system to check the time
    };
    OnOff solenoidAction[]={
    ON ,-1                 // Solenoid will remain ON as invalid access detected
    };                     // invalid access wouldnt turn off the solenoid
    StartStop beepAction[]={
    START,STOP,-1          // beep START when invalid access event detected
};                     // beep STOP after 3 second of beeping at DOOR_CLOSED_AND_LOCKED_BEEP_STATE
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

// this test module is used to test the door system received a VALID_DOOR_ACCESS_EVENT
// and user opened the door for more than 15 sec
// the system will beep after the door opened for more than 15 second
// the system only stop the beep and lock the door after the user close the door
void test_Door_validaccess_and_open_until_door_beep(void){
    Event evt = {VALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo;
    uint32_t timeline[] ={
    0,10,10,16,16                   // time generated for system to check the time
    };
    OnOff solenoidAction[]={
    OFF ,ON,-1                      // Solenoid become OFF at first as valid access detected
    };                              // then ON due to the door locked after door closed
    StartStop beepAction[]={
    START ,STOP,-1                 // beep become START when user opened door for more than 15 second
};                                 // beep become STOP after user closed the door
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
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
    TEST_ASSERT_EQUAL(16,doorInfo.time);
}


// this test module is used to test the door system received a VALID_DOOR_ACCESS_EVENT
// and user  opened the door and close within 15 second which wouldnt beep
// the system only lock the door after the door was closed
void test_Door_validaccess_and_open_then_close(void){
    Event evt = {VALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo;
    uint32_t timeline[] ={
    0,9,10,12             // time generated for system to check the time
    };
    OnOff solenoidAction[]={
    OFF ,ON,-1            // Solenoid become OFF at first as valid access detected
    };                    // then ON due to the door locked after door closed
    StartStop beepAction[]={
    STOP  ,-1             // beep is STOP as no event trigger it to beep
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // in closed and locked state with valid access card detected
    // Expected DOOR_CLOSED_AND_UNLOCKED_STATE
    handleDoor(&evt,&doorInfo);
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(0,doorInfo.time);
    evt.type = DOOR_OPENED_EVENT;
    // door detected open
    // Expected DOOR_OPENED_STATE and 9 sec on timing
    handleDoor(&evt,&doorInfo);
    TEST_ASSERT_EQUAL(DOOR_OPENED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(9,doorInfo.time);
    handleDoor(&evt,&doorInfo); //10 sec detected so no beep
    TEST_ASSERT_EQUAL(10,doorInfo.time);
    evt.type = DOOR_CLOSED_EVENT;   // the door is closed
    handleDoor(&evt,&doorInfo); //DOOR CLOSED and 12 second timer detected
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(12,doorInfo.time);
}

// this test module is used to test the door system received a VALID_DOOR_ACCESS_EVENT
// and user did not opened the door
// the system only lock the door after 10 second when user doesnt opened the door
void test_Door_validaccess_and_did_not_opened_wait_10_sec_stop(void){
    Event evt = {VALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo;
    uint32_t timeline[] ={
    0,2,3,17            // time generated for system to check the time
    };
    OnOff solenoidAction[]={
    OFF ,ON,-1          // Solenoid become OFF at first as valid access detected
    };                  // then ON due to the door locked after 10 second
    StartStop beepAction[]={
    STOP ,-1           // beep is STOP as no event trigger it to beep
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // in closed and locked state with valid access card detected
    // Expected DOOR_CLOSED_AND_UNLOCKED_STATE
    handleDoor(&evt,&doorInfo); // 0 sec detected
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(0,doorInfo.time);
    handleDoor(&evt,&doorInfo); // 2 sec detected
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(2,doorInfo.time);
    handleDoor(&evt,&doorInfo); // 3 sec detected
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(3,doorInfo.time);
    handleDoor(&evt,&doorInfo); // 17 sec detected & DOOR locked & returned to DOOR_CLOSED_AND_LOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(17,doorInfo.time);
    evt.type = IDLE_EVENT;
    handleDoor(&evt,&doorInfo); // check is system still remain on DOOR_CLOSED_AND_LOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_STATE,doorInfo.state);
}
