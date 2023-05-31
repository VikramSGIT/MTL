#pragma once

#define ME_MEMMAX (100 * 1024)

#ifdef ME_ISOLATE
#include <iostream>

#define ME_MEM_ERROR(condition, ...)\
if(!(condition)){\
std::cerr << __VA_ARGS__ << std::endl;\
throw "Memory Corrupted!!";\
}
#else
#include "Core/Logger.h"

#define ME_MEM_ERROR(condition, ...)\
if(!(condition)){\
spdlog::critical(__VA_ARGS__);\
throw "Memory Corrupted!!";\
}

#endif

void InitAllocator();
void DeInitAllocator();

namespace ME
{
    typedef unsigned long long size_t;

    class MemoryManager
    {
    public:
        MemoryManager() = default;
        ~MemoryManager() = default;

        static int CheckHeapIntegrity() { return _CrtCheckMemory(); }

        [[nodiscard]] virtual void* allocate(const size_t&) = 0;
        [[nodiscard]] virtual void* reallocate(void*&, const size_t&) = 0;
        virtual void deallocate(void*) = 0;
        virtual void forced_deallocate(void*) = 0;

        virtual void release() = 0;

        virtual size_t getFreeMemory() noexcept = 0;
        virtual size_t getMaxMemory() noexcept = 0;
        virtual size_t getUsedMemory() noexcept = 0;

        // The global Allocator
        static MemoryManager* Allocator;
    };

    class UpstreamMemory
    {
    public:
        virtual void* allocate(const size_t& size) = 0;
        virtual void* reallocate(void *&ptr, const size_t& size) = 0;
        virtual void deallocate(void* ptr) = 0;
        virtual void forced_deallocated(void* ptr) = 0;
#ifdef ME_MEM_DEBUG
        template<typename ...Args>
        void message(Args... args) {
            std::cout << "WARNING: ";
            ((std::cout << args << " "), ...);
            std::cout << std::endl;
        }
#else
        template<typename ...Args>
        void message(Args... args) {}
#endif
    };
    class STDUpstreamMemory : public UpstreamMemory
    {
    public:
        virtual void* allocate(const size_t& size) override { return std::malloc(size); }
        virtual void* reallocate(void*& ptr, const size_t& size) override { return std::realloc(ptr, size); }
        virtual void deallocate(void* ptr) override { return std::free(ptr); }
        virtual void forced_deallocated(void* ptr) override { return std::free(ptr); }

        static STDUpstreamMemory* stref;
    };
    class MEUpstreamMemory : public UpstreamMemory
    {
    public:
        virtual void* allocate(const size_t& size) override {
            ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");
            return MemoryManager::Allocator->allocate(size);
        }
        virtual void* reallocate(void*& ptr, const size_t& size) override {
            ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");
            return MemoryManager::Allocator->reallocate(ptr, size);
        }
        virtual void deallocate(void* ptr) override {
            ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");
            return MemoryManager::Allocator->deallocate(ptr);
        }
        virtual void forced_deallocated(void* ptr) override {
            ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");
            return MemoryManager::Allocator->forced_deallocate(ptr);
        }

        static MEUpstreamMemory* stref;
    };

    static UpstreamMemory* getSTDUpstreamMemory() { return STDUpstreamMemory::stref; }
    static UpstreamMemory* getMEUpstreamMemory() { return MEUpstreamMemory::stref; }

    // Faster global Allocators
    // Params Size: Number of variable to be allocated
    template<typename T>
    constexpr static T* alloc(const size_t& count = 1)
    {

        ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");

        T* ptr = (T*)MemoryManager::Allocator->allocate(sizeof(T) * count);
#ifdef ME_MEM_DEBUG
#ifndef ME_ISOLATE
        ME_CORE_WARNING("Using alloc | Allocated Size: {} | {}", sizeof(T) * count, (void*)ptr);
#else
        std::cout << "WARNING: Using alloc | Allocated Size: " << sizeof(T) * count << " | " << (void*)ptr << std::endl;
#endif
#endif
        return ptr;
    }
    template<typename T, typename ...Args>
    constexpr static T* allocon(Args&& ...args)
    {

        ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");
        T* ptr = (T*)MemoryManager::Allocator->allocate(sizeof(T));

#ifdef ME_MEM_DEBUG
#ifndef ME_ISOLATE
        ME_CORE_WARNING("Using allocon | Allocated Size: {} | {}", sizeof(T), (void*)ptr);
#else
        std::cout << "WARNING: Using allocon | Allocated Size: " << sizeof(T) << " | " << (void*)ptr << std::endl;
#endif
#endif
        new (ptr) T(args...);
        return ptr;
    }

    template<typename T>
    constexpr static void dealloc(T* ptr)
    {

        ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");

#ifdef ME_MEM_DEBUG
#ifndef ME_ISOLATE
        ME_CORE_WARNING("Using dealloc | {}", (void*)ptr);
#else
        std::cout << "WARNING: Using dealloc | " << (void*)ptr << std::endl;
#endif
#endif
        ptr->~T();
        MemoryManager::Allocator->deallocate((void*)ptr);
    }
    template<typename T>
    constexpr static void forced_dealloc(T* ptr)
    {

        ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");

#ifdef ME_MEM_DEBUG
#ifndef ME_ISOLATE
        ME_CORE_WARNING("Using forced dealloc | {}", (void*)ptr);
#else
        std::cout << "WARNING: Using forced dealloc | " << (void*)ptr << std::endl;
#endif
#endif
        ptr->~T();
        MemoryManager::Allocator->forced_deallocate((void*)ptr);
    }
    template<typename T>
    constexpr static T* realloc(T *&ptr, const size_t& size)
    {

        ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");

        auto res = (T*)MemoryManager::Allocator->reallocate((void*&)ptr, sizeof(T) * size);
#ifdef ME_MEM_DEBUG
#ifndef ME_ISOLATE
        ME_CORE_WARNING("Using realloc | Reallocated Size: {} | {}", size, (void*)ptr);
#else
        std::cout << "WARNING: Using realloc | Reallocated Size: " << size * sizeof(T) << " | " << res << std::endl;
#endif
#endif
        return res;
    }
    static size_t Maxmem() noexcept { return MemoryManager::Allocator->getMaxMemory(); }
    static size_t LeftMem() noexcept { return MemoryManager::Allocator->getFreeMemory(); }
}