#include "Engine/Core/Memory/MemoryTracking.hpp"
#include "Engine/Core/Memory/Callstack.hpp"
#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/Console.hpp"

MemoryMetadata* g_memoryMetadataList = nullptr;

#if defined(TRACK_MEMORY)

MemoryAnalytics g_memoryAnalytics;

//-----------------------------------------------------------------------------------
void* operator new(size_t numBytes) //size_t is the size of a void*
{
    return g_memoryAnalytics.Allocate(numBytes);
}

//-----------------------------------------------------------------------------------
void operator delete(void* data)
{
    g_memoryAnalytics.Free(data);
}

//-----------------------------------------------------------------------------------
void* operator new[](size_t numBytes)
{
    return g_memoryAnalytics.Allocate(numBytes);
}

//-----------------------------------------------------------------------------------
void operator delete[](void *ptr)
{
    g_memoryAnalytics.Free(ptr);
}

//-----------------------------------------------------------------------------------
MemoryAnalytics::MemoryAnalytics()
    : m_isInitialized(false)
    , m_numberOfAllocations(0)
    , m_numberOfBytes(0)
    , m_startupNumberOfAllocations(0)
    , m_numberOfShaderAllocations(0)
    , m_numberOfVAOAllocations(0)
    , m_numberOfRenderBufferAllocations(0)
{
    InitializeCriticalSection(&m_memoryManagerCriticalSection);
}

//-----------------------------------------------------------------------------------
void MemoryAnalytics::Startup()
{
    m_isInitialized = true;
    CallstackSystemInit();
    DebuggerPrintf("Number of allocations before startup: %i.  Total size: %luB\n", m_numberOfAllocations, m_numberOfBytes);
    m_startupNumberOfAllocations = m_numberOfAllocations;
}

//-----------------------------------------------------------------------------------
void MemoryAnalytics::Shutdown()
{
    m_isInitialized = false;
    CallstackSystemDeinit();
    DebuggerPrintf("Number of allocations at shutdown: %i.  Total size: %luB\n", m_numberOfAllocations, m_numberOfBytes);
}

//-----------------------------------------------------------------------------------
void* MemoryAnalytics::Allocate(const size_t numBytes)
{
    MemoryMetadata* metadata = UntrackedNew<MemoryMetadata>();
    const size_t real_size = sizeof(size_t) + numBytes;
    size_t* ptr = (size_t*) ::malloc(real_size);

    metadata->newedDataPtr = ptr;
    metadata->sizeOfAllocInBytes = numBytes;

    #if (TRACK_MEMORY > 0)
    {
        Callstack* callstackPtr = AllocateCallstack();
        metadata->callstack = callstackPtr;
    }
    #endif

    #if (TRACK_MEMORY == 2)
    {
        DebuggerPrintf("New called for %lu bytes. Pointer: %p\n", numBytes, ptr);
    }
    #endif

    //Save off the metadata so that we know how much memory to free later
    *ptr = (size_t)metadata;
    ++ptr;

    //THIS IS THE PART THAT NEEDS TO BE THREAD-SAFE
    AttemptLock();
    {
        ++m_numberOfAllocations;
        m_numberOfBytes += numBytes;
        if (m_numberOfBytes > m_highwaterInBytes)
        {
            m_highwaterInBytes = m_numberOfBytes;
        }
#if (TRACK_MEMORY > 0)
        //Add to map
        MemoryMetadata::AddMemoryMetadataToList(metadata);
#endif // TRACK_MEMORY > 0
    }
    AttemptLeave();
    return ptr;
    
    // BONUS MATERIAL 
    // Track average allocations and size per second as well
}

//-----------------------------------------------------------------------------------
void MemoryAnalytics::Free(const void* ptr)
{
    size_t *sizedPtr = (size_t *)ptr;
    --sizedPtr;
    MemoryMetadata* metadata = (MemoryMetadata*)*sizedPtr;
    ::free(sizedPtr);

#if (TRACK_MEMORY == 2)
    DebuggerPrintf("Delete called for %p.\n", ptr);
#endif // TRACK_MEMORY == 2

    //THIS IS THE PART THAT NEEDS TO BE THREAD-SAFE
    AttemptLock();
    {
        --m_numberOfAllocations;
        m_numberOfBytes -= metadata->sizeOfAllocInBytes;

#if (TRACK_MEMORY > 0)
        MemoryMetadata* foundCallstack = MemoryMetadata::FindMemoryMetadataInList((void*)sizedPtr);
        ASSERT_OR_DIE(foundCallstack, "Did not find a callstack for the associated memory free. Double Free?");
        MemoryMetadata::RemoveMemoryMetadataFromList(foundCallstack);
#endif // TRACK_MEMORY > 0

    }
    AttemptLeave();
}

