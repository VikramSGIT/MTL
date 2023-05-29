#pragma once

#ifndef ME_VECTOR
#define ME_VECTOR
#endif

#include "Memory/MemoryManager.h"

#include <mutex>
#include <initializer_list>
#include <cmath>

#define ME_DEFAULT_VECTOR_SIZE 3
#define ME_VECTOR_SCALE 0.5f

namespace ME
{
	template<typename T, typename upstreammemory = MEUpstreamMemory>
	class Vector
	{
	public:
		Vector()
			:m_Capacity(ME_DEFAULT_VECTOR_SIZE), m_Size(0)
		{
			m_Head = (T*)upstreammemory::stref->allocate(sizeof(T) * m_Capacity);
			upstreammemory::stref->message("VECTOR: Initialization");
		}
		Vector(const std::initializer_list<T>& list)
			:m_Size(list.size())
		{
			m_Capacity = std::ceilf(m_Size + m_Size * ME_VECTOR_SCALE);
			m_Head = (T*)upstreammemory::stref->allocate(sizeof(T) * (m_Capacity));
			upstreammemory::stref->message("VECTOR: Initialization using list");

			T* it = m_Head;
			for (T const& i : list)
				*it++ = i;
		}
		Vector(const Vector& vector)
			:m_Size(vector.size())
		{
			m_Capacity = std::ceilf(m_Size + m_Size * ME_VECTOR_SCALE);
			m_Head = (T*)upstreammemory::stref->allocate(sizeof(T) * (m_Capacity));
			upstreammemory::stref->message("VECTOR: Copying");
			memcpy(m_Head, vector.m_Head, vector.size() * sizeof(T));
		}

		Vector& operator=(const std::initializer_list<T>& list) {
			m_Size = list.size();
			if (m_Capacity < m_Size) {
				m_Capacity = std::ceilf(m_Size + m_Size * ME_VECTOR_SCALE);
				upstreammemory::stref->deallocate(m_Head);
				upstreammemory::stref->message("VECTOR: Insufficient capacity");
				m_Head = (T*)upstreammemory::stref->allocate(sizeof(T) * m_Capacity);
				upstreammemory::stref->message("VECTOR: Increased capacity");
			}

			T* it = m_Head;
			for (T const& i : list)
				*it++ = i;
			
			return *this;
		}

		Vector& operator=(const Vector& right) {
			m_Size = right.size();
			if (m_Capacity < m_Size) {
				m_Capacity = std::ceilf(m_Size + m_Size * ME_VECTOR_SCALE);
				upstreammemory::stref->deallocate(m_Head);
				upstreammemory::stref->message("VECTOR: Insufficient capacity");
				m_Head = (T*)upstreammemory::stref->allocate(sizeof(T) * m_Capacity);
				upstreammemory::stref->message("VECTOR: Increased capacity");
			}

			memcpy(m_Head, right.m_Head, right.size() * sizeof(T));
			return *this;
		}

		~Vector() {
			auto it = begin();
			if (m_Capacity != 0) {
				while (it != end())
					it++->~T();
				upstreammemory::stref->deallocate(m_Head);
				upstreammemory::stref->message("VECTOR: Deinitializing");
			}
		}

		void push_back(const T& element) {
			if (m_Capacity == m_Size) expand(std::ceilf(m_Size * ME_VECTOR_SCALE));

			*end() = element;
			m_Size++;
		}

		void push(T* pos, const T& element) {

			ME_MEM_ERROR(belongs(pos), "VECTOR: Position out of scope");

			size_t at = (pos - m_Head);
			if (m_Capacity == m_Size) expand(std::ceilf(m_Size * ME_VECTOR_SCALE));

			memmove(m_Head + at + 1, m_Head + at, (m_Size - at) * sizeof(T));
			*pos = element;
			m_Size++;
		}

		template<typename ...Args>
		void emplace_back(Args&& ...args) {
			if (m_Capacity == m_Size) expand(std::ceilf(m_Size * ME_VECTOR_SCALE));

			new (end()) T(args...);
			m_Size++;
		}

		template<typename ...Args>
		void emplace(T* pos, Args&& ...args) {

			ME_MEM_ERROR(belongs(pos), "VECTOR: Position out of scope");

			size_t at = (pos - m_Head);
			if (m_Capacity == m_Size) expand(std::ceilf(m_Size * ME_VECTOR_SCALE));

			memmove(m_Head + at + 1, m_Head + at, (m_Size - at) * sizeof(T));
			new (pos) T(args...);
			m_Size++;
		}

		void reserve(const size_t& size) { if (m_Capacity < size) expand(size); }

		void erase(T* pos) {

			ME_MEM_ERROR(belongs(pos), "VECTOR: Position out of scope");

			pos->~T();

			memmove(pos, pos + 1, (end() - pos) * sizeof(T));
			m_Size--;
		}

		T& at(size_t index)
		{
			ME_MEM_ERROR(index < m_Size, "VERTOR: Index out of range!!");
			return *(m_Head + index);
		}
		T& operator[](size_t index)
		{
			ME_MEM_ERROR(index < m_Size, "VECTOR: Index out of range!!");
			return *(m_Head + index);
		}

