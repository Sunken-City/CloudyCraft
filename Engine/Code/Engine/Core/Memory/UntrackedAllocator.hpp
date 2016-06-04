#pragma once
#undef max
#include <limits.h>
#include <xmemory0>
#include <string>
#include <iosfwd>

template <typename T>
class UntrackedAllocator
{
public:
    //TYPEDEFS//////////////////////////////////////////////////////////////////////////
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

public:
    //convert an allocator<T> to allocator<U>
    template<typename U>
    struct rebind
    {
        typedef UntrackedAllocator<U> other;
    };

public:
    inline explicit UntrackedAllocator() {}
    inline ~UntrackedAllocator() {}
    inline UntrackedAllocator(UntrackedAllocator const&) {}
    template<typename U>
    inline UntrackedAllocator(UntrackedAllocator<U> const&) {}

    //address
    inline pointer address(reference r)
    {
        return &r;
    }

    inline const_pointer address(const_reference r)
    {
        return &r;
    }

    //memory allocation
    //The second parameter is just a hint that the thing you want is near.
    //This is used for something but I'm not sure what.
    inline pointer allocate(size_type cnt, std::allocator<void>::const_pointer = 0)
    {
        T* ptr = (T*)malloc(cnt * sizeof(T));
        return ptr;
    }

    inline void deallocate(pointer p, size_type)
    {
        free(p);
    }

    //size
    inline size_type max_size() const
    {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    //construction/destruction
    inline void construct(pointer p, const T& t)
    {
        new(p) T(t);
    }

    inline void destroy(pointer ptr)
    {
        ptr;
        ptr->~T();
    }

    inline bool operator==(UntrackedAllocator const& a) { return this == &a; }
    inline bool operator!=(UntrackedAllocator const& a) { return !operator==(a); }
};