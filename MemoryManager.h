#pragma once

#ifndef MEMORYMANAGER
#define MEMORYMANAGER

#include "Core/Logger.h"

#include <memory>
#include <shared_mutex>
#include <iostream>


#define ME_MEMMAX (100 * 1024)
#define ME_MEM_ERROR(condition, msg)\
if(!(condition)){\
std::cout << msg << std::endl;\
throw std::bad_alloc();\
}
#define ME_MEM_INIT() ME::InitAllocator()
#define ME_MEM_CLEAR() ME::DeInitAllocator()

#ifdef ME_MEM_DEEPDEBUG
#include <unordered_map>
#include <set>
#endif

namespace ME
{
    typedef unsigned long long size_t;

    void InitAllocator();
    void DeInitAllocator();

    class UpstreamMemory
    {
    public:
        virtual void* allocate(const size_t& size, std::string msg = "") = 0;
        virtual void* reallocate(void* end_ptr, const size_t& size, std::string msg = "") = 0;
        virtual void deallocate(void* ptr, const size_t& size, std::string msg = "") = 0;
    };
    class malloc_stdfree_UpstreamMemory : public UpstreamMemory
    {
    public:
        virtual void* allocate(const size_t& size, std::string msg = "") override;
        virtual void* reallocate(void* end_ptr, const size_t& size, std::string msg = "") override;
        virtual void deallocate(void* ptr, const size_t& size, std::string msg = "") override;

        static malloc_stdfree_UpstreamMemory* stref;
    };
    class alloc_dealloc_UpstreamMemory : public UpstreamMemory
    {
    public:
        virtual void* allocate(const size_t& size, std::string msg = "") override;
        virtual void* reallocate(void* end_ptr, const size_t& size, std::string msg = "") override;
        virtual void deallocate(void* ptr, const size_t& size, std::string msg = "") override;

        static alloc_dealloc_UpstreamMemory* stref;
    };
    class null_UpstreamMemory : public UpstreamMemory
    {
    public:
        virtual void* allocate(const size_t&, std::string msg = "") override { ME_MEM_ERROR(true, "Triggered null_Upstream"); return nullptr; }
        virtual void* reallocate(void* end_ptr, const size_t& size, std::string msg = "") override { return allocate(0); }
        virtual void deallocate(void*, const size_t&, std::string msg = "") override { allocate(0); }

        static null_UpstreamMemory* stref;
    };

    static UpstreamMemory* set_malloc_stdfree_UpstreamMemory() { return malloc_stdfree_UpstreamMemory::stref; }
    static UpstreamMemory* set_alloc_dealloc_UpstreamMemory() { return alloc_dealloc_UpstreamMemory::stref; }
    static UpstreamMemory* set_null_UpstreamMemory() { return null_UpstreamMemory::stref; }

    class MemoryManager
    {
    public:
        UpstreamMemory* m_UpstreamMemory;

        MemoryManager(UpstreamMemory* upstreammemory = set_alloc_dealloc_UpstreamMemory());
        ~MemoryManager() = default;

        [[nodiscard]] virtual void* allocate(const size_t& size) = 0;
        [[nodiscard]] virtual void* verified_allocate(void* end_ptr, const size_t& size) = 0;
        virtual void deallocate(void* ptr, const size_t& size) noexcept = 0;
        virtual void release() = 0;
        virtual size_t getFreeMemory() const = 0;
        virtual size_t getMaxMemory() const = 0;
        virtual size_t getUsedMemory() const = 0;
#ifdef ME_MEM_DEEPDEBUG
        virtual const std::unordered_map<void*, std::set<long long>>& getAllocationRegistry() = 0;
#endif
        // The global Allocator
        static MemoryManager* Allocator;
    };

    // Faster global Allocators
    // Params Size: Number of variable to be allocated
    template<typename T>
    constexpr static T* alloc()
    {

        ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");

        T* ptr = (T*)MemoryManager::Allocator->allocate(sizeof(T));
#ifdef ME_MEM_DEBUG_2
        ME_CORE_WARNING("Using alloc | Allocated Size: {} | {}", sizeof(T), (void*)ptr);
#endif
        return ptr;
    }
    template<typename T>
    constexpr static T* allocarr(const size_t& element_count)
    {

        ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");

        T* ptr = (T*)MemoryManager::Allocator->allocate(element_count * sizeof(T));
#ifdef ME_MEM_DEBUG_2
        ME_CORE_WARNING("Using alloc | Allocated Size: {} | {}", element_count * sizeof(T), (void*)ptr);
#endif
        if (element_count)
            return ptr;

        return nullptr;
    }
    template<typename T, typename ...Args>
    constexpr static T* allocon(Args&& ...args)
    {

        ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");
        T* ptr = (T*)MemoryManager::Allocator->allocate(sizeof(T));

#ifdef ME_MEM_DEBUG_2
        ME_CORE_WARNING("Using allocon | Allocated Size: {} | {}", sizeof(T), (void*)ptr);
#endif
        new (ptr) T(args...);
        return ptr;
    }

    template<typename T>
    constexpr static void dealloc(T* ptr, const size_t& size)
    {

        ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");

#ifdef ME_MEM_DEBUG_2
        ME_CORE_WARNING("Using dealloc | Deallocated Size: {} | {}", size, (void*)ptr);
#endif
        ptr->~T();
        MemoryManager::Allocator->deallocate((void*)ptr, size);
    }
    template<typename T>
    constexpr static T* realloc(T* end_ptr, const size_t& size)
    {

        ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");

        T* ptr = (T*)MemoryManager::Allocator->verified_allocate(end_ptr, sizeof(T) * size);
#ifdef ME_MEM_DEBUG_2
        ME_CORE_WARNING("using realloc | Reallocated Size: {} | {}", size, (void*)ptr);
#endif
        return ptr;
    }
    static size_t Maxmem() noexcept { return MemoryManager::Allocator->getMaxMemory(); }
    static size_t LeftMem() noexcept { return MemoryManager::Allocator->getFreeMemory(); }
#ifdef ME_MEM_DEEPDEBUG
#define dealloc(X, Y) \
    dealloc(X, Y);\
    {\
        auto reg = ME::MemoryManager::Allocator->getAllocationRegistry();\
        if(reg[(void*)X].size())\
        {\
            auto pt = reg[(void*)X].find(Y);\
            if (pt == reg[(void*)X].end())\
                throw;\
        }\
    }
#endif
}

#endif // !MEMORYMANAGER