#include "MemoryManager.h"
#include "PoolAllocator.h"

ME::STDUpstreamMemory* ME::STDUpstreamMemory::stref = nullptr;
ME::MEUpstreamMemory* ME::MEUpstreamMemory::stref = nullptr;
ME::MemoryManager* ME::MemoryManager::Allocator = nullptr;

bool enabled = false;

void InitAllocator()
{
#ifndef ME_ISOLATE
#ifdef ME_MEM_DEBUG
    ME_CORE_WARNING("Memory Debugging enabled!!");
#endif
#else
#ifdef ME_MEM_DEBUG
    std::cout << "WARNING: Memory Debugging enabled!!" << std::endl;
#endif
#endif
    if (!enabled) {
        ME::STDUpstreamMemory::stref = new ME::STDUpstreamMemory;
        ME::MEUpstreamMemory::stref = new ME::MEUpstreamMemory;
        ME::MemoryManager::Allocator = new  ME::PoolAllocator<ME::STDUpstreamMemory>;
        enabled = true;
    }
    else {
        std::cout << "WARNING: Global Allocators already initialized!" << std::endl;
    }
}

void DeInitAllocator()
{
#ifndef ME_ISOLATE
    ME_CORE_ERROR(MemoryManager::Allocator->getUsedMemory(), "MEMORYMANAGER: Memory leak detected (<={}/{} bytes)",
        MemoryManager::Allocator->getUsedMemory(), MemoryManager::Allocator->getMaxMemory())
#else
    if (ME::MemoryManager::Allocator->getUsedMemory())
        std::cerr << "ERROR: MEMORYMANAGER: Memory leak detected (<=" << ME::MemoryManager::Allocator->getUsedMemory() << "/"
        << ME::MemoryManager::Allocator->getMaxMemory() << " bytes)" << std::endl;
#endif
    if (enabled) {
        delete ME::MemoryManager::Allocator;
        delete ME::STDUpstreamMemory::stref;
        delete ME::MEUpstreamMemory::stref;
        enabled = false;
    }
}