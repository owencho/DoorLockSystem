#include "CustomUnityMsg.h"
#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <stdarg.h>
#include "unity.h"

void testFailMessage(char *format ,...){
    int actualLength;
    char* buffer;
    va_list arg;
    va_start(arg, format);
    actualLength = vsnprintf(NULL,0,format, arg);   //trick system to take actualLength
    buffer =malloc(actualLength + 1);               // allocate value to buffer
    vsnprintf(buffer,actualLength + 1,format, arg); 
    TEST_FAIL_MESSAGE(buffer);
    va_end(arg);
}
