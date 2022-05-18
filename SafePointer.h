#pragma once

#include "MarsHeader.h"

namespace ME
{
	template<typename T>
	class SafePointer
	{
	public:
		SafePointer() = default;
		SafePointer(T* pointer, const size_t count)
			:m_Count(count), m_Pointer(pointer) {}

		template<typename U> SafePointer(const SafePointer<U>& other)
			: m_Count(other.m_Count), Pointer(reinterpret_cast<U*>(other.Pointer)) {}

		T* Get() { return m_Pointer; }
		T& operator*() { return *m_Pointer; }
		T* operator+(const size_t& left)
		{
			if (left < m_Count)
				return m_Pointer + left;
			return nullptr;
		}
		T* operator+(const unsigned int& left)
		{
			if (left < m_Count)
				return m_Pointer + left;
			return nullptr;
		}
		T* operator+(const int& left)
		{
			if (left < m_Count)
				return m_Pointer + left;
			return nullptr;
		}
		T& operator[] (const size_t& left)
		{
			ME_CORE_CRITICAL(left > m_Count, "Index out of bound!!");

			return *(m_Pointer + left);
		}
		T* operator->() { return m_Pointer; }
		template<typename U> operator U*() { return reinterpret_cast<U*>(m_Pointer); }
		template<typename U> bool operator==(const SafePointer<U>& left) { return m_Pointer == left.m_Pointer; }
		template<typename U> bool operator!=(const SafePointer<U>& left) { return m_Pointer != left.m_Pointer; }
	private:
		size_t m_Count;
		T* m_Pointer;
	};
}