		void clear() {
			T* it = m_Head;
			while (it != end())
				it++->~T();
			m_Size = 0;

		}
		void release() {
			if (m_Capacity != 0) {
				clear();
				upstreammemory::stref->deallocate(m_Head);
				upstreammemory::stref->message("VECTOR: Releasing memory");
				m_Capacity = 0;
			}
		}

		size_t size() const noexcept { return m_Size; }
		size_t capacity() const noexcept { return m_Capacity; }
		T& back() noexcept { return *(m_Head + (m_Size - 1)); }
		T* begin() noexcept { return m_Head; }
		T* end() noexcept { return m_Head + m_Size; }
		const T* begin() const noexcept { return m_Head; }
		const T* end() const noexcept { return m_Head + m_Size; }

	private:
		void expand(const size_t& count)
		{
			upstreammemory::stref->reallocate((void*&)m_Head, count * sizeof(T));
			upstreammemory::stref->message("VECTOR: Expanding");
			m_Capacity += count;
		}

		bool belongs(T* ptr) { return (ptr - begin() >= 0) && (end() - ptr >= 0); }
		T* m_Head;
		size_t m_Capacity, m_Size;
	};

	// Special vector which is capable of managing C-style string
	template<typename upstreammemory> class Vector<const char*, upstreammemory>
	{
	public:
		Vector()
			:m_Capacity(ME_DEFAULT_VECTOR_SIZE), m_Size(0) 
		{
			m_Head = (char**)upstreammemory::stref->allocate(sizeof(const char*) * (m_Capacity));
			upstreammemory::stref->message("VECTOR: Initialization");
		}

		Vector(const std::initializer_list<const char*>& list)
			:m_Size(list.size())
		{
			m_Capacity = std::ceilf(m_Size + m_Size * ME_VECTOR_SCALE);
			m_Head = (char**)upstreammemory::stref->allocate(sizeof(const char*) * (m_Capacity));
			upstreammemory::stref->message("VECTOR: Initialization");

			char** it = m_Head;
			for (const char* str : list) {
				size_t len = strlen(str);
				*it = (char*)upstreammemory::stref->allocate(sizeof(char) * len + 1);
				upstreammemory::stref->message("VECTOR: Allocating Cstring");
				memcpy(*it, str, len + 1);
				it++;
			}
		}
		Vector(const Vector& vector)
			:m_Size(vector.size())
		{
			m_Capacity = std::ceilf(m_Size + m_Size * ME_VECTOR_SCALE);
			m_Head = (char**)upstreammemory::stref->allocate(sizeof(const char*) * (m_Capacity));
			upstreammemory::stref->message("VECTOR: Copying");

			char** it = m_Head;
			for (const char* str : vector) {
				size_t len = strlen(str);
				*it = (char*)upstreammemory::stref->allocate(sizeof(char) * len + 1);
				upstreammemory::stref->message("VECTOR: Allocating Cstring");
				memcpy(*it, str, len + 1);
				it++;
			}
		}

		Vector& operator=(const Vector& right) {
			char** it = m_Head;
			while (it != (m_Head + m_Size)) {
				upstreammemory::stref->deallocate(*it++);
				upstreammemory::stref->message("VECTOR: Deallocating old CString");
			}

			m_Size = right.size();
			if (m_Capacity < m_Size) {
				m_Capacity = std::ceilf(m_Size + m_Size * ME_VECTOR_SCALE);
				upstreammemory::stref->deallocate(m_Head);
				upstreammemory::stref->message("VECTOR: Insufficient capacity");
				m_Head = (char**)upstreammemory::stref->allocate(sizeof(const char*) * m_Capacity);
				upstreammemory::stref->message("VECTOR: Increased capacity");
			}

			it = m_Head;
			for (const char* str : right) {
				size_t len = strlen(str);
				*it = (char*)upstreammemory::stref->allocate(sizeof(char) * len + 1);
				upstreammemory::stref->message("VECTOR: Allocating Cstring");
				memcpy(*it, str, len + 1);
				it++;
			}
			return *this;
		}

		Vector& operator=(const std::initializer_list<const char*>& list) {

			char** it = m_Head;
			while (it != (m_Head + m_Size)) {
				upstreammemory::stref->deallocate(*it++);
				upstreammemory::stref->message("VECTOR: Deallocating old CString");

			}

			m_Size = list.size();
			if (m_Capacity < m_Size) {
				m_Capacity = m_Size;
				upstreammemory::stref->deallocate(m_Head);
				upstreammemory::stref->message("VECTOR: Insufficient capacity");
				m_Head = (char**)upstreammemory::stref->allocate(sizeof(const char*) * m_Capacity);
				upstreammemory::stref->message("VECTOR: Increased capacity");
			}

			it = m_Head;
			for (const char* str : list) {
				size_t len = strlen(str);
				*it = (char*)upstreammemory::stref->allocate(sizeof(char) * len + 1);
				upstreammemory::stref->message("VECTOR: Allocating Cstring");
				memcpy(*it, str, len + 1);
				it++;
			}
			return *this;
		}

		~Vector() {
			char** it = begin();
			if (m_Capacity != 0) {
				while (it != end()) {
					upstreammemory::stref->deallocate(*it++);
					upstreammemory::stref->message("VECTOR: Deallocating CString");
				}
				upstreammemory::stref->deallocate(m_Head);
				upstreammemory::stref->message("VECTOR: Deinitializing");
			}
		}

		void push_back(const char* element) {
			if (m_Capacity == m_Size) expand(std::ceilf(m_Size * ME_VECTOR_SCALE));

			size_t len = strlen(element);
			*end() = (char*)upstreammemory::stref->allocate(sizeof(char) * len + 1);
			upstreammemory::stref->message("VECTOR: Allocating Cstring");
			char* it = *end();
			memcpy(it, element, len);
			*(it + len) = '\0';

			m_Size++;
		}

		void push(char** pos, const char* element) {

			ME_MEM_ERROR(belongs(pos), "VECTOR: Position out of scope");

			size_t at = (pos - m_Head);
			if (m_Capacity == m_Size) expand(std::ceilf(m_Size * ME_VECTOR_SCALE));

			pos = m_Head + at;
			memmove(m_Head + at + 1, m_Head + at, (m_Size - at) * sizeof(const char*));

			size_t len = strlen(element);
			*pos = (char*)upstreammemory::stref->allocate(sizeof(char) * len + 1);
			upstreammemory::stref->message("VECTOR: Allocating Cstring");
			memcpy(*pos, element, len);
			*(*pos + len) = '\0';

			m_Size++;
		}

		void emplace_back(const char* element) {
			if (m_Capacity == m_Size) expand(std::ceilf(m_Size * ME_VECTOR_SCALE));

			size_t len = strlen(element);
			*end() = (char*)upstreammemory::stref->allocate(sizeof(char) * len + 1);
			upstreammemory::stref->message("VECTOR: Allocating Cstring");
			char* it = *end();
			memcpy(it, element, len);
			*(it + len) = '\0';

			m_Size++;
		}

		void emplace(char** pos, const char* element) {

			ME_MEM_ERROR(belongs(pos), "VECTOR: Position out of scope");

			size_t at = (pos - m_Head);
			if (m_Capacity == m_Size) expand(std::ceilf(m_Size * ME_VECTOR_SCALE));

			pos = m_Head + at;
			memmove(m_Head + at + 1, m_Head + at, (m_Size - at) * sizeof(const char*));

			size_t len = strlen(element);
			*pos = (char*)upstreammemory::stref->allocate(sizeof(char) * len + 1);
			upstreammemory::stref->message("VECTOR: Allocating Cstring");
			memcpy(*pos, element, len);
			*(*pos + len) = '\0';

			m_Size++;
		}

		void reserve(const size_t& size) { if (m_Capacity < size) expand(size); }

		void erase(char** pos) {

			ME_MEM_ERROR(belongs(pos), "VECTOR: Position out of scope");

			upstreammemory::stref->deallocate(*pos);
			upstreammemory::stref->message("VECTOR: Deallocating Cstring while erase");

			memmove(pos, pos + 1, (end() - pos) * sizeof(const char*));

			m_Size--;
		}

		const char* at(size_t index) {
			ME_MEM_ERROR(index < m_Size, "VERTOR: Index out of range!!");
			return *(m_Head + index);
		}

		const char* operator[](size_t index) {
			ME_MEM_ERROR(index < m_Size, "VECTOR: Index out of range!!");
			return *(m_Head + index);
		}

		void clear() {
			char** it = begin();
			if (it != end())
				while (it != end()) {
					upstreammemory::stref->deallocate(*it++);
					upstreammemory::stref->message("VECTOR: Deallocating CString");
				}
			m_Size = 0;
		}

		void release() {
			if (m_Capacity != 0) {
				clear();
				upstreammemory::stref->deallocate(m_Head);
				upstreammemory::stref->message("VECTOR: Deinitializing");
				m_Capacity = 0;
			}
		}

		size_t size() const noexcept { return m_Size; }
		size_t capacity() const noexcept { return m_Capacity; }
		const char* back() const noexcept { return *(m_Head + (m_Size - 1)); }
		char** begin() noexcept { return m_Head; }
		char** end() noexcept { return m_Head + m_Size; }
		char** begin() const noexcept { return m_Head; }
		char** end() const noexcept { return m_Head + m_Size; }

	private:
		void expand(const size_t& count)
		{
			upstreammemory::stref->reallocate((void*&)m_Head, count * sizeof(char*));
			upstreammemory::stref->message("VECTOR: Expanding");
			m_Capacity += count;
		}

		bool belongs(char** ptr) { return (ptr - begin() >= 0) && (end() - ptr >= 0); }
		char** m_Head;
		size_t m_Capacity, m_Size;
	};
	template<typename T> 
	using vector = Vector<T, ME::MEUpstreamMemory>;
}