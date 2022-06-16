#include "MarsHeader.h"

#include "MemoryManager.h"
#include "PoolAllocator.h"
#include "Core/Logger.h"

#include <sstream>

namespace ME
{
	MemoryManager* MemoryManager::Allocator = nullptr;
	malloc_stdfree_UpstreamMemory* malloc_stdfree_UpstreamMemory::stref = nullptr;
	alloc_dealloc_UpstreamMemory* alloc_dealloc_UpstreamMemory::stref = nullptr;
	null_UpstreamMemory* null_UpstreamMemory::stref = nullptr;

	void InitAllocator()
	{
#ifdef ME_MEM_DEBUG
		ME_CORE_WARNING("Memory Debuging enabled!!");
#endif
#ifdef ME_MEM_DEBUG_2
		ME_CORE_WARNING("Level-2 Memory debugging enabled!!");
#endif
#ifdef ME_MEM_DEEPDEBUG
		ME_CORE_WARNING("Deep Memory debugging enabled!!");
#endif
		malloc_stdfree_UpstreamMemory::stref = new malloc_stdfree_UpstreamMemory;
		alloc_dealloc_UpstreamMemory::stref = new alloc_dealloc_UpstreamMemory;
		null_UpstreamMemory::stref = new null_UpstreamMemory;
		MemoryManager::Allocator = new  PoolAllocator<malloc_stdfree_UpstreamMemory>;
	}

	void DeInitAllocator() 
	{
		ME_CORE_ERROR(MemoryManager::Allocator->getUsedMemory(), "MEMORYMANAGER: Memory leak detected (<={}/{} bytes)",
			MemoryManager::Allocator->getUsedMemory(), MemoryManager::Allocator->getMaxMemory())
#ifdef ME_MEM_DEEPDEBUG
		auto& registry = MemoryManager::Allocator->getAllocationRegistry();
		if (registry.size())
		{
			size_t size = 0;
			for (auto& a : registry)
			{
				if (a.second.size())
				{
					std::stringstream ss;
					for (auto& lst : a.second)
					{
						ss << lst << " ";
						if(size < 0)
							size += lst;
					}
					ME_CORE_ERROR(true, "MEMORYMANAGER: {} : {} ", a.first, ss.str());
				}
			}
			ME_CORE_ERROR(true, "MEMORYMANAGER: {} bytes of leak found!!", size);
		}
		
#endif

		((PoolAllocator<malloc_stdfree_UpstreamMemory>*)MemoryManager::Allocator)->~PoolAllocator();
		delete MemoryManager::Allocator;
		delete malloc_stdfree_UpstreamMemory::stref;
		delete alloc_dealloc_UpstreamMemory::stref;
		delete null_UpstreamMemory::stref;
	}

	void* malloc_stdfree_UpstreamMemory::allocate(const size_t& size, std::string msg) 
	{
		void* ptr = malloc(size);
#ifdef ME_MEM_DEBUG
		ME_CORE_FILTER
			ME_CORE_WARNING(msg + " using malloc | Allocated Size: {} | {} ", size, ptr);
#endif
		return ptr;
	}
	void* malloc_stdfree_UpstreamMemory::reallocate(void*& ptr, const size_t& size, std::string msg)
	{
#ifdef ME_MEM_DEBUG
		ME_CORE_FILTER
			ME_CORE_WARNING(msg + " using realloc | Reallocated Size: {} ", size);
#endif
		return std::realloc(ptr, size);
	}
	void malloc_stdfree_UpstreamMemory::deallocate(void* ptr, std::string msg) 
	{
#ifdef ME_MEM_DEBUG
		ME_CORE_FILTER
			ME_CORE_WARNING(msg + " using std::free | {} ", ptr);
#endif
		std::free(ptr);
	}
	void* alloc_dealloc_UpstreamMemory::allocate(const size_t& size, std::string msg) 
	{
		void* ptr = (void*)alloc<char>(size);

#ifdef ME_MEM_DEBUG
		ME_CORE_FILTER
			ME_CORE_WARNING(msg + " | Allocated Size: {} | {} ", size, ptr);
#endif
		return ptr;
	}
	void* alloc_dealloc_UpstreamMemory::reallocate(void *&ptr, const size_t& size, std::string msg)
	{
#ifdef ME_MEM_DEBUG
		ME_CORE_FILTER
			ME_CORE_WARNING(msg + " | Reallocated Size: {} ", size);
#endif
		realloc<char>((char*&)ptr, size);
		return ptr;
	}
	void alloc_dealloc_UpstreamMemory::deallocate(void* ptr, std::string msg) 
	{
#ifdef ME_MEM_DEBUG
		ME_CORE_FILTER
			ME_CORE_WARNING(msg + " | {} ", ptr);
#endif
		dealloc(ptr);
	}

	MemoryManager::MemoryManager(UpstreamMemory* upstreammemory)
		:m_UpstreamMemory(upstreammemory) {}
}