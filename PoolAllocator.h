#ifndef POOLALLOCATOR
#define POOLALLOCATOR

#define ME_BUCKETSIZE 8 // make sure it to be powers of 2
#define ME_BUCKETCOUNT 1000
#define ME_POOLGUARDCOUNT 1 // used to identify the end of pool
#define ME_GUARDBUCKET -1011.01001011
#define ME_POOLGUARD -1.01101110

#include "MemoryManager.h"

#include <mutex>

#ifdef ME_MEM_DEEPDEBUG
#include <set>
#endif

namespace ME
{
	template<size_t bytes> union Bucket
	{
		char item[bytes];
		Bucket* next;
		double GUARD;
	};
	typedef Bucket<ME_BUCKETSIZE> bucket;

	template<typename upstreammemory = alloc_dealloc_UpstreamMemory>
	class PoolAllocator : public MemoryManager
	{
	public:
		PoolAllocator()
			:Size(ME_BUCKETCOUNT * sizeof(bucket)), Count(0), PoolCount(1), expanded(false)
		{
			bucket* cur = (bucket*)upstreammemory::stref->allocate(sizeof(bucket) * (ME_BUCKETCOUNT + ME_POOLGUARDCOUNT), "POOLALLOCATOR: Pool Initialization");
			m_Pools = (Pool*)upstreammemory::stref->allocate(sizeof(Pool), "POOLALLOCATOR: Initializating pool ledger");
			m_Pools->Head = cur;
			m_Pools->Size = ME_BUCKETCOUNT;
			(cur + ME_BUCKETCOUNT)->GUARD = ME_POOLGUARD;
			m_nextFree = cur;

			while (cur->GUARD != ME_POOLGUARD)
			{
				cur->next = cur + 1;
				cur++;
			}
		}

		~PoolAllocator() 
		{
			for (size_t i = 0; i < PoolCount; i++)
				upstreammemory::stref->deallocate(m_Pools[i].Head, "POOLALLOCATOR: Deallocating Pool");
			upstreammemory::stref->deallocate(m_Pools, "POOLALLOCATOR: Deallocating pool ledger");
		}

		virtual void* allocate(const size_t& size = ME_BUCKETSIZE) override
		{
			size_t continious = 0;
			bucket* cur = m_nextFree;
			// To find a contiguous pool of legnth "size"
			while (cur->GUARD != ME_POOLGUARD)
			{
				continious++;
				if (cur->next - cur != 1)
					continious = 0;

				if (continious * sizeof(bucket) >= (size + ME_BUCKETSIZE)) //extrabucket for guard
				{
#ifdef ME_MEM_DEEPDEBUG
					AllocationDatabase[reinterpret_cast<void*>(cur - (continious - 1 ))].insert(continious - 1);
#endif
					m_nextFree = cur->next;
					cur->GUARD = ME_GUARDBUCKET;

					Count += continious * sizeof(bucket);
					expanded = false;
					return reinterpret_cast<void*>(cur - (continious - 1));
				}
				cur = cur->next;
			}

			expand(size + ME_BUCKETSIZE);
			expanded = true;
			// using recursion to allocate with newly allocated pool.
			return allocate(size);
		}
		virtual void* reallocate(void *&ptr, const size_t& size) override
		{
			size_t continious = 0, filledbuckets = getBucketCount((bucket*)ptr);
			bucket* cur = (bucket*)ptr + filledbuckets + 1;
			if (cur == m_nextFree)
			{
				while (cur->GUARD != ME_POOLGUARD)
				{
					if (cur->next - cur != 1)
						break;
					else
						continious++;

					if (continious * sizeof(bucket) >= size)
					{
						m_nextFree = cur->next;
						memcpy((bucket*)ptr + filledbuckets, "0", 1); // removes guardbucket, removing deallocation confusions
						cur->GUARD = ME_GUARDBUCKET;
						Count += continious * sizeof(bucket);
#ifdef ME_MEM_DEEPDEBUG
						auto pt = AllocationDatabase[ptr].find(filledbuckets);
						AllocationDatabase[ptr].erase(pt);
						AllocationDatabase[ptr].insert(filledbuckets + continious);
#endif

						return ptr;
					}
					cur = cur->next;
				}
			}

			size_t total = filledbuckets * sizeof(bucket) + size;
			bucket* newmem = (bucket*)allocate(total);
			memcpy(newmem, ptr, filledbuckets * sizeof(bucket));
			deallocate(ptr);
			ptr = newmem;
			return ptr;
		}
		virtual void deallocate(void* ptr) noexcept override
		{
			if (ptr == nullptr)
				return;

			size_t size = 1;
			bucket* cur = reinterpret_cast<bucket*>(ptr);

			while (cur->GUARD != ME_GUARDBUCKET && cur->GUARD != ME_POOLGUARD)
			{
				cur->next = (cur + 1);
				cur = cur->next;
				size++;
			}
			cur->next = m_nextFree;
			m_nextFree = (bucket*)ptr;

#ifdef ME_MEM_DEEPDEBUG
			auto pt = AllocationDatabase[ptr].find(size - 1);
			if (pt != AllocationDatabase[ptr].end())
				AllocationDatabase[ptr].erase(pt);
			else
				AllocationDatabase[ptr].insert(1 - size);
#endif

			//ME_MEMERROR(belongs(cur), "Memory out of Bound!!"); Causing issues

			Count -= size * sizeof(bucket);
		}
		virtual void release() override
		{
			for (size_t i = 1; i < PoolCount; i++)
			{
				for (size_t j = 1; j < m_Pools[i - 1].Size; j++)
					m_Pools[i - 1].Head->next = m_Pools[i - 1].Head + j;

				// Connecting pools
				(m_Pools[i - 1].Head + m_Pools[i-1].Size)->next = m_Pools[i].Head;
			}
			// Setting last Pool's PoolGuard to nullptr
			(m_Pools[PoolCount - 1].Head + m_Pools[PoolCount - 1].Size)->next = nullptr;

			ME_CORE_WARNING("POOLALLOCATOR: Memory Realeased!!");
		}

