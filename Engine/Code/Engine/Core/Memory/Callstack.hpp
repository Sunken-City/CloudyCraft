#pragma once
#if !defined( __DEBUG_CALLSTACK__ )
#define __DEBUG_CALLSTACK__

//Code based off of code from Prof. Christopher Forseth

/************************************************************************/
/*                                                                      */
/* INCLUDE                                                              */
/*                                                                      */
/************************************************************************/

#include <stdint.h>

/************************************************************************/
/*                                                                      */
/* GLOBAL VARIABLES                                                     */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* DEFINES AND CONSTANTS                                                */
/*                                                                      */
/************************************************************************/

const int CALLSTACK_BUFFER_SIZE = 1024;

/************************************************************************/
/*                                                                      */
/* MACROS                                                               */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* TYPES                                                                */
/*                                                                      */
/************************************************************************/

typedef unsigned int uint;
typedef unsigned char byte;

/************************************************************************/
/*                                                                      */
/* STRUCTS                                                              */
/*                                                                      */
/************************************************************************/

struct Callstack
{
    Callstack() : frames(nullptr), frameCount(0) {};
    void** frames;
    uint frameCount;
};

struct CallstackLine 
{
    char filename[CALLSTACK_BUFFER_SIZE];
    char functionName[CALLSTACK_BUFFER_SIZE];
    uint32_t line;
    uint32_t offset;
};

/************************************************************************/
/*                                                                      */
/* CLASSES                                                              */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* FUNCTION PROTOTYPES                                                  */
/*                                                                      */
/************************************************************************/

bool CallstackSystemInit();
void CallstackSystemDeinit();
Callstack* AllocateCallstack(uint skipFrames = 1);
void FreeCallstack(Callstack* stackToFree);

// Single Threaded - only from debug output thread (if I need the string names elsewhere
// then I need to make a "debug" job consumer)
CallstackLine* CallstackGetLines(Callstack* cs);

#endif 
