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
	template<typename T, typename upstreammemory = alloc_dealloc_UpstreamMemory> class Ref
	{
	public:
		Ref()
			:ptr_count(nullptr), Ptr(nullptr), m_UpstreamMemory(new upstreammemory) {}

		Ref(nullptr_t)
			:ptr_count(nullptr), Ptr(nullptr), m_UpstreamMemory(new upstreammemory) {}

		template<typename U> Ref(Ref<U>& ref)
		{
			Ptr = ref.Ptr;
			m_UpstreamMemory = ref.m_UpstreamMemory;
			ptr_count = ref.ptr_count;
			if (ptr_count != nullptr)
				*ptr_count += 1;
		}

		template<typename U> Ref(Ref<U>&& ref)
		{
			Ptr = ref.Ptr;
			m_UpstreamMemory = ref.m_UpstreamMemory;
			ptr_count = ref.ptr_count;
			if (ptr_count != nullptr)
				*ptr_count += 1;
		}

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
		/**
		* \brief Clears the handled Object.
		* Destroys the object and clears the handled Memory.
		**/
		void reset() 
		{ 
			destruct(Ptr);
			m_UpstreamMemory->deallocate(Ptr, sizeof(T)); 
		}
		T get() const noexcept { return *Ptr; }

		template<typename U> void swap(Ref<U>& ref)
		{
			T* ptr = Ptr;
			UpstreamMemory* upstream = m_UpstreamMemory;
			size_t* count = ptr_count;

			Ptr = ref.Ptr;
			m_UpstreamMemory = ref.m_UpstreamMemory;
			ptr_count = ref.ptr_count;

			ref.Ptr = ptr;
			ref.m_UpstreamMemory = upstream;
			ref.ptr_count = count;
		}

		T& operator*() noexcept { return *Ptr; }
		T* operator->() const noexcept { return Ptr; }
		template<typename U> bool operator==(const Ref<U>& right) { return Ptr == right.Ptr; }
		template<typename U> Ref& operator=(const Ref<U>& ref)
		{
			Ptr = ref.Ptr;
			m_UpstreamMemory = ref.m_UpstreamMemory;
			ptr_count = ref.ptr_count;
			if (ptr_count != nullptr)
				*ptr_count += 1;

			return *this;
		}
		template<typename U> Ref& operator=(Ref<U>&& ref)
		{
			Ptr = ref.Ptr;
			m_UpstreamMemory = ref.m_UpstreamMemory;
			ptr_count = ref.ptr_count;
			if (ptr_count != nullptr)
				*ptr_count += 1;

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