		inline size_t getFreeMemory() const noexcept { return Size - Count; }
		inline size_t getMaxMemory() const noexcept { return Size; }
		inline size_t getUsedMemory() const noexcept { return Count; }
	private:
		struct Pool
		{
			bucket* Head;
			size_t Size;
		};

		size_t getBucketCount(bucket* ptr)
		{
			size_t count = 0;

			while (ptr->GUARD != ME_GUARDBUCKET)
			{
				ptr = ptr + 1;
				count++;
			}
			return count;
		}

		inline bucket* BucketCorrection(const void* ptr)
		{
			for (size_t i = 0; i < PoolCount; i++)
			{
				if (m_Pools[i].Head <= ptr && (m_Pools[i].Head + m_Pools[i].Size) >= ptr)
				{
					size_t offset = (((char*)ptr - (char*)m_Pools[i].Head) % ME_BUCKETSIZE);
					return (bucket*)((char*)ptr - offset);
				}
			}
			return nullptr;
		}

		void expand(const size_t& size,const bool& init = true)
		{
			size_t count = (size_t)std::ceil((double)size / sizeof(bucket)) + 1, bucketcount = ME_BUCKETCOUNT;
			ME_MEM_ERROR(!expanded, "Infite loop allocation detected!! {}", count);
			count = std::max(count, bucketcount);

			// Pool Expantion
			bucket* poolhead = (bucket*)upstreammemory::stref->allocate(sizeof(bucket) * (count + ME_POOLGUARDCOUNT), "POOLALLOCATOR: Allocating Extra Pool");
			bucket* cur = poolhead;
			// Pool Initialization
			(cur + count)->GUARD = ME_POOLGUARD;
			(cur + count - 1)->next = m_nextFree;

			while (cur->next != m_nextFree)
			{
				cur->next = cur + 1;
				cur++;
			}

			// Connecting Old to New Pool
			m_nextFree = poolhead;

			// Expanding PoolLedger
			Pool *newpool = (Pool*)upstreammemory::stref->allocate(sizeof(Pool) * (PoolCount + 1), "POOLALLOCATOR: Allocating new ledger");
			memcpy(newpool, m_Pools, sizeof(Pool) * PoolCount);
			(newpool + PoolCount)->Head = poolhead;
			(newpool + PoolCount)->Size = (count + ME_POOLGUARDCOUNT);
			upstreammemory::stref->deallocate(m_Pools, "POOLALLOCATOR: Deallocating old ledger");
			PoolCount++;

			m_Pools = newpool;
			Size += count * sizeof(bucket);
		}

		// A function that verifies that a memory segment belongs to the pool
		bool belongs(bucket* ptr)
		{
			for (int i = 0; i < m_Pools.PoolCount; i++)
				for (int j = 0; j < ME_BUCKETCOUNT; j++)
					if (ptr == (bucket*)m_Pools.LedgerHead[i] + (j * sizeof(bucket)))
						return true;

			return false;
		}

		bucket * m_nextFree;
		Pool* m_Pools; // a ledger for all pools
		size_t Size, Count, PoolCount;
		bool expanded;
#ifdef ME_MEM_DEEPDEBUG
		std::unordered_map<void*, std::set<long long>> AllocationDatabase;
		virtual const std::unordered_map<void*, std::set<long long>>& getAllocationRegistry() override { return AllocationDatabase; }
#endif 
	};
}
#endif // !POOLALLOCATOR