#ifndef MEMORYMANAGER
#define MEMORYMANAGER

#pragma once

#include "Core/Logger.h"

//#define ME_MEM_DEBUG
//#define ME_MEM_DEBUG_2
//#define ME_MEM_DEEPDEBUG

#define ME_MEMMAX (100 * 1024)
#define ME_MEM_ERROR(condition, ...)\
if(!(condition)){\
spdlog::critical(__VA_ARGS__);\
throw "Memory Corrupted!!";\
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
        virtual void* reallocate(void *&ptr, const size_t& size, std::string msg = "") = 0;
        virtual void deallocate(void* ptr, std::string msg = "") = 0;
    };
    class malloc_stdfree_UpstreamMemory : public UpstreamMemory
    {
    public:
        virtual void* allocate(const size_t& size, std::string msg = "") override;
        virtual void* reallocate(void *&ptr, const size_t& size, std::string msg = "") override;
        virtual void deallocate(void* ptr, std::string msg = "") override;

        static malloc_stdfree_UpstreamMemory* stref;
    };
    class alloc_dealloc_UpstreamMemory : public UpstreamMemory
    {
    public:
        virtual void* allocate(const size_t& size, std::string msg = "") override;
        virtual void* reallocate(void *&ptr, const size_t& size, std::string msg = "") override;
        virtual void deallocate(void* ptr, std::string msg = "") override;

        static alloc_dealloc_UpstreamMemory* stref;
    };
    class null_UpstreamMemory : public UpstreamMemory
    {
    public:
        virtual void* allocate(const size_t&, std::string msg = "") override { ME_MEM_ERROR(true, "Triggered null_Upstream"); return nullptr; }
        virtual void* reallocate(void *&end_ptr, const size_t& size, std::string msg = "") override { return allocate(0); }
        virtual void deallocate(void*, std::string msg = "") override { allocate(0); }

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

        static int CheckHeapIntegrity() { return _CrtCheckMemory(); }

        [[nodiscard]] virtual void* allocate(const size_t& size) = 0;
        [[nodiscard]] virtual void* reallocate(void *&ptr, const size_t& size) = 0;
        virtual void deallocate(void* ptr) noexcept = 0;
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
    constexpr static T* alloc(const size_t& count = 1)
    {

        ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");

        T* ptr = (T*)MemoryManager::Allocator->allocate(sizeof(T) * count);
#ifdef ME_MEM_DEBUG_2
        ME_CORE_WARNING("Using alloc | Allocated Size: {} | {}", sizeof(T), (void*)ptr);
#endif
        return ptr;
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
    constexpr static void dealloc(T* ptr)
    {

        ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");

#ifdef ME_MEM_DEBUG_2
        ME_CORE_WARNING("Using dealloc | {}", (void*)ptr);
#endif
        ptr->~T();
        MemoryManager::Allocator->deallocate((void*)ptr);
    }
    template<typename T>
    constexpr static T* realloc(T *&ptr, const size_t& size)
    {

        ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");

        MemoryManager::Allocator->reallocate((void*&)ptr, size);
#ifdef ME_MEM_DEBUG_2
        ME_CORE_WARNING("using realloc | Reallocated Size: {} | {}", size, (void*)ptr);
#endif
        return (T*)ptr;
    }
    static size_t Maxmem() noexcept { return MemoryManager::Allocator->getMaxMemory(); }
    static size_t LeftMem() noexcept { return MemoryManager::Allocator->getFreeMemory(); }
}

#endif // !MEMORYMANAGER