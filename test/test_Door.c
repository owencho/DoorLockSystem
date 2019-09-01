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

/*
* getSolenoidTurnString is a function that is return char * of the destinated type
* which getSolenoidTurnString will return ON or off char* when either of them is detected on OnOff enum
* getBeepActionString is a function that is return char * of the destinated type
* which getBeepActionString will return START or STOP char* when either of them is detected on StartStop enum
* both of them will return NULL when it is not both STOP/START at getBeepActionString and
*  ON/OFF at getSolenoidTurnString
*/
char * getSolenoidTurnString(OnOff state){
  switch(state){
    case ON:
        return "ON";
        break;
    case OFF:
        return "OFF";
        break;
    default : return NULL;
  }
}

char * getBeepActionString(StartStop action){
  switch(action){
    case START:
        return "START";
        break;
    case STOP:
        return "STOP";
        break;
    default : return NULL;
  }
}
// this function is used to fake the getTime function
//it will print current time when numberOfTimelines larger than NumCalls
// it will generate error message when number of call more than the value inside the array
uint32_t fake_getTime(int NumCalls){
    if(NumCalls < numberOfTimelines){
        printf("Current time get is %d \n",timelinePtr[NumCalls]);
        return timelinePtr[NumCalls];
    }
    testFailMessage("getTime() called too many time %d than expected %d"
                    ,NumCalls,numberOfTimelines);
    return 0;
}
// this function is used to fake the solenoidTurn function
//it will print Door is locked or unlocked depends on the input is ON or off
// it will generate called too many time error when the solenoidScript pointed to -1 when testing
// it will also generate error when expected state are not equal to generated state
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
      generatedState = getSolenoidTurnString(state);  // function that get string depend on ON or OFF
      expectedState = getSolenoidTurnString(solenoidScript[NumCalls]);
      testFailMessage("Solenoid state is incorrect at %d ,expected %s but detected %s"
                      ,NumCalls,expectedState,generatedState);
    }

}
// this function is used to fake the beeping function
//it will printf beep or not beep depends on the input is START or STOP
// it will generate called too many time error when the beepingScript pointed to -1 when testing
// it will also generate error when expected action are not equal to generated action
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
// this function is used to initialize the fake_beeping ,fake_solenoidTurn and fake_getTime
// by taking in the timeline value and all the solenoidAction and beepAction array
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

//to initialize the DoorLock system
// it will lock the door and stop beep when it reach INIT_STATE
void test_Door_init_state(void){
    Event evt = {IDLE_EVENT,NULL};
    DoorInfo doorInfo = {DOOR_INIT_STATE};
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

    // Expected DOOR_CLOSED_AND_LOCKED_STATE after init state
    handleDoor(&evt,&doorInfo);
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(0,doorInfo.time);
}

