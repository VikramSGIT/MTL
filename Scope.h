#pragma once

#include "Memory/MemoryManager.h"
namespace ME
{
	template<typename T, typename upstreammemory = MEUpstreamMemory>
	class Scope
	{
	public:
		Scope()
			:Ptr(nullptr) {}
		
		Scope(nullptr_t)
			:Ptr(nullptr) {}

		~Scope()
		{
			if(Ptr != nullptr)
				(deleter*)Ptr->~deleter();
			upstreammemory::stref->deallocate(Ptr, "SCOPE: Deallocating scope");
		}

		T* get() noexcept { return Ptr; }
		T& operator*() { return *Ptr; }
		T* operator->() noexcept { return Ptr; }
		const T* get() const noexcept { return Ptr; }
		const T& operator*() const { return *Ptr; }
		const T* operator->() const noexcept { return Ptr; }
		Scope& operator=(const Scope&) = delete;
		Scope(const Scope&) = delete;
	private:
		T* Ptr;
		template<typename U> Scope(const Scope<U, upstreammemory>& other)
		{
			if (Ptr != nullptr)
				Ptr->~T();
			upstreammemory::stref->deallocate(Ptr, "SCOPE: Replacing old pointer");
			Ptr = other.Ptr;
			other.Ptr = nullptr;
		}
		template<typename U, typename ...Args, typename memory> Scope<U, memory> friend CreateScope(Args ...args);
	};
	template<typename U, typename ...Args, typename upstreammemory = MEUpstreamMemory> Scope<U, upstreammemory> CreateScope(Args ...args)
	{
		U* Ptr = (U*)(upstreammemory::stref->allocate(sizeof(U), "SCOPE: Initializing scope"));
		new (Ptr) U(args...);
		return Scope<U, upstreammemory>(Ptr);
	}
	template<typename T> using scope = Scope<T, MEUpstreamMemory>;
}