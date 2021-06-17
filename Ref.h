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
			upstreammemory upstream;
			upstream.deallocate(Ptr, sizeof(T));
		}

		void inc() { Count++; }
		void dec() { Count--; }

		T* Ptr;
		size_t Count;
	};
	template<typename T, typename upstreammemory = alloc_dealloc_UpstreamMemory> class Ref
	{
	public:
		Ref(nullptr_t)
			:ControlBlock(nullptr) {}

		template<typename U> Ref(const Ref<U>& other)
			: m_ControlBlock(other.m_ControlBlock) {}

		template<typename U> Ref(Ref<U>&& other)
			:m_ControlBlock(other.m_ControlBlock) 
		{
			other.m_ControlBlock = nullptr;
		}

		~Ref()
		{
			if (m_ControlBlock != nullptr)
			{
				if (m_ControlBlock->Count == 1)
				{
					upstreammemory upstream;
					m_ControlBlock->destruct();
					upstream.deallocate(m_Controlblock, sizeof(ControlBlock))
				}
				else
					m_ControlBlock->dec();
			}
		}

		Ref& operator=(const Ref& other)
		{
			Ref ref;
			ref.m_ControlBlock = other.m_ControlBlock;
			ref.m_ControlBlock->inc();
			return ref;
		}

		Ref& operator=(Ref&& other)
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
	};
	template<typename T, typename ...Args, typename upstreammemory = alloc_dealloc_UpstreamMemory> Ref<T, upstreammemory>& CreateRef(Args&& ...args) 
	{
		Ref<T, upstreammemory> ref;
		upstreammemory upstream;

		ref.m_ControlBlock = upstream.allocate(sizeof(ControlBlock));
		ref.m_ControlBlock->Count = 1;
		ref.m_ControlBlock->Ptr = upstream.allocate(sizeof(T));
		new (ref.m_ControlBlock->Ptr) T(args...);

		return ref;
	}
}
#endif // !REF