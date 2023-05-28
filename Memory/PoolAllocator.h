#pragma once

#define ME_BUCKETSIZE 8 // make sure it to be powers of 2
#define ME_BUCKETCOUNT 1000

#define ME_POOLGUARDCOUNT 2 // used to identify the start and end of pool
#define ME_POOLGUARD -1.01101110

#ifdef ME_ENABLE_OVERFLOW_CHECK
#define ME_BUCKETGUARDCOUNT 2 // used to identify the start and end of a allocated bucket
#define ME_GUARDBUCKET -1011.01001011
#else
#define ME_BUCKETGUARDCOUNT 0
#define ME_GUARDBUCKET
#endif

#include "MemoryManager.h"

#include <mutex>
#include <unordered_map>
#include <cmath>

namespace ME
{
	template<size_t bytes> union Bucket
	{
		char item[bytes];
		Bucket* next;
		double GUARD;
	};
	typedef Bucket<ME_BUCKETSIZE> bucket;

	template<typename upstreammemory = MEUpstreamMemory>
	class PoolAllocator : public MemoryManager
	{
	public:
		PoolAllocator()
			:Size(ME_BUCKETCOUNT * sizeof(bucket)), Count(0), PoolCount(1), expanded(false)
		{
			bucket* cur = (bucket*)upstreammemory::stref->allocate(sizeof(bucket) * (ME_BUCKETCOUNT + ME_POOLGUARDCOUNT), "POOLALLOCATOR: Pool Initialization");
			m_Pools = (Pool*)upstreammemory::stref->allocate(sizeof(Pool), "POOLALLOCATOR: Initializating pool ledger");

			(cur + ME_BUCKETCOUNT)->GUARD = ME_POOLGUARD;
			cur->GUARD = ME_POOLGUARD;
			cur += 1; // Moving away from POOLGUARD

			m_Pools->Head = cur;
			m_Pools->Size = ME_BUCKETCOUNT;
			m_nextFree = cur;

			while (cur->GUARD != ME_POOLGUARD)
			{
				cur->next = cur + 1;
				cur++;
			}
		}

		~PoolAllocator() 
		{
#ifdef ME_ENABLE_OVERFLOW_CHECK
			checkBufferOverflows();
#endif
			for (size_t i = 0; i < PoolCount; i++)
				upstreammemory::stref->deallocate(m_Pools[i].Head - 1, "POOLALLOCATOR: Deallocating Pool");
			upstreammemory::stref->deallocate(m_Pools, "POOLALLOCATOR: Deallocating pool ledger");
		}

		virtual void* allocate(const size_t& size = ME_BUCKETSIZE) override
		{

			//std::lock_guard<std::mutex>(m_Lock);

			size_t continious = 0;
			bucket* cur = m_nextFree;
			// To find a contiguous pool of length "size"
			while (cur->GUARD != ME_POOLGUARD)
			{
				continious++;
				if (cur->next - cur != 1)
					continious = 0;

				if (continious * sizeof(bucket) >= (size + (ME_BUCKETSIZE * ME_BUCKETGUARDCOUNT)))
				{
					m_nextFree = cur->next;
#ifdef ME_ENABLE_OVERFLOW_CHECK
					m_AllocationBook[cur - (continious - 2)] = size;
					cur->GUARD = ME_GUARDBUCKET;
					(cur - (continious - 1))->GUARD = ME_GUARDBUCKET;

					Count += continious * sizeof(bucket);
					continious--; // offset guardbytes
#else
					m_AllocationBook[cur - (continious - 1)] = size;
					Count += continious * sizeof(bucket);
#endif
					expanded = false;
					return reinterpret_cast<void*>(cur - (continious - 1));
				}
				cur = cur->next;
			}

			expand(size + (ME_BUCKETSIZE * ME_BUCKETCOUNT));
			expanded = true;
			// using recursion to allocate with newly allocated pool.
			return allocate(size);
		}
		virtual void* reallocate(void *&ptr, const size_t& size) override
		{

			//std::lock_guard<std::mutex>(m_Lock);

			size_t continious = 0, filledbuckets = getBucketCount(ptr);
			if (m_AllocationBook[ptr] + size <= filledbuckets * ME_BUCKETSIZE) { // no need for unknown value protection, getBucketCount checks for it.
				upstreammemory::stref->message("POOLALLOCATOR: Skipping reallocation request as bucket is not full.");
				m_AllocationBook[ptr] += size;
				return ptr;
			}
#ifdef ME_ENABLE_OVERFLOW_CHECK
			bucket* cur = (bucket*)ptr + filledbuckets + 1; //offset for guard bucket
#else
			bucket* cur = (bucket*)ptr + filledbuckets;
#endif
			if (cur == m_nextFree) {
				while (cur->GUARD != ME_POOLGUARD) {
					if (cur->next - cur != 1) // Checks if cur and the next bucket are adjacent
						break;
					else
						continious++;

					if (continious * sizeof(bucket) >= size) {
						m_nextFree = cur->next;
#ifdef ME_ENABLE_OVERFLOW_CHECK
						memset((bucket*)ptr + filledbuckets, 0, ME_BUCKETSIZE); // removes guardbucket, security purposes
						cur->GUARD = ME_GUARDBUCKET;
#endif
						m_AllocationBook[ptr] += size;
						Count += continious * sizeof(bucket);
						return ptr;
					}
					cur = cur->next;
				}
			}

			void* newmem = allocate(filledbuckets * ME_BUCKETSIZE + size);
			memcpy(newmem, ptr, filledbuckets * ME_BUCKETSIZE);
			deallocate(ptr);
			ptr = newmem;
			return ptr;
		}
		virtual void deallocate(void* ptr) override
		{

			//std::lock_guard<std::mutex>(m_Lock);

			if (ptr == nullptr)
				return;

			size_t filledbuckets = getBucketCount(ptr);

#ifdef ME_ENABLE_OVERFLOW_CHECK
			checkBufferOverflow(ptr);
			
			bucket* cur = reinterpret_cast<bucket*>(ptr) - 1;
			bucket* end = reinterpret_cast<bucket*>(ptr) + filledbuckets;
#else
			bucket* cur = reinterpret_cast<bucket*>(ptr);
			bucket* end = cur + filledbuckets - 1;
#endif
			size_t size = 1;
			while (cur != end && cur->GUARD != ME_POOLGUARD)
			{
				cur->next = (cur + 1);
				cur = cur->next;
				size++;
			}
			cur->next = m_nextFree;
#ifdef ME_ENABLE_OVERFLOW_CHECK
			m_nextFree = reinterpret_cast<bucket*>(ptr) - 1;
#else
			m_nextFree = reinterpret_cast<bucket*>(ptr);
#endif
			//ME_MEMERROR(belongs(cur), "Memory out of Bound!!"); Causing issues

			Count -= size * sizeof(bucket);
			m_AllocationBook.erase(ptr);
		}
		virtual void forced_deallocate(void* ptr) override {
			if (ptr == nullptr) return;

			bucket* p = reinterpret_cast<bucket*>(ptr);
			(p - 1)->GUARD = ME_GUARDBUCKET;
			(p + getBucketCount(ptr))->GUARD = ME_GUARDBUCKET;
			deallocate(ptr);
		}
		virtual void release() override
		{

			//std::lock_guard<std::mutex>(m_Lock);

			for (size_t i = 1; i < PoolCount; i++)
			{
				for (size_t j = 1; j < m_Pools[i - 1].Size; j++)
					m_Pools[i - 1].Head->next = m_Pools[i - 1].Head + j;

				// Connecting pools
				(m_Pools[i - 1].Head + m_Pools[i-1].Size)->next = m_Pools[i].Head;
			}
			// Setting last Pool's PoolGuard to nullptr
			(m_Pools[PoolCount - 1].Head + m_Pools[PoolCount - 1].Size)->next = nullptr;
#ifndef ME_ISOLATE
			ME_CORE_WARNING("POOLALLOCATOR: Memory Realeased!!");
#else
			std::cout << "WARNING: POOLALLOCATOR: Memory Realeased!!" << std::endl;
#endif
		}

