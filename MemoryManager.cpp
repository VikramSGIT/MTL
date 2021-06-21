#include "MemoryManager.h"
#include "PoolAllocator.h"
//#include "Core/Logger.h"

#include <sstream>

namespace ME
{
	MemoryManager* MemoryManager::Allocator = nullptr;
	malloc_stdfree_UpstreamMemory* malloc_stdfree_UpstreamMemory::stref = nullptr;
	alloc_dealloc_UpstreamMemory* alloc_dealloc_UpstreamMemory::stref = nullptr;
	null_UpstreamMemory* null_UpstreamMemory::stref = nullptr;

	void InitAllocator() 
	{ 
		malloc_stdfree_UpstreamMemory::stref = new malloc_stdfree_UpstreamMemory;
		alloc_dealloc_UpstreamMemory::stref = new alloc_dealloc_UpstreamMemory;
		null_UpstreamMemory::stref = new null_UpstreamMemory;
		MemoryManager::Allocator = new  PoolAllocator<malloc_stdfree_UpstreamMemory>();
	}

	void DeInitAllocator() 
	{
		reinterpret_cast<PoolAllocator<malloc_stdfree_UpstreamMemory>*>(MemoryManager::Allocator)->~PoolAllocator();
		delete malloc_stdfree_UpstreamMemory::stref;
		delete alloc_dealloc_UpstreamMemory::stref;
		delete null_UpstreamMemory::stref;
		delete MemoryManager::Allocator;
	}

	void* malloc_stdfree_UpstreamMemory::allocate(const size_t& size, const char* msg) 
	{
		if (msg == nullptr)
			msg = "";

		std::stringstream ss;
		ss << msg << " using malloc | AllocatedSize:" << size;
		ME_CORE_WARNING(ss.str());

		return malloc(size);
	}
	void* malloc_stdfree_UpstreamMemory::reallocate(void* end_ptr, const size_t& size, const char* msg)
	{
		if (msg == nullptr)
			msg = "";

		std::stringstream ss;
		ss << msg << " using realloc | ReallocatedSize: " << size;
		ME_CORE_WARNING(ss.str());

		return ::realloc(end_ptr, size);
	}
	void malloc_stdfree_UpstreamMemory::deallocate(void* ptr, const size_t& size, const char* msg) 
	{
		if (msg == nullptr)
			msg = "";

		std::stringstream ss;
		ss << msg << " using std::free | DeallocatedSize: " << size;
		ME_CORE_WARNING(ss.str());

		std::free(ptr);
	}
	void* alloc_dealloc_UpstreamMemory::allocate(const size_t& size, const char* msg) 
	{
		if (msg == nullptr)
			msg = "";

		void* ptr = (void*)alloc<char>(size);
		std::stringstream ss;
		ss << msg << " | AllocatedSize:" << size << " | " << ptr;
		ME_CORE_WARNING(ss.str());

		return ptr;
	}
	void* alloc_dealloc_UpstreamMemory::reallocate(void* end_ptr, const size_t& size, const char* msg)
	{
		if (msg == nullptr)
			msg = "";

		std::stringstream ss;
		ss << msg << " | ReallocatedSize:" << size;
		ME_CORE_WARNING(ss.str());

		return (void*)realloc<char>((char*)end_ptr, size);
	}
	void alloc_dealloc_UpstreamMemory::deallocate(void* end_ptr, const size_t& size, const char* msg) 
	{
		if (msg == nullptr)
			msg = "";

		std::stringstream ss;
		ss << msg << " | DeallocatedSize:" << size;
		dealloc(end_ptr, size);
		ME_CORE_WARNING(ss.str());
	}

	MemoryManager::MemoryManager(UpstreamMemory* upstreammemory)
		:m_UpstreamMemory(upstreammemory) {}
}