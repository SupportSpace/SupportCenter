/////////////////////////////////////////////////////////////////////////
///
///  Archer Software.
///
///  lib_allocator.h
///
///  Allocator template for library
///
///  @author Dmitry Netrebenko @date 25.12.2006
///
////////////////////////////////////////////////////////////////////////
#pragma once

#include <memory>

///  Allocator
template<typename Type>
struct nwl_allocator : public std::allocator<Type>
{
    std::allocator<Type>::pointer allocate(std::allocator<Type>::size_type count, const void* = 0) const
    {
        return reinterpret_cast<std::allocator<Type>::pointer>(this->allocate_impl(count * sizeof(Type)));
    }

    void deallocate(std::allocator<Type>::pointer ptr, std::allocator<Type>::size_type) const
    {
        this->deallocate_impl(ptr);
    }

    template<typename Other>
    struct rebind 
    {
        typedef nwl_allocator<Other> other;
    };

private:
    virtual void* allocate_impl(size_t count) const
    {
        return operator new(count);
//		 return HeapAlloc(GetProcessHeap(),0,count);
    }

    virtual void deallocate_impl(void* p) const
    {
        operator delete(p);
//		HeapFree(GetProcessHeap(),0,p);
    }
};

