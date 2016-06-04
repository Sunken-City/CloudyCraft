//Code based off of code from Prof. Christopher Forseth

/************************************************************************/
/*                                                                      */
/* INCLUDE                                                              */
/*                                                                      */
/************************************************************************/
#include "Engine/Core/Memory/Callstack.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <Windows.h>
#include <DbgHelp.h>


/************************************************************************/
/*                                                                      */
/* DEFINES AND CONSTANTS                                                */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* GLOBAL VARIABLES                                                     */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* MACROS                                                               */
/*                                                                      */
/************************************************************************/
#define MAX_SYMBOL_NAME_LENGTH 128
#define MAX_FILENAME_LENGTH 1024
#define MAX_DEPTH 128

/************************************************************************/
/*                                                                      */
/* TYPES                                                                */
/*                                                                      */
/************************************************************************/

// SymInitialize()
typedef BOOL (__stdcall *sym_initialize_t)( IN HANDLE hProcess, IN PSTR UserSearchPath, IN BOOL fInvadeProcess );
typedef BOOL (__stdcall *sym_cleanup_t)( IN HANDLE hProcess );
typedef BOOL (__stdcall *sym_from_addr_t)( IN HANDLE hProcess, IN DWORD64 Address, OUT PDWORD64 Displacement, OUT PSYMBOL_INFO Symbol );

typedef BOOL (__stdcall *sym_get_line_t)( IN HANDLE hProcess, IN DWORD64 dwAddr, OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE64 Symbol );

/************************************************************************/
/*                                                                      */
/* STRUCTS                                                              */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* CLASSES                                                              */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* LOCAL VARIABLES                                                      */
/*                                                                      */
/************************************************************************/
static HMODULE gDebugHelp;
static HANDLE gProcess;
static SYMBOL_INFO* gSymbol;

// only called from single thread - so can use a shared buffer
static char gFileName[MAX_FILENAME_LENGTH];
static CallstackLine gCallstackBuffer[MAX_DEPTH];

static sym_initialize_t LSymInitialize;
static sym_cleanup_t LSymCleanup;
static sym_from_addr_t LSymFromAddr;
static sym_get_line_t LSymGetLineFromAddr64;

/************************************************************************/
/*                                                                      */
/* LOCAL FUNCTIONS                                                      */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/* EXTERNAL FUNCTIONS                                                   */
/*                                                                      */
/************************************************************************/

//------------------------------------------------------------------------
bool CallstackSystemInit()
{
    gDebugHelp = LoadLibraryA("dbghelp.dll");
    ASSERT_OR_DIE(gDebugHelp != nullptr, "Unable to load dbghelp.dll");
    LSymInitialize = (sym_initialize_t)GetProcAddress(gDebugHelp, "SymInitialize");
    LSymCleanup = (sym_cleanup_t)GetProcAddress(gDebugHelp, "SymCleanup");
    LSymFromAddr = (sym_from_addr_t)GetProcAddress(gDebugHelp, "SymFromAddr");
    LSymGetLineFromAddr64 = (sym_get_line_t)GetProcAddress(gDebugHelp, "SymGetLineFromAddr64");

    gProcess = GetCurrentProcess();
    LSymInitialize(gProcess, NULL, TRUE);

    gSymbol = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + (MAX_FILENAME_LENGTH * sizeof(char)));
    gSymbol->MaxNameLen = MAX_FILENAME_LENGTH;
    gSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);

   return true;
}

//------------------------------------------------------------------------
static int gCallstackCount = 0;
void CallstackSystemDeinit()
{
    LSymCleanup(gProcess);

    FreeLibrary(gDebugHelp);
    gDebugHelp = NULL;

    if (gCallstackCount != 0)
    {
        gCallstackCount = 0;
    }

    free(gSymbol);
    --gCallstackCount;
}

//------------------------------------------------------------------------
Callstack* AllocateCallstack(uint skipFrames)
{
    void* stack[MAX_DEPTH];
    //Getting from the calling function (0 is this function) to the number you want to capture
    uint32_t frames = CaptureStackBackTrace(1 + skipFrames, MAX_DEPTH, stack, NULL);
        
    ++gCallstackCount;

    size_t size = sizeof(Callstack) + sizeof(void*) * frames;
    void* bufferDataBegin = malloc(size);
    Callstack* callstack = new(bufferDataBegin) Callstack();
    byte* frameDataFront = (byte*)bufferDataBegin + sizeof(Callstack);

    callstack->frames = (void**)frameDataFront;
    callstack->frameCount = frames;
    memcpy(callstack->frames, stack, sizeof(void*) * frames);

    return callstack;
}

//-----------------------------------------------------------------------------------
void FreeCallstack(Callstack* stackToFree)
{
    //This frees everything allocated by malloc in one, big chunk
    //Since malloc inherently knows how big that is since we created it with malloc
    free(stackToFree);
}

//------------------------------------------------------------------------
// Should only be called from the debug trace thread.  
CallstackLine* CallstackGetLines(Callstack* cs) 
{
    IMAGEHLP_LINE64 LineInfo; 
    DWORD LineDisplacement = 0; // Displacement from the beginning of the line 
    LineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    uint count = cs->frameCount;
    for (uint i = 0; i < count; ++i) 
    {
        CallstackLine* line = &(gCallstackBuffer[i]);
        DWORD64 ptr = (DWORD64)(cs->frames[i]);
        LSymFromAddr(gProcess, ptr, 0, gSymbol);

        strcpy_s(line->functionName, CALLSTACK_BUFFER_SIZE, gSymbol->Name);

        BOOL bRet = LSymGetLineFromAddr64( 
            GetCurrentProcess(), // Process handle of the current process 
            ptr, // Address 
            &LineDisplacement, // Displacement will be stored here by the function 
            &LineInfo );         // File name / line information will be stored here 

        if (bRet) 
        {
            line->line = LineInfo.LineNumber;
            const char* filename = LineInfo.FileName;
//             char const *filename = StringFindLast( LineInfo.FileName, "\\src" );
//             if (filename == NULL) 
//             {
//                 filename = LineInfo.FileName;
//             } 
//             else 
//             {
//             filename += 5; // skip to the important bit - so that it can be double clicked in Output
//             }
            strcpy_s(line->filename, CALLSTACK_BUFFER_SIZE, filename);
            line->offset = LineDisplacement;
        } 
        else 
        {
            line->line = 0;
            line->offset = 0;
            strcpy_s(line->filename, CALLSTACK_BUFFER_SIZE, "N/A");
        }
    }

    return gCallstackBuffer;
}
