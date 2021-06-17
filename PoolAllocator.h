#ifndef POOLALLOCATOR
#define POOLALLOCATOR

#define ME_BUCKETSIZE 8 // make sure it to be powers of 2
#define ME_BUCKETCOUNT 1000
#define ME_BUCKETGUARD 1 // used to identify the end of bucket pool

#include "MemoryManager.h"

namespace ME
{

	template<size_t bytes>
	union Bucket
	{
		char item[bytes];
		Bucket* next;
	};

	typedef Bucket<ME_BUCKETSIZE> bucket;

	template<typename upstreammemory = alloc_dealloc_UpstreamMemory>
	class PoolAllocator : public MemoryManager
	{
	public:
		PoolAllocator()
			:Size(ME_BUCKETCOUNT), Count(ME_BUCKETCOUNT), MemoryManager(new upstreammemory),
			m_PoolCount(1)
		{
			m_PoolHead = (bucket*)m_UpstreamMemory->allocate(sizeof(bucket) * (ME_BUCKETCOUNT + ME_BUCKETGUARD), "POOLALLOCATOR: Pool Initialization");
			m_Pools = (bucket**)m_UpstreamMemory->allocate(sizeof(bucket**), "POOLALLOCATOR: Ledger Initialization");
			new (m_Pools) bucket* (m_PoolHead);

			m_nextFree = m_PoolHead;
			m_PoolEnd = m_PoolHead + Count;

			for (size_t i = 0; i < ME_BUCKETCOUNT; i++)
			{
				new (m_PoolHead + i) bucket* (m_PoolHead + i + 1);
			}
			new (m_PoolHead + ME_BUCKETCOUNT)  bucket*(nullptr);
		}

		~PoolAllocator() 
		{
			for (size_t i = 0; i < m_PoolCount; i++)
				m_UpstreamMemory->deallocate(*(m_Pools + i), sizeof(bucket) * (ME_BUCKETCOUNT + ME_BUCKETGUARD), "POOLALLOCATOR: Deallocating pool");
			m_UpstreamMemory->deallocate((bucket*)m_Pools, sizeof(bucket*) * m_PoolCount, "POOLALLOCATOR: Deallocating pool ledger");
			delete m_UpstreamMemory;
		}

		virtual void* allocate(const size_t& size = ME_BUCKETSIZE) override
		{

			std::shared_lock<std::shared_mutex> lock(mutex);
			size_t jize = size;
			size_t continious = 0;
			bucket* cur = m_nextFree;
			// To find a contiguous pool of legnth "size"
			while (cur != nullptr)
			{
				bucket* temp = nullptr;
				if(cur->next != nullptr)
					temp = cur->next; // needed a fix here

				if (temp - cur == 1)
					continious++;
				else
					continious = 0;
				if (continious * sizeof(bucket) >= size)
				{
					m_nextFree = temp;
					Size -= continious;
					return reinterpret_cast<void*>(temp - continious);
				}
				cur = temp;
			}

			expand(size);

			// using recursion to allocate with newly allocated pool.
			return allocate(size);
		}
		virtual void* verified_allocate(void* end_ptr, const size_t& size) override
		{

			std::shared_lock<std::shared_mutex> lock(mutex);

			bucket* cur = reinterpret_cast<bucket*>(end_ptr);
			double bucketcount = (double)size / sizeof(bucket);
			size_t count = 0;

			if (bucketcount > size / sizeof(bucket))
				count = static_cast<size_t>(bucketcount + 1);
			else
				count = static_cast<size_t>(bucketcount);

			for (size_t i = 0; i <= count; i++)
			{
				if (cur->next - cur != 1)
					break;

				if (i == count)
				{
					m_nextFree = cur;
					Size -= count;
					return end_ptr;
				}
				cur = cur->next;
			}
			return nullptr;
		}
		virtual void deallocate(void* ptr, const size_t& size) noexcept override
		{
			std::shared_lock<std::shared_mutex> lock(mutex);

			if (!size || ptr == nullptr)
				return;


			double bucketcount = (double)size / sizeof(bucket);
			size_t count = 0;

			if (bucketcount > size / sizeof(bucket))
				count = static_cast<size_t>(bucketcount + 1);
			else
				count = static_cast<size_t>(bucketcount);

			bucket* cur = reinterpret_cast<bucket*>(ptr);

			ME_MEMERROR(belongs(cur) && belongs(cur + count), "Memory out of Bound!!");

			for (size_t i = 1; i <= count; i++)
			{
				if (i == count)
				{
					cur->next = m_nextFree;
					m_nextFree = reinterpret_cast<bucket*>(ptr);
				}
				else
					new (cur) bucket* (cur + 1);

				cur = cur->next;
			}

			Size += count;
		}
		virtual void release() override
		{

			// FIX: Add a way to deallocate the other pools

			m_nextFree = m_PoolHead;

			for (size_t i = 1; i < ME_BUCKETCOUNT; i++)
				(m_PoolHead + (i - 1))->next = (m_PoolHead + i);
		}

		inline size_t getFreeMemory() const noexcept { return Size * ME_BUCKETSIZE; }
	private:

		void expand(const size_t& size)
		{
			size_t count = 0;
			if (size > ME_BUCKETCOUNT * sizeof(bucket))
			{
				float bucketcount = (float)size / sizeof(bucket);

				if (bucketcount > size / sizeof(bucket))
					count = static_cast<size_t>(bucketcount + 1);
				else
					count = static_cast<size_t>(bucketcount);
			}
			else
			{
				count = ME_BUCKETCOUNT;
			}

			// Pool Expantion
			bucket* expand = (bucket*)m_UpstreamMemory->allocate(sizeof(bucket) * (count + ME_BUCKETGUARD), "POOLALLOCATOR: Allocating Extra Pool");
			Count += count;
			Size += count;

			// Pool Initialization
			for (size_t i = 0; i < count; i++)
				new (expand + i) bucket* (expand + i + 1);
			new (expand + count) bucket* (m_nextFree);

			// Connecting Old to New Pool
			m_nextFree = expand;

			bucket** temp = (bucket**)m_UpstreamMemory->allocate(sizeof(bucket**) * (m_PoolCount + 1), "POOLALLOCATOR: Expanding Leadger");
			// Copying old expanded pool pointers
			for(size_t i = 0; i < m_PoolCount; i++)
				new (temp + i) bucket* (*(m_Pools + i));
			new (temp + m_PoolCount) bucket* (expand); // Copying the new pool pointer
			m_UpstreamMemory->deallocate(m_Pools, sizeof(bucket*) * m_PoolCount, "POOLALLOCATOR: Deallocating old leadger");
			m_Pools = temp;
			m_PoolCount++;
		}

		// A function that verifies that a memory segment belongs to the pool
		// FIX: A way to verify multiple pools
		bool belongs(bucket* ptr)
		{
			char* pos = reinterpret_cast<char*>(ptr);
			long long cond1 = (pos - (char*)m_PoolHead), cond2 = ((char*)m_PoolEnd - pos);
			if (cond1 >= 0 && cond2 >= 0 && cond1 % ME_BUCKETSIZE == 0)
				return true;
			return false;
		}

		bucket* m_PoolHead, * m_PoolEnd, * m_nextFree, ** m_Pools; // a ledger for all pools
		size_t Size, Count, m_PoolCount;
		std::shared_mutex mutex;
	};
}
#endif // !POOLALLOCATOR