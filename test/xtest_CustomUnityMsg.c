#include "unity.h"
#include "CustomUnityMsg.h"

void setUp(void){}

void tearDown(void){}

/*
* testFailMessage(char *format ,...) is a function that is similar like
* TEST_FAIL_MESSAGE() but it can produce better output which it can bring in value and
* showed at ceedling fail message
* this test is default added x , for testing it required to remove x at first letter of the
* test file name
*/
void test_CustomUnityMsg_produce_my_name_and_age_error_message(void)
{
    char * str = "alibaba";
    int age = 32;
    testFailMessage("My name is %s and my age is %d"
                    ,str,age);
}

void test_CustomUnityMsg_produce_float_value_error_message(void)
{
    char * str = "maths";
    float mark = 32.34567;
    testFailMessage("My %s final exam scored %.5f"
                    ,str,mark);
}