//-----------------------------------------------------------------------------------
void MemoryMetadata::AddMemoryMetadataToList(MemoryMetadata* stackToAdd)
{
    if (!g_memoryMetadataList)
    {
        g_memoryMetadataList = stackToAdd;
        stackToAdd->next = stackToAdd;
        stackToAdd->prev = stackToAdd;
    }
    else
    {
        stackToAdd->next = g_memoryMetadataList->next;
        stackToAdd->prev = g_memoryMetadataList;
        g_memoryMetadataList->next->prev = stackToAdd;
        g_memoryMetadataList->next = stackToAdd;
    }
}

//-----------------------------------------------------------------------------------
void MemoryMetadata::RemoveMemoryMetadataFromList(MemoryMetadata* stackToRemove)
{
    stackToRemove->prev->next = stackToRemove->next;
    stackToRemove->next->prev = stackToRemove->prev;
    if (stackToRemove == g_memoryMetadataList)
    {
        g_memoryMetadataList = stackToRemove->next == stackToRemove ? nullptr : stackToRemove->next;
    }
}

//-----------------------------------------------------------------------------------
MemoryMetadata* MemoryMetadata::FindMemoryMetadataInList(void* dataPayload)
{
    MemoryMetadata* currentNode = g_memoryMetadataList;
    do
    {
        if (currentNode->newedDataPtr == dataPayload)
        {
            return currentNode;
        }
        currentNode = currentNode->next;
    } while (currentNode != g_memoryMetadataList);
    return nullptr;
}

//-----------------------------------------------------------------------------------
void MemoryMetadata::PrintAllMetadataInList()
{
    if (!g_memoryMetadataList)
    {
        DebuggerPrintf("Metadata list was null, nothing to print. (Are you not running in verbose mode?)\n");
        return;
    }
    MemoryMetadata* currentNode = g_memoryMetadataList;
    int callstackListIndex = -1;
    do
    {
        Callstack* callstack = currentNode->callstack;
        CallstackLine* callstackLines = CallstackGetLines(callstack);
        DebuggerPrintf("---===Allocation #%i===---\n>>>Size: %i bytes\n", ++callstackListIndex, currentNode->sizeOfAllocInBytes);
        DebuggerPrintf(">>>Callstack:\n//-----------------------------------------------------------------------------------\n");
        for (unsigned int i = 0; i < callstack->frameCount; ++i)
        {
            DebuggerPrintf("%s(%i): %s\n", callstackLines[i].filename, callstackLines[i].line, callstackLines[i].functionName);
        }
        currentNode = currentNode->next;
        DebuggerPrintf("//-----------------------------------------------------------------------------------\n\n", callstackListIndex);
    } while (currentNode != g_memoryMetadataList);
}

//-----------------------------------------------------------------------------------
void MemoryAnalyticsStartup()
{
    g_memoryAnalytics.Startup();
}

//-----------------------------------------------------------------------------------
void MemoryAnalyticsShutdown()
{
    //Walk the list of callstacks, print them all out. This is what you haven't freed
    //Can bucketize the callstacks into unique callstack hash lists, then print the # of reports you got
    if (g_memoryAnalytics.m_numberOfAllocations > g_memoryAnalytics.m_startupNumberOfAllocations)
    {
        ERROR_RECOVERABLE(Stringf("Leaked a total of %i bytes, from %i individual leaks. %i of these were from before startup.\n[Press enter to continue]", g_memoryAnalytics.m_numberOfBytes, g_memoryAnalytics.m_numberOfAllocations, g_memoryAnalytics.m_startupNumberOfAllocations));
        MemoryMetadata::PrintAllMetadataInList();
    }
    if (g_memoryAnalytics.m_numberOfShaderAllocations != 0)
    {
        ERROR_RECOVERABLE(Stringf("Leaked a total of %i shaders.\n[Press enter to continue]", g_memoryAnalytics.m_numberOfShaderAllocations));
    }
    if (g_memoryAnalytics.m_numberOfVAOAllocations != 0)
    {
        ERROR_RECOVERABLE(Stringf("Leaked a total of %i VAOs.\n[Press enter to continue]", g_memoryAnalytics.m_numberOfVAOAllocations));
    }
    if (g_memoryAnalytics.m_numberOfRenderBufferAllocations != 0)
    {
        ERROR_RECOVERABLE(Stringf("Leaked a total of %i Render Buffers.\n[Press enter to continue]", g_memoryAnalytics.m_numberOfRenderBufferAllocations));
    }
    g_memoryAnalytics.Shutdown();
}

CONSOLE_COMMAND(memoryFlush)
{
    Console::instance->PrintLine("Flushing metadata to conosle...", RGBA::BADDAD);
    MemoryMetadata::PrintAllMetadataInList();
    Console::instance->PrintLine("Metadata flushed to console.", RGBA::BADDAD);
}

#else

//If we aren't currently tracking memory, don't do anything.
//-----------------------------------------------------------------------------------
void MemoryAnalyticsStartup()
{

}

//-----------------------------------------------------------------------------------
void MemoryAnalyticsShutdown()
{

}

#endif