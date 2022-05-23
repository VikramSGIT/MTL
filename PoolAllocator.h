#ifndef POOLALLOCATOR
#define POOLALLOCATOR

#define ME_BUCKETSIZE 8 // make sure it to be powers of 2
#define ME_BUCKETCOUNT 1000
#define ME_BUCKETGUARD 1 // used to identify the end of bucket pool

#include "MemoryManager.h"

#ifdef ME_MEM_DEEPDEBUG
#include <set>
#endif

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
			:Size(ME_BUCKETCOUNT * sizeof(bucket)), Count(0)
		{
			bucket* PoolHead = (bucket*)upstreammemory::stref->allocate(sizeof(bucket) * (ME_BUCKETCOUNT + ME_BUCKETGUARD), "POOLALLOCATOR: Pool Initialization");
			*m_Pools.LedgerHead = PoolHead;
			*m_Pools.PoolSize = ME_BUCKETCOUNT + ME_BUCKETGUARD;

			m_nextFree = PoolHead;

			for (size_t i = 0; i < ME_BUCKETCOUNT; i++)
				new (PoolHead + i) bucket* (PoolHead + i + 1);

			new (PoolHead + ME_BUCKETCOUNT)  bucket*(nullptr);
		}

		~PoolAllocator() 
		{
			for (size_t i = 0; i < m_Pools.PoolCount; i++)
				upstreammemory::stref->deallocate(m_Pools[i], sizeof(bucket) * m_Pools.PoolSize[i], "POOLALLOCATOR: Deallocating Pool");
		}

		virtual void* allocate(const size_t& size = ME_BUCKETSIZE) override
		{
			std::shared_lock<std::shared_mutex> lock(mutex);

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
					Count += continious * sizeof(bucket);
#ifdef ME_MEM_HARDDEBUG
					AllocationDatabase[reinterpret_cast<void*>(temp - continious)].insert(size);
#endif
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
			size_t count = (size_t)std::ceil((double)size / sizeof(bucket));

			for (size_t i = 0; i <= count; i++)
			{
				if (cur->next - cur != 1)
					break;

				if (i == count)
				{
					m_nextFree = cur;
					Count += count * sizeof(bucket);
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

#ifdef ME_MEM_HARDDEBUG
			auto pt = AllocationDatabase[ptr].find(size);
			if (pt != AllocationDatabase[ptr].end())
				AllocationDatabase[ptr].erase(pt);
			else
				AllocationDatabase[ptr].insert(-size);
#endif
			size_t count = (size_t)std::ceil((double)size / sizeof(bucket));
			bucket* cur = reinterpret_cast<bucket*>(ptr);

			//ME_MEMERROR(belongs(cur), "Memory out of Bound!!"); Causing issues

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

			Count -= count * sizeof(bucket);
		}
		virtual void release() override
		{
			for (size_t i = 1; i < m_Pools.PoolCount; i++)
			{
				for (size_t j = 1; j < m_Pools.PoolSize[i - 1]; j++)
					m_Pools[i - 1]->next = m_Pools[i - 1] + j;

				// Connecting pools
				(m_Pools[i - 1] + m_Pools.PoolSize[i - 1])->next = m_Pools[i];
			}
			// Setting last Pool's PoolGuard to nullptr
			(m_Pools[m_Pools.PoolCount - 1] + m_Pools.PoolSize[m_Pools.PoolCount - 1])->next = nullptr;

			ME_CORE_WARNING("POOLALLOCATOR: Memory Realeased!!");
		}

		inline size_t getFreeMemory() const noexcept { return Size - Count; }
		inline size_t getMaxMemory() const noexcept { return Size; }
		inline size_t getUsedMemory() const noexcept { return Count; }
	private:

		struct PoolLedger
		{
			PoolLedger()
				:PoolCount(1)
			{
				LedgerHead = (bucket**)upstreammemory::stref->allocate(sizeof(bucket*), "POOLALLOCATOR: Initialising PoolLedger");
				PoolSize = (size_t*)upstreammemory::stref->allocate(sizeof(size_t), "POOLALLOCATOR: Initialising PoolLedger Count");
			}
			~PoolLedger()
			{
				upstreammemory::stref->deallocate(LedgerHead, sizeof(bucket) * PoolCount, "POOLALLOCATOR: Deallocating Ledger");
				upstreammemory::stref->deallocate(PoolSize, sizeof(size_t) * PoolCount, "POOLALLOCATOR: Deallocating Count");
			}

			void Expand(bucket* poolhead, const size_t poolsize)
			{
				bucket** pool = (bucket**)upstreammemory::stref->allocate(sizeof(bucket*) * (PoolCount + 1), "POOLALLOCATOR: Expanding Ledger");
				memcpy(pool, LedgerHead, sizeof(bucket) * PoolCount);
				upstreammemory::stref->deallocate(LedgerHead, sizeof(bucket) * PoolCount, "POOLALLOCATOR: Deallocating old Ledger");
				*(pool + PoolCount) = poolhead;
				LedgerHead = pool;

				size_t* countexpand = (size_t*)upstreammemory::stref->allocate(sizeof(size_t) * (PoolCount + 1), "POOLALLOCATOR: Expanding Ledger Count");
				memcpy(countexpand, PoolSize, sizeof(size_t) * PoolCount);
				upstreammemory::stref->deallocate(PoolSize, sizeof(size_t) * PoolCount, "POOLALLOCATOR: Deallocating old Count");
				*(countexpand + PoolCount) = poolsize;
				PoolSize = countexpand;

				PoolCount++;
			}

			bucket* operator[](const int& index) { return *(LedgerHead + index); }

			size_t PoolCount;
			bucket** LedgerHead;
			size_t* PoolSize;
		};

		void expand(const size_t& size)
		{
			size_t count = (size_t)std::ceil((double)size / ME_BUCKETSIZE);
			count = std::max(ME_BUCKETCOUNT, ME_BUCKETCOUNT);

			// Pool Expantion
			bucket* newmem = (bucket*)upstreammemory::stref->allocate(sizeof(bucket) * (count + ME_BUCKETGUARD), "POOLALLOCATOR: Allocating Extra Pool");

			// Pool Initialization
			for (size_t i = 0; i < count; i++)
				new (newmem + i) bucket* (newmem + i + 1);
			new (newmem + count) bucket* (m_nextFree);

			// Connecting Old to New Pool
			m_nextFree = newmem;

			// Expanding PoolLedger
			m_Pools.Expand(newmem, count + ME_BUCKETGUARD);
			
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
		PoolLedger m_Pools; // a ledger for all pools
		size_t Size, Count;
		std::shared_mutex mutex;
#ifdef ME_MEM_DEEPDEBUG
		std::unordered_map<void*, std::set<long long>> AllocationDatabase;
		virtual const std::unordered_map<void*, std::set<long long>>& getAllocationRegistry() override { return AllocationDatabase; }
#endif 
	};
}
#endif // !POOLALLOCATOR