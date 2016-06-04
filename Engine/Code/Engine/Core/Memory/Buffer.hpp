#pragma once

class Buffer
{
public:
    Buffer(void* initialData, size_t size);

    template <class T>
    void Alloc<T>();
    
};