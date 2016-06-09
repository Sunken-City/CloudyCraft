#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Engine/Core/ErrorWarningAssert.hpp"

//FORWARD DECLARATIONS//////////////////////////////////////////////////////////////////////////
struct Callstack;
class MemoryAnalytics;

//GLOBAL VARIABLES//////////////////////////////////////////////////////////////////////////
extern class MemoryMetadata* g_memoryMetadataList;
extern MemoryAnalytics g_memoryAnalytics;

//-----------------------------------------------------------------------------------
class MemoryMetadata
{
public:
    MemoryMetadata()
        : sizeOfAllocInBytes(0)
        , callstack(nullptr)
        , next(nullptr)
        , prev(nullptr)
    {};

    ~MemoryMetadata();

    //In place linked list methods.
    static void AddMemoryMetadataToList(MemoryMetadata* stackToAdd);
    static void RemoveMemoryMetadataFromList(MemoryMetadata* metadataToRemove);
    static void PrintAllMetadataInList();

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    size_t sizeOfAllocInBytes;
    Callstack* callstack;
    MemoryMetadata* next;
    MemoryMetadata* prev;
};

//-----------------------------------------------------------------------------------
class MemoryAnalytics
{
public:
    MemoryAnalytics();
    void* Allocate(const size_t numBytes);
    void Free(const void* ptr);
    void Startup();
    void Shutdown();
    inline void TrackRenderBufferAllocation()
    {
        ++m_numberOfRenderBufferAllocations;
        #if (TRACK_MEMORY == 2)
                {
                    DebuggerPrintf("Allocate RenderBuffer! Total Number: %i\n", m_numberOfRenderBufferAllocations);
                }
        #endif
    };
    inline void TrackRenderBufferFree()
    {
        --m_numberOfRenderBufferAllocations;
        #if (TRACK_MEMORY == 2)
                {
                    DebuggerPrintf("Free RenderBuffer! Total Number: %i\n", m_numberOfRenderBufferAllocations);
                }
        #endif
    };
    inline void TrackVAOAllocation() 
    { 
        ++m_numberOfVAOAllocations;
        #if (TRACK_MEMORY == 2)
        {
            DebuggerPrintf("Allocate VBO! Total Number: %i\n", m_numberOfVAOAllocations);
        }
        #endif
    };
    inline void TrackVAOFree() 
    { 
        --m_numberOfVAOAllocations;
        #if (TRACK_MEMORY == 2)
        {
            DebuggerPrintf("Free VBO! Total Number: %i\n", m_numberOfVAOAllocations);
        }
        #endif
    };
    inline void TrackShaderAllocation() 
    { 
        ++m_numberOfShaderAllocations;
        #if (TRACK_MEMORY == 2)
        {
            DebuggerPrintf("Allocate Shader! Total Number: %i\n", m_numberOfShaderAllocations);
        }
        #endif
    };
    inline void TrackShaderFree() 
    { 
        --m_numberOfShaderAllocations;
        #if (TRACK_MEMORY == 2)
        {
            DebuggerPrintf("Free Shader! Total Number: %i\n", m_numberOfShaderAllocations);
        }
        #endif
    };

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    bool m_isInitialized;
    unsigned int m_startupNumberOfAllocations;
    unsigned int m_numberOfAllocations;
    unsigned int m_numberOfShaderAllocations;
    unsigned int m_numberOfVAOAllocations;
    unsigned int m_numberOfRenderBufferAllocations;
    unsigned int m_highwaterInBytes;
    size_t m_numberOfBytes;
    CRITICAL_SECTION m_memoryManagerCriticalSection;

private:
    //-----------------------------------------------------------------------------------
    void AttemptLock()
    {
        if (m_isInitialized)
        {
            EnterCriticalSection(&m_memoryManagerCriticalSection);
        }
    }

    //-----------------------------------------------------------------------------------
    void AttemptLeave()
    {
        if (m_isInitialized)
        {
            LeaveCriticalSection(&m_memoryManagerCriticalSection);
        }
    }
};

void MemoryAnalyticsStartup();
void MemoryAnalyticsShutdown();

//-----------------------------------------------------------------------------------
template <typename T>
T* UntrackedNew()
{
    T* newPtr = (T*)malloc(sizeof(T));
    return new (newPtr) T();
}

//VS2015 ONLY! SUPER DUPER MODERN!
//-----------------------------------------------------------------------------------
template <typename T, typename ...ARGS>
T* UntrackedNew(ARGS... args)
{
    T *ptr = (T*)malloc(sizeof(T));
    new (ptr) T(args...);
    return ptr;
}

//-----------------------------------------------------------------------------------
template <typename T>
void UntrackedDelete(T* ptrToDelete)
{
    ptrToDelete->~T();
    ::free(ptrToDelete);
}
