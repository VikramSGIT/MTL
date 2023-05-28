#ifndef ME_ISOLATE
#include "MarsHeader.h"
#include "Core/Logger.h"
#endif

#include "MemoryManager.h"
#include "PoolAllocator.h"

#include <sstream>

namespace ME
{
	MemoryManager* MemoryManager::Allocator = nullptr;
	STDUpstreamMemory* STDUpstreamMemory::stref = nullptr;
	MEUpstreamMemory* MEUpstreamMemory::stref = nullptr;

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
		STDUpstreamMemory::stref = new STDUpstreamMemory;
		MEUpstreamMemory::stref = new MEUpstreamMemory;
		MemoryManager::Allocator = new  PoolAllocator<STDUpstreamMemory>;
	}

	void DeInitAllocator() 
	{
#ifndef ME_ISOLATE
		ME_CORE_ERROR(MemoryManager::Allocator->getUsedMemory(), "MEMORYMANAGER: Memory leak detected (<={}/{} bytes)",
			MemoryManager::Allocator->getUsedMemory(), MemoryManager::Allocator->getMaxMemory())
#else
		if (MemoryManager::Allocator->getUsedMemory())
			std::cerr << "ERROR: MEMORYMANAGER: Memory leak detected (<=" << MemoryManager::Allocator->getUsedMemory() << "/"
			<< MemoryManager::Allocator->getMaxMemory() << " bytes)" << std::endl;
#endif
		((PoolAllocator<STDUpstreamMemory>*)MemoryManager::Allocator)->~PoolAllocator();
		delete MemoryManager::Allocator;
		delete STDUpstreamMemory::stref;
		delete MEUpstreamMemory::stref;
	}

	void* STDUpstreamMemory::allocate(const size_t& size, std::string msg)
	{
		void* ptr = malloc(size);
#ifdef ME_MEM_DEBUG
#ifndef ME_ISOLATE
		ME_CORE_FILTER
			ME_CORE_WARNING(msg + " using malloc | Allocated Size: {} | {} ", size, ptr);
#else
		std::cout << "WARNING: " << msg << " using malloc | Allocated Size: " << size << " | " << ptr << std::endl;
#endif
#endif
		return ptr;
	}
	void* STDUpstreamMemory::reallocate(void*& ptr, const size_t& size, std::string msg)
	{
#ifdef ME_MEM_DEBUG
#ifndef ME_ISOLATE

		ME_CORE_FILTER
			ME_CORE_WARNING(msg + " using realloc | Reallocated Size: {} ", size);
#else
		std::cout << "WARNING: " << msg << " using realloc | Reallocated Size: " << size << std::endl;
#endif
#endif
		return std::realloc(ptr, size);
	}
	void STDUpstreamMemory::deallocate(void* ptr, std::string msg)
	{
#ifdef ME_MEM_DEBUG
#ifndef ME_ISOLATE

		ME_CORE_FILTER
			ME_CORE_WARNING(msg + " using std::free | {} ", ptr);
#else
		std::cout << "WARNING: " << msg << " using std::free | " << ptr << std::endl;
#endif
#endif
		std::free(ptr);
	}
	void STDUpstreamMemory::message(const char* msg)
	{
#ifdef ME_MEM_DEBUG
#ifndef ME_ISOLATE
		ME_CORE_FILTER
			ME_CORE_WARNING(msg);
#else
		std::cout << "MESSAGE: " << msg << std::endl;
#endif
#endif
	}
	void* MEUpstreamMemory::allocate(const size_t& size, std::string msg)
	{
		ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");

		void* ptr = MemoryManager::Allocator->allocate(size);

#ifdef ME_MEM_DEBUG
#ifndef ME_ISOLATE

		ME_CORE_FILTER
			ME_CORE_WARNING(msg + " | Allocated Size: {} | {} ", size, ptr);
#else
		std::cout << "WARNING: " << msg << " | Allocated Size: " <<  size << " | " << ptr << std::endl;
#endif
#endif
		return ptr;
	}
	void* MEUpstreamMemory::reallocate(void *&ptr, const size_t& size, std::string msg)
	{
		ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");
		ptr = MemoryManager::Allocator->reallocate(ptr, size);
#ifdef ME_MEM_DEBUG
#ifndef ME_ISOLATE
		ME_CORE_FILTER
			ME_CORE_WARNING(msg + " | Reallocated Size: {} ", size);
#else
		std::cout << "WARNING: " << msg << " | Reallocated Size: " << size << std::endl;
#endif
#endif
		return ptr;
	}
	void MEUpstreamMemory::deallocate(void* ptr, std::string msg)
	{
#ifdef ME_MEM_DEBUG
#ifndef ME_ISOLATE
		ME_CORE_FILTER
			ME_CORE_WARNING(msg + " | Deallocating: {} ", ptr);
#else
		std::cout << "WARNING: " << msg << " | Deallocating: " << ptr << std::endl;
#endif
#endif
		ME_MEM_ERROR(MemoryManager::Allocator != nullptr, "Allocator not initialized!!");
		MemoryManager::Allocator->deallocate((void*)ptr);
	}

	void MEUpstreamMemory::forced_deallocated(void* ptr, std::string msg)
	{
#ifdef ME_MEM_DEBUG
#ifndef ME_ISOLATE
		ME_CORE_FILTER
			ME_CORE_WARNING(msg + " | Forced Deallocating: {} ", ptr);
#else
		std::cout << "WARNING: " << msg << " | Forced Deallocating: " << ptr << std::endl;
#endif
#endif
		MemoryManager::Allocator->forced_deallocate(ptr);
	}

	void MEUpstreamMemory::message(const char* msg)
	{
#ifdef ME_MEM_DEBUG
#ifndef ME_ISOLATE
		ME_CORE_FILTER
			ME_CORE_WARNING(msg);
#else
		std::cout << "MESSAGE: " << msg << std::endl;
#endif
#endif
	}

	MemoryManager::MemoryManager(UpstreamMemory* upstreammemory)
		:m_UpstreamMemory(upstreammemory) {}
}