#ifndef REF
#define REF

#include<type_traits>

#include "MemoryManager.h"

namespace ME
{
	template<typename T, typename upstreammemory = alloc_dealloc_UpstreamMemory> class Ref
	{
	public:
		Ref()
			:ptr_count(nullptr), Ptr(nullptr), m_UpstreamMemory(new upstreammemory) {}

		template<typename U>
		Ref(Ref<U>& Reference)
		{
			swap(Reference);
		}

		template<typename U>
		Ref(Ref<U>&& Reference)
		{
			swap(Reference);
		}

		template<typename same> bool check(same, same) { return same; }
		template<typename left, typename right> bool check(left, right) { return false; }

		~Ref()
		{
			if (ptr_count != nullptr)
			{
				if (*ptr_count == 1) 
				{
					destruct(Ptr);
					m_UpstreamMemory->deallocate(Ptr, sizeof(T), "REF: Deinitializing ref");
					m_UpstreamMemory->deallocate(ptr_count, sizeof(size_t), "REF: Deinitializing ref count");
					delete m_UpstreamMemory;
				}
				else 
				{
					*ptr_count -= 1;
				}
			}
		}

		void reset() { m_UpstreamMemory->deallocate(Ptr); }
		T get() const noexcept { return *Ptr; }

		template<typename U>
		void swap(Ref<U>& ref)
		{
			this->~Ref();

			this->Ptr = ref.Ptr;
			this->m_UpstreamMemory = ref.m_UpstreamMemory;
			if (ref.ptr_count != nullptr)
			{
				this->ptr_count = ref.ptr_count;
				*this->ptr_count += 1;
			}
		}

		T& operator*() noexcept { return *Ptr; }
		T* operator->() const noexcept { return Ptr; }

		template<typename U>
		Ref<T>& operator=(Ref<U>& ref)
		{
			swap(ref);
			return *this;
		}
	private:
		void destruct(T* ptr)
		{
			ptr->~T();
		}

		size_t* ptr_count;
		T* Ptr;
		UpstreamMemory* m_UpstreamMemory;
		template<typename T, typename ...Args, typename upstreammemory> friend Ref<T> CreateRef(Args&& ...args);
		friend class Ref;
	};
	template<typename T, typename ...Args, typename upstreammemory = alloc_dealloc_UpstreamMemory> Ref<T> CreateRef(Args&& ...args) 
	{
		Ref<T> res;

		res.m_UpstreamMemory = new upstreammemory;
		res.Ptr = (T*)(res.m_UpstreamMemory->allocate(sizeof(T), "REF: Initializing ref"));
		new (res.Ptr) T(args...);

		res.ptr_count = (size_t*)(res.m_UpstreamMemory->allocate(sizeof(size_t), "REF: Initializing ref count"));
		*res.ptr_count = 1;
		return res;
	}
}
#endif // !REF