// DOOR_CLOSED_AND_LOCKED_STATE remain on same state with other event that wont change state
// this module will start and remain on DOOR_CLOSED_AND_LOCKED_STATE
// this test shouldnt happened in real life and it tested to avoid if unexpected event happened
// refer to the state diagram(Resources\images\state diagram.png) for more information
void test_Door_DOOR_CLOSED_AND_LOCKED_STATE_remain_as_other_event_detected(void){
    Event evt = {DOOR_OPENED_EVENT,NULL};
    DoorInfo doorInfo = {DOOR_CLOSED_AND_LOCKED_STATE};
    uint32_t timeline[] ={
              // time generated for system empty as it doesnt call getTime for this test
    };
    OnOff solenoidAction[]={
    -1       //Array solenoidAction only have -1 as it doesnt call lockDoor and unlockDoor
    };
    StartStop beepAction[]={
    -1              //Array beepAction only have -1 as it doesnt call beeping() for this test
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    handleDoor(&evt,&doorInfo); //expect remain at DOOR_CLOSED_AND_LOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_STATE,doorInfo.state);
    evt.type = DOOR_CLOSED_EVENT; // door closed event
    handleDoor(&evt,&doorInfo); //expect remain at DOOR_CLOSED_AND_LOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_STATE,doorInfo.state);
    evt.type = IDLE_EVENT; // no event happened
    handleDoor(&evt,&doorInfo); //expect remain at DOOR_CLOSED_AND_LOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_STATE,doorInfo.state);
    handleDoor(&evt,&doorInfo); //expect remain at DOOR_CLOSED_AND_LOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_STATE,doorInfo.state);
}

// DOOR_CLOSED_AND_LOCKED_STATE to DOOR_CLOSED_AND_UNLOCKED_STATE with valid access event
// this code test the handleDoor module when it detected VALID DOOR access from
// DOOR_CLOSED_AND_LOCKED_STATE
// this module will unlock the door
// refer to the state diagram(Resources\images\state diagram.png) for more information
void test_Door_DOOR_CLOSED_AND_LOCKED_STATE_valid_access(void){
    Event evt = {VALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo = {DOOR_CLOSED_AND_LOCKED_STATE};
    uint32_t timeline[] ={
    0,1           // time generated for system to check the time
    };
    OnOff solenoidAction[]={
    OFF ,-1                 // Solenoid will OFF as valid access detected
    };
    StartStop beepAction[]={
    -1              //Array beepAction only have -1 as it doesnt call beeping()
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // in closed and locked state with valid access card detected
    // Expected DOOR_CLOSED_AND_UNLOCKED_STATE
    handleDoor(&evt,&doorInfo); //valid access detected , expected DOOR_CLOSED_AND_UNLOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(0,doorInfo.time);  //0 sec detected
    evt.type = IDLE_EVENT; // no event happened
    handleDoor(&evt,&doorInfo); //valid access detected , expected DOOR_CLOSED_AND_UNLOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(1,doorInfo.time);  //1 sec detected
    TEST_ASSERT_EQUAL(1,doorInfo.timeDiff); // expected 1 second difference from previous state
}


// DOOR_CLOSED_AND_LOCKED_STATE to DOOR_CLOSED_AND_LOCKED_BEEP_STATE with invalid access event
// this code test the handleDoor module when it detected INVALID DOOR access from
// DOOR_CLOSED_AND_LOCKED_STATE
// this module will go to  DOOR_CLOSED_AND_LOCKED_BEEP_STATE and beep
// refer to the state diagram(Resources\images\state diagram.png) for more information
void test_Door_DOOR_CLOSED_AND_LOCKED_STATE_invalid_access(void){
    Event evt = {INVALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo = {DOOR_CLOSED_AND_LOCKED_STATE};
    uint32_t timeline[] ={
    0,1,           // time generated for system to check the time
    };
    OnOff solenoidAction[]={
    ON ,-1                 // Solenoid will remain ON as invalid access detected
    };                     // invalid access wouldnt turn off the solenoid
    StartStop beepAction[]={
    START,-1          // beep START when invalid access event detected
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // in closed and locked state with invalid access card detected
    // Expected DOOR_CLOSED_AND_LOCKED_BEEP_STATE
    handleDoor(&evt,&doorInfo); //invalid access detected , start beep
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_BEEP_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(0,doorInfo.time);  //0 sec detected , keep beeping
    handleDoor(&evt,&doorInfo);
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_BEEP_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(1,doorInfo.time);  //0 sec detected , keep beeping
    TEST_ASSERT_EQUAL(1,doorInfo.timeDiff); // expected 1 second difference from previous state
    // check state is remain on DOOR_CLOSED_AND_LOCKED_BEEP_STATE
}

// DOOR_CLOSED_AND_LOCKED_BEEP_STATE beep 3 second and go back DOOR_CLOSED_AND_LOCKED_STATE
// this module will start to beep when invalid door access event detected
// the beep will remain for 3 second only and then will return to DOOR_CLOSED_AND_LOCKED_STATE
// refer to the state diagram(Resources\images\state diagram.png) for more information
void test_Door_DOOR_CLOSED_AND_LOCKED_BEEP_STATE_invalid_access_and_beep_3_second(void){
    Event evt = {INVALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo = {DOOR_CLOSED_AND_LOCKED_STATE};
    uint32_t timeline[] ={
    0,1,5,           // time generated for system to check the time
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
    TEST_ASSERT_EQUAL(0,doorInfo.time);  //0 sec detected , keep beeping
    handleDoor(&evt,&doorInfo);
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_BEEP_STATE,doorInfo.state);
    evt.type = IDLE_EVENT; // no event happened
     // expected in LOCK_BEEP_STATE
     //1 sec detected , keep beeping
    TEST_ASSERT_EQUAL(1,doorInfo.time);
    TEST_ASSERT_EQUAL(1,doorInfo.timeDiff); // expected 1 second difference from previous state
    handleDoor(&evt,&doorInfo);
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(5,doorInfo.time); //5 sec detected , stop beeping as 5 second past
    TEST_ASSERT_EQUAL(5,doorInfo.timeDiff); // expected 5 second difference from previous state
    handleDoor(&evt,&doorInfo); //expect remain at DOOR_CLOSED_AND_LOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_STATE,doorInfo.state);
}
//DOOR_CLOSED_AND_LOCKED_BEEP_STATE remain on same state with other event that wont change state
// refer to the state diagram(Resources\images\state diagram.png) for more information
void test_Door_DOOR_CLOSED_AND_LOCKED_BEEP_STATE_remain_with_other_event(void){
    Event evt = {INVALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo = {DOOR_CLOSED_AND_LOCKED_STATE};
    uint32_t timeline[] ={
    0,1,2,3                   // time generated for system to check the time
    };
    OnOff solenoidAction[]={
    ON,-1                  // Solenoid will remain ON as invalid access detected
    };

    StartStop beepAction[]={
    START,-1                // beep START when invalid access event detected
    };

    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // in closed and locked state with invalid access card detected
    // Expected DOOR_CLOSED_AND_LOCKED_BEEP_STATE
    handleDoor(&evt,&doorInfo); //invalid access detected , start beep
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_BEEP_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(0,doorInfo.time); // 0 second detected
    evt.type = DOOR_OPENED_EVENT;
    handleDoor(&evt,&doorInfo);
    //it will stay at DOOR_CLOSED_AND_LOCKED_STATE and continue beep
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_BEEP_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(1,doorInfo.time); // 1 second detected
    TEST_ASSERT_EQUAL(1,doorInfo.timeDiff); // expected 1 second difference from previous state
    evt.type = IDLE_EVENT;
    handleDoor(&evt,&doorInfo); // Expected remain DOOR_CLOSED_AND_LOCKED_BEEP_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_BEEP_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(2,doorInfo.time); // 2 second detected
    TEST_ASSERT_EQUAL(2,doorInfo.timeDiff); // expected 2 second difference from previous state
    evt.type = DOOR_CLOSED_EVENT;
    handleDoor(&evt,&doorInfo); // Expected remain DOOR_CLOSED_AND_LOCKED_BEEP_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_BEEP_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(3,doorInfo.time); // 3 second detected
    TEST_ASSERT_EQUAL(3,doorInfo.timeDiff); // expected 3 second difference from previous state
    // the beep only stop after 3 second difference from previous state
}

// DOOR_CLOSED_AND_LOCKED_BEEP_STATE to DOOR_CLOSED_AND_UNLOCKED_STATE as valid access detected
// this code test the handleDoor module when it detected as INVALID DOOR access
// this module will start to beep when invalid door access event detected
// after that detected VALID_DOOR_ACCESS_EVENT
// then it will go to DOOR_CLOSED_AND_UNLOCKED_STATE and stop beep
// refer to the state diagram(Resources\images\state diagram.png) for more information
void test_Door_DOOR_CLOSED_AND_LOCKED_BEEP_STATE_and_beep_and_valid_access_detected (void){
    Event evt = {INVALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo = {DOOR_CLOSED_AND_LOCKED_STATE};
    uint32_t timeline[] ={
    0,1,2                   // time generated for system to check the time
    };
    OnOff solenoidAction[]={
    OFF,-1                  //solenoid turn off as valid access detected
    };

    StartStop beepAction[]={
    START,STOP,-1                // beep START when invalid access event detected
    };                          // beep STOP after valid access detected

    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // in closed and locked state with invalid access card detected
    // Expected DOOR_CLOSED_AND_LOCKED_BEEP_STATE
    handleDoor(&evt,&doorInfo); //invalid access detected , start beep
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_BEEP_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(0,doorInfo.time); // 0 second detected
    evt.type = VALID_DOOR_ACCESS_EVENT;
    handleDoor(&evt,&doorInfo);
    //valid door access detected ,
    //it will go to DOOR_CLOSED_AND_UNLOCKED_STATE and stop beeping
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(1,doorInfo.time); // 1 second detected
    handleDoor(&evt,&doorInfo); //expect remain at DOOR_CLOSED_AND_UNLOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(2,doorInfo.time); // 2 second detected
}


//DOOR_CLOSED_AND_UNLOCKED_STATE remain on same state with other event that wont change state
// this code test the handleDoor module when it detected as other state
// this module will start and remain on DOOR_CLOSED_AND_UNLOCKED_STATE
// this test shouldnt happened in real life and it tested to avoid if unexpected event happened
// refer to the state diagram(Resources\images\state diagram.png) for more information
void test_Door_DOOR_CLOSED_AND_UNLOCKED_STATE_remain_as_other_event_detected(void){
    Event evt = {VALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo = {DOOR_CLOSED_AND_LOCKED_STATE};
    uint32_t timeline[] ={
    0,1,2,3,4                   // time generated for system to check the time
    };
    OnOff solenoidAction[]={
    OFF,-1                 // Solenoid will remain OFF as valid access detected
    };
    StartStop beepAction[]={
    -1              //Array beepAction empty as it doesnt call beeping()
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // in closed and locked state with valid access card detected
    // Expected DOOR_CLOSED_AND_UNLOCKED_STATE
    handleDoor(&evt,&doorInfo); //valid access detected
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    evt.type = IDLE_EVENT; // no event happened
    handleDoor(&evt,&doorInfo); //expect remain at DOOR_CLOSED_AND_LOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    evt.type = DOOR_CLOSED_EVENT; // closed door happened
    handleDoor(&evt,&doorInfo); //expect remain at DOOR_CLOSED_AND_LOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    evt.type = VALID_DOOR_ACCESS_EVENT; // valid access detected
    handleDoor(&evt,&doorInfo); //expect remain at DOOR_CLOSED_AND_LOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    evt.type = INVALID_DOOR_ACCESS_EVENT; // valid access detected
    handleDoor(&evt,&doorInfo); //expect remain at DOOR_CLOSED_AND_LOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(4,doorInfo.time); // 4 second detected
    TEST_ASSERT_EQUAL(4,doorInfo.timeDiff); // expected 4 second difference from previous state
    // the door only lock after 10 second difference from previous state
}

//DOOR_CLOSED_AND_UNLOCKED_STATE to DOOR_OPENED_STATE when door opened
// this code test the handleDoor module when it detected user opened door
// this module will unlock the door and test when user opened the door
// refer to the state diagram(Resources\images\state diagram.png) for more information
void test_Door_DOOR_CLOSED_AND_UNLOCKED_STATE_door_opened(void){
    Event evt = {VALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo = {DOOR_CLOSED_AND_LOCKED_STATE};
    uint32_t timeline[] ={
    0,1           // time generated for system to check the time
    };
    OnOff solenoidAction[]={
    OFF,-1                 // Solenoid will OFF as valid access detected
    };
    StartStop beepAction[]={
    -1              //Array beepAction empty as it doesnt call beeping()
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // in closed and locked state with valid access card detected
    // Expected DOOR_CLOSED_AND_UNLOCKED_STATE
    handleDoor(&evt,&doorInfo); //valid access detected , expected DOOR_CLOSED_AND_UNLOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(0,doorInfo.time);  //0 sec detected
    evt.type = DOOR_OPENED_EVENT; //  door opened happened
    handleDoor(&evt,&doorInfo); //expected DOOR_OPENED_STATE
    TEST_ASSERT_EQUAL(DOOR_OPENED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(1,doorInfo.time);  //1 sec detected
    TEST_ASSERT_EQUAL(1,doorInfo.timeDiff); // expected 1 second difference from previous state
}
//DOOR_CLOSED_AND_UNLOCKED_STATE to DOOR_CLOSED_AND_LOCKED_STATE after 10 second door without open
// this code test the handleDoor module when it detected user open door
// this module will unlock the door and test when user doesnt open the door
// and door will lock after 10 second
// refer to the state diagram(Resources\images\state diagram.png) for more information
void test_Door_DOOR_CLOSED_AND_UNLOCKED_STATE_door_doesnt_open_and_lock_after_10sec(void){
    Event evt = {VALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo = {DOOR_CLOSED_AND_LOCKED_STATE};
    uint32_t timeline[] ={
    0,1,11           // time generated for system to check the time
    };
    OnOff solenoidAction[]={
    OFF,ON,-1                 // Solenoid will OFF as valid access detected
    };                          // and turn on after door unlocked for 10 second
    StartStop beepAction[]={
    -1              //Array beepAction empty as it doesnt call beeping()
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // in closed and locked state with valid access card detected
    // Expected DOOR_CLOSED_AND_UNLOCKED_STATE
    handleDoor(&evt,&doorInfo); //valid access detected , expected DOOR_CLOSED_AND_UNLOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(0,doorInfo.time);  //0 sec detected
    evt.type = IDLE_EVENT; //  nothing happened
    handleDoor(&evt,&doorInfo); // expected DOOR_CLOSED_AND_UNLOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(1,doorInfo.time);  //1 sec detected
    TEST_ASSERT_EQUAL(1,doorInfo.timeDiff); // expected 1 second difference from previous state
    TEST_ASSERT_EQUAL(0,doorInfo.previousTime); // expected previous State time is 0
    handleDoor(&evt,&doorInfo); // expected DOOR_CLOSED_AND_LOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_STATE,doorInfo.state);
    //door expected locked as 10 second past
    TEST_ASSERT_EQUAL(11,doorInfo.time);  //11 sec detected
    TEST_ASSERT_EQUAL(11,doorInfo.timeDiff); // expected 11 second difference from previous state
}
//DOOR_OPENED_STATE remain on same state with other event that wont change state
// this code test the handleDoor module when it detected as other state
// this module will start and remain on DOOR_OPENED_STATE
// this test shouldnt happened in real life and it tested to avoid if unexpected event happened
// refer to the state diagram(Resources\images\state diagram.png) for more information
void test_Door_DOOR_OPENED_STATE_remain_as_other_event_detected(void){
    Event evt = {VALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo = {DOOR_CLOSED_AND_LOCKED_STATE};
    uint32_t timeline[] ={
    0,1,2,3,4                   // time generated for system to check the time
    };
    OnOff solenoidAction[]={
    OFF,-1                 // Solenoid will remain OFF as valid access detected
    };
    StartStop beepAction[]={
    -1              //Array beepAction empty as it doesnt call beeping()
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // in closed and locked state with valid access card detected
    // Expected DOOR_CLOSED_AND_UNLOCKED_STATE
    handleDoor(&evt,&doorInfo); //valid access detected
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(0,doorInfo.time);  //0 sec detected
    evt.type = DOOR_OPENED_EVENT; // DOOR_OPENED happened
    handleDoor(&evt,&doorInfo); //expect assigned to DOOR_OPENED_STATE
    TEST_ASSERT_EQUAL(DOOR_OPENED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(1,doorInfo.time);  //1 sec detected
    TEST_ASSERT_EQUAL(1,doorInfo.timeDiff); // expected 1 second difference from previous state
    TEST_ASSERT_EQUAL(1,doorInfo.previousTime); // expected previous State time is 1
    evt.type = IDLE_EVENT; //  nothing happened
    handleDoor(&evt,&doorInfo); //expect remain at DOOR_OPENED_STATE
    TEST_ASSERT_EQUAL(DOOR_OPENED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(2,doorInfo.time);  //2 sec detected
    TEST_ASSERT_EQUAL(1,doorInfo.timeDiff); // expected 1 second difference from previous state
    TEST_ASSERT_EQUAL(1,doorInfo.previousTime); // expected previous State time is 1
    evt.type = VALID_DOOR_ACCESS_EVENT; // valid access detected
    handleDoor(&evt,&doorInfo); //expect remain at DOOR_OPENED_STATE
    TEST_ASSERT_EQUAL(3,doorInfo.time);  //3 sec detected
    TEST_ASSERT_EQUAL(2,doorInfo.timeDiff); // expected 2 second difference from previous state
    TEST_ASSERT_EQUAL(1,doorInfo.previousTime); // expected previous State time is 1
    TEST_ASSERT_EQUAL(DOOR_OPENED_STATE,doorInfo.state);
    evt.type = INVALID_DOOR_ACCESS_EVENT; // valid access detected
    handleDoor(&evt,&doorInfo); //expect remain at DOOR_OPENED_STATE
    TEST_ASSERT_EQUAL(DOOR_OPENED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(4,doorInfo.time);  //4 sec detected
    TEST_ASSERT_EQUAL(3,doorInfo.timeDiff); // expected 3 second difference from previous state
    TEST_ASSERT_EQUAL(1,doorInfo.previousTime); // expected previous State time is 1
}
//DOOR_OPENED_STATE after 15 second beep
// this code test the handleDoor module when door opened for more than 15 second on
// DOOR_OPENED_STATE
// this module will beep after 15 second opening the door
// refer to the state diagram(Resources\images\state diagram.png) for more information
void test_Door_DOOR_OPENED_STATE_door_opened_more_than_15sec_and_beep(void){
    Event evt = {VALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo = {DOOR_CLOSED_AND_LOCKED_STATE};
    uint32_t timeline[] ={
    0,1,17           // time generated for system to check the time
    };
    OnOff solenoidAction[]={
     OFF,-1                 // Solenoid will OFF as valid access detected
    };
    StartStop beepAction[]={
    ON,-1              //beeping will on as door opened for more than 15 second
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // in closed and locked state with valid access card detected
    // Expected DOOR_CLOSED_AND_UNLOCKED_STATE
    handleDoor(&evt,&doorInfo); //valid access detected , expected DOOR_CLOSED_AND_UNLOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(0,doorInfo.time);  //0 sec detected
    evt.type = DOOR_OPENED_EVENT; //  door opened happened
    handleDoor(&evt,&doorInfo); //door open detected , expected assigned to DOOR_OPENED_STATE
    TEST_ASSERT_EQUAL(DOOR_OPENED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(1,doorInfo.time);  //1 sec detected
    TEST_ASSERT_EQUAL(1,doorInfo.timeDiff); // expected 1 second difference from previous state
    handleDoor(&evt,&doorInfo); // expected DOOR_OPENED_STATE
    // beep start as the door already opened for more than 15 second
    TEST_ASSERT_EQUAL(DOOR_OPENED_STATE,doorInfo.state); //remain DOOR_OPENED_STATE and beep
    TEST_ASSERT_EQUAL(17,doorInfo.time);  //17 sec detected
    TEST_ASSERT_EQUAL(16,doorInfo.timeDiff); // expected 16 second difference from previous state
    TEST_ASSERT_EQUAL(1,doorInfo.previousTime);
    // expected previous State time is 1 as the time is recorded when changing state from closed unlocked to opened
}
//DOOR_OPENED_STATE to DOOR_CLOSED_AND_LOCKED_STATE after door was closed
// this code test the handleDoor module when door opened then closed
// this module will lock the door after door was closed
// refer to the state diagram(Resources\images\state diagram.png) for more information
void test_Door_DOOR_OPENED_STATE_door_opened_then_closed(void){
    Event evt = {VALID_DOOR_ACCESS_EVENT,NULL};
    DoorInfo doorInfo = {DOOR_CLOSED_AND_LOCKED_STATE};
    uint32_t timeline[] ={
    0,1,3           // time generated for system to check the time
    };
    OnOff solenoidAction[]={
     OFF,ON ,-1                 // Solenoid will OFF as valid access detected
    };                           // Solenoid will ON after the door was closed
    StartStop beepAction[]={
    STOP,-1              // beep will STOP as the door closed
    };
    initTimerAndLowLevelHardware(timeline,(sizeof(timeline)/sizeof(uint32_t)),
                                 solenoidAction,beepAction);
    // in closed and locked state with valid access card detected
    // Expected DOOR_CLOSED_AND_UNLOCKED_STATE
    handleDoor(&evt,&doorInfo); //valid access detected , expected DOOR_CLOSED_AND_UNLOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_UNLOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(0,doorInfo.time);  //0 sec detected
    evt.type = DOOR_OPENED_EVENT; //  door opened happened
    handleDoor(&evt,&doorInfo); // expected DOOR_OPENED_STATE
    TEST_ASSERT_EQUAL(DOOR_OPENED_STATE,doorInfo.state);// expected assigned to DOOR_OPENED_STATE
    TEST_ASSERT_EQUAL(1,doorInfo.time);  //1 sec detected
    TEST_ASSERT_EQUAL(1,doorInfo.timeDiff); // expected 1 second difference from previous state
    evt.type = DOOR_CLOSED_EVENT; //  door closed happened
    handleDoor(&evt,&doorInfo); // expected DOOR_CLOSED_AND_LOCKED_STATE
    TEST_ASSERT_EQUAL(DOOR_CLOSED_AND_LOCKED_STATE,doorInfo.state);
    TEST_ASSERT_EQUAL(3,doorInfo.time);  //3 sec detected
    TEST_ASSERT_EQUAL(2,doorInfo.timeDiff); // expected 2 second difference from previous state
    TEST_ASSERT_EQUAL(1,doorInfo.previousTime);
    // expected previous State time is 1 as the time is recorded when changing state from closed unlocked to opened
}

/*
* getSolenoidTurnString is a function that is return char * of the destinated type
* which getSolenoidTurnString will return ON or off char* when either of them is detected on OnOff enum
* getBeepActionString is a function that is return char * of the destinated type
* which getBeepActionString will return START or STOP char* when either of them is detected on StartStop enum
* both of them will return NULL when it is not both STOP/START at getBeepActionString and
*  ON/OFF at getSolenoidTurnString
*/
void test_getSolenoidTurnString_input_ON_expect_ON_string_returned(void){
    char * expectString = NULL;
    OnOff swt = ON;
    expectString=getSolenoidTurnString(swt);
    TEST_ASSERT_EQUAL_STRING(expectString,"ON"); //expected ON string as input is ON in OnOff enum
}

void test_getSolenoidTurnString_input_OFF_expect_OFF_string_returned(void){
    char * expectString = NULL;
    OnOff swt = OFF;
    expectString=getSolenoidTurnString(swt);
    TEST_ASSERT_EQUAL_STRING(expectString,"OFF"); //expected OFF string as input is OFF in OnOff enum
}

void test_getSolenoidTurnString_input_weird_expect_NULL_returned(void){
    char * expectString = NULL;
    OnOff swt = -1;
    expectString=getSolenoidTurnString(swt);
    TEST_ASSERT_NULL(expectString);             //expected NULL as invalid input is detected
}

void test_getBeepActionString_input_START_expect_START_string_returned(void){
    char * expectString = NULL;
    StartStop action = START;
    expectString=getBeepActionString(action);
    TEST_ASSERT_EQUAL_STRING(expectString,"START");  //expected START string as input is START in StartStop enum
}

void test_getBeepActionString_input_STOP_expect_STOP_string_returned(void){
    char * expectString = NULL;
    StartStop action = STOP;
    expectString=getBeepActionString(action);
    TEST_ASSERT_EQUAL_STRING(expectString,"STOP"); //expected STOP string as input is STOP in StartStop enum
}

void test_getBeepActionString_input_weird_expect_NULL_returned(void){
    char * expectString = NULL;
    StartStop action = 3;
    expectString=getBeepActionString(action);
    TEST_ASSERT_NULL(expectString);          //expected NULL as invalid input is detected
}
