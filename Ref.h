#ifndef REF
#define REF

#include "MemoryManager.h"

namespace ME
{
	/**
	 Ref is Reference smart pointer built over Allocator systems of MarsEngine. This class aloow the user to have 
	 multiple reference of a pointer.
	 FIX: Need to make a better implementation for UpstreamMemory initializer.
	**/
	template<typename T, typename upstreammemory = alloc_dealloc_UpstreamMemory> class ControlBlock
	{
	public:
		ControlBlock()
			:Ptr(nullptr), Count(0) {}

		void destruct() 
		{
			Ptr->~T();
			upstreammemory::stref->deallocate(Ptr, sizeof(T), "REF: Deallocating Object");
		}

		void inc() { Count++; }
		void dec() { Count--; }

		T* Ptr;
		size_t Count;
	};
	template<typename T, typename upstreammemory = alloc_dealloc_UpstreamMemory> class Ref
	{
	public:
		Ref()
			:m_ControlBlock(nullptr) {}

		Ref(nullptr_t)
			:ControlBlock(nullptr) {}

		template<typename U> Ref(const Ref<U, upstreammemory>& other)
		{
			m_ControlBlock = reinterpret_cast<ControlBlock<T, upstreammemory>*>(other.m_ControlBlock);
			m_ControlBlock->inc();
		}

		template<typename U> Ref(Ref<U, upstreammemory>&& other)
		{
			m_ControlBlock = reinterpret_cast<ControlBlock<T, upstreammemory>*>(other.m_ControlBlock);
			m_ControlBlock->inc();

			other.m_ControlBlock = nullptr;
		}

		~Ref()
		{
			if (m_ControlBlock != nullptr)
			{
				if (m_ControlBlock->Count == 1)
				{
					m_ControlBlock->destruct();
					upstreammemory::stref->deallocate(m_ControlBlock, sizeof(ControlBlock<T, upstreammemory>), "REF: Deallocating Control Block");
				}
				else
					m_ControlBlock->dec();
			}
		}

		template<typename U> Ref& operator=(const Ref<U, upstreammemory>& other)
		{
			Ref ref;
			ref.m_ControlBlock = other.m_ControlBlock;
			ref.m_ControlBlock->inc();
			return ref;
		}

		template<typename U> Ref& operator=(Ref<U, upstreammemory>&& other)
		{
			Ref ref;
			ref.m_ControlBlock = other.m_ControlBlock;
			other.m_ControlBlock = nullptr;
			return ref;
		}

		T& operator*() { return *m_ControlBlock->Ptr; }
		T* operator->() {return m_ControlBlock->Ptr; }
	private:
		ControlBlock<T, upstreammemory>* m_ControlBlock;
		friend Ref;
		template<typename T, typename ...Args, typename upstreammemory> friend auto CreateRef(Args&& ...args);
	};
	template<typename T, typename ...Args, typename upstreammemory = alloc_dealloc_UpstreamMemory> auto CreateRef(Args&& ...args) 
	{
		Ref<T, upstreammemory> ref;
		ref.m_ControlBlock = (ControlBlock<T, upstreammemory>*)(upstreammemory::stref->allocate(sizeof(ControlBlock<T, upstreammemory>), "REF: Allocating Control Block"));
		ref.m_ControlBlock->Count = 1;
		ref.m_ControlBlock->Ptr = (T*)(upstreammemory::stref->allocate(sizeof(T), "REF: Allocating Object"));
		new (ref.m_ControlBlock->Ptr) T(args...);

		return ref;
	}
}
#endif // !REF