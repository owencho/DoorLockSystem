#include "unity.h"
#include "Convert.h"
#include "OnOff.h"
#include "Event.h"
#include <stdio.h>

void setUp(void){}

void tearDown(void){}
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
