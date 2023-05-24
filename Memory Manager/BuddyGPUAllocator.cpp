#include "BuddyGPUAllocator.h"
#include "MemoryManager.h"

namespace ME
{
	BuddyAllocator::BuddyAllocator()
		:m_Size(MAX_VERTEX_BUFFER)
	{
		vertex_id.push_back(0);
		index_id.push_back(0);
		v_head.push_back(alloc<MemoryChunk>());
		i_head.push_back(alloc<MemoryChunk>());

		v_head[0]->size = MAX_VERTEX_BUFFER;
		v_head[0]->filled = false;
		v_head[0]->next = nullptr;
	}
	BuddyAllocator::~BuddyAllocator()
	{
		MemoryChunk* cur = v_head[0];
		while (cur != nullptr)
		{
			MemoryChunk* temp = cur;
			cur = cur->next;

			dealloc(temp);
		}
	}
	GPUResponse BuddyAllocator::Allocate(const size_t& v_size, const size_t& i_size)
	{
		if (!v_size || !i_size)
			return GPUResponse();

		int v_offset = 0, i_offset = 0;
		{
			MemoryChunk* cur = v_head[0], * prev = nullptr;
			while (cur != nullptr)
			{
				if (cur->size >= v_size && !cur->filled)
				{
					if (prev != nullptr)
					{
						prev->size += v_size;
						cur->size -= v_size;
						if (cur->size < 1)
						{
							detachnextnode(prev);
							break;
						}
					}
				}
				prev = cur;
				cur = cur->next;
				v_offset += cur->size;
			}
		}

		{
			MemoryChunk* cur = i_head[0], * prev = nullptr;
			while (cur != nullptr)
			{
				if (cur->size >= i_size && !cur->filled)
				{
					if (prev != nullptr)
					{
						prev->size += i_size;
						cur->size -= i_size;
						if (cur->size < 1)
						{
							detachnextnode(prev);
							break;
						}
					}
				}
				prev = cur;
				cur = cur->next;
				i_offset += cur->size;
			}
		}

		return;
	}
	void BuddyAllocator::Deallocate(const GPUResponse& resouce, const size_t& v_size, const size_t& i_size)
	{

	}
	size_t BuddyAllocator::QueryUsedMemory()
	{
		return size_t();
	}
	size_t BuddyAllocator::QueryTotalMemory()
	{
		return size_t();
	}
	double BuddyAllocator::QueryFragmentation()
	{
		return 0.0;
	}
	void BuddyAllocator::detachnextnode(MemoryChunk* before)
	{
		if (before->next == nullptr)
			return;

		MemoryChunk* remove = before->next;
		before->next = remove->next;

		dealloc(remove);
		heal(before);
	}
	void BuddyAllocator::heal(MemoryChunk* before)
	{
		MemoryChunk* cur = before->next, * prev = before;

		while (cur != nullptr)
		{
			if (cur->filled && prev->filled)
			{
				prev->size += cur->size;
				prev->next = cur->next;
				dealloc(cur);
			}
			if (!cur->filled && !prev->filled)
			{
				prev->size += cur->size;
				prev->next = cur->next;
				dealloc(cur);
			}
			prev = prev->next;
			cur = prev->next;
		}
	}
}