		virtual size_t getFreeMemory() noexcept override {

			//std::lock_guard<std::mutex>(m_Lock);

			return Size - Count; 
		}
		virtual size_t getMaxMemory() noexcept override {

			//std::lock_guard<std::mutex>(m_Lock);

			return Size; 
		}
		virtual size_t getUsedMemory() noexcept override {

			//std::lock_guard<std::mutex>(m_Lock);

			return Count; 
		}
	private:
		struct Pool
		{
			bucket* Head;
			size_t Size;
		};

		size_t getBucketCount(void* ptr)
		{
			auto it = m_AllocationBook.find(ptr);
			ME_MEM_ERROR(it != m_AllocationBook.end(), "Invalid pointer found!!")

			return std::ceilf(it->second * 1.0f/ME_BUCKETSIZE);
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
			count = std::max(count, bucketcount);
#ifndef ME_ISOLATE
			ME_MEM_ERROR(expanded, "Infite loop allocation detected!! {}", count);
#else
			if (expanded)
				std::cerr << "Infite loop allocation detected!! " << std::endl;
#endif

			// Pool Expantion
			bucket* poolhead = (bucket*)upstreammemory::stref->allocate(sizeof(bucket) * (count + ME_POOLGUARDCOUNT), "POOLALLOCATOR: Allocating Extra Pool");
			// Pool Initialization
			(poolhead + count)->GUARD = ME_POOLGUARD;
			poolhead->GUARD = ME_POOLGUARD;
			poolhead++; // guard offset

			bucket* cur = poolhead;

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

		std::unordered_map<void*, size_t> m_AllocationBook;

		std::mutex m_Lock;
#ifdef ME_ENABLE_OVERFLOW_CHECK
		void checkBufferOverflow(void* ptr) {
			bool front = ((bucket*)(ptr) - 1)->GUARD == ME_GUARDBUCKET;
			bool back = ((bucket*)(ptr) + getBucketCount(ptr))->GUARD == ME_GUARDBUCKET;
			ME_MEM_ERROR(front && back, "POOLALLOCATOR: Buffer overflow detected!!");
		}

		void checkBufferOverflows() {
			for (auto it : m_AllocationBook) {
				bool front = ((bucket*)(it.first) - 1)->GUARD == ME_GUARDBUCKET;
				bool back = ((bucket*)(it.first) + (size_t)std::ceilf(it.second * 1.0f/ME_BUCKETSIZE))->GUARD == ME_GUARDBUCKET;
				ME_MEM_ERROR(front && back, "POOLALLOCATOR: Buffer overflow detected!!");
			}
		}
#endif
	};
}