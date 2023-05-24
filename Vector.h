#pragma once

#include "Core/Memory/MemoryManager.h"

#include <mutex>
#include <initializer_list>
#include <string>

#define ME_DEFAULT_VECTOR_SIZE 3

namespace ME
{
	template<typename T> struct Iterator
	{
		T* m_begin, m_end;
		T* begin() { return m_begin; }
		T* end() { return m_end; }
	};
	template<typename T, typename upstreammemory = alloc_dealloc_UpstreamMemory>
	class Vector
	{
	public:
		Vector()
			:m_Capacity(ME_DEFAULT_VECTOR_SIZE), m_Size(0)
		{
			m_Head = (T*)upstreammemory::stref->allocate(sizeof(T) * m_Capacity, std::string("VECTOR: Initialization for ") + typeid(T).name());
			m_Tail = m_Head;
		}
		Vector(const std::initializer_list<T>& list)
			:m_Capacity(ME_DEFAULT_VECTOR_SIZE), m_Size(list.size())
		{
			m_Head = (T*)upstreammemory::stref->allocate(sizeof(T) * (m_Size + m_Capacity), std::string("VECTOR: Initialization using list for ") + typeid(T).name());
			memcpy(m_Head, list.begin(), list.size() * sizeof(T));
			m_Tail = m_Head + m_Size;
		}
		Vector(const Vector& vector)
			:m_Capacity(ME_DEFAULT_VECTOR_SIZE), m_Size(vector.size())
		{
			m_Head = (T*)upstreammemory::stref->allocate(sizeof(T) * (m_Size + m_Capacity), std::string("VECTOR: Initialization using list for ") + typeid(T).name());
			memcpy(m_Head, vector.m_Head, vector.size() * sizeof(T));
			m_Tail = m_Head + m_Size;
		}

		Vector& operator=(const Vector& right)
		{
			upstreammemory::stref->deallocate(m_Head, std::string("VECTOR: Reinitialising for") + typeid(T).name());

			m_Size = right.size();
			m_Capacity = ME_DEFAULT_VECTOR_SIZE;
			m_Head = (T*)upstreammemory::stref->allocate(sizeof(T) * (m_Size + m_Capacity), std::string("VECTOR: Initialization using list for ") + typeid(T).name());
			memcpy(m_Head, right.m_Head, right.size() * sizeof(T));
			m_Tail = m_Head + m_Size;
			return *this;
		}

		~Vector()
		{
			auto it = begin();
			if (it != end()) {
				while (it != end())
				{
					it->~T();
					it++;
				}
				upstreammemory::stref->deallocate(m_Head, std::string("VECTOR: Deinitializing for ") + std::string(typeid(T).name()) + std::string(" | Deallocation Size: ") + std::to_string(m_Size * sizeof(T)));
			}
		}

		void push_back(const T& element)
		{
			if (m_Capacity == 0)
				expand(ME_DEFAULT_VECTOR_SIZE);

			new (m_Tail) T(element);
			m_Tail++;
			m_Size++;
			m_Capacity--;
		}
		void push(T* pos, const T& element)
		{
			ME_MEM_ERROR(belongs(pos), "VECTOR: Position out of scope");

			if (m_Capacity == 0)
				expand(ME_DEFAULT_VECTOR_SIZE);

			T* temp = alloc<T>(end() - pos);
			memcpy(temp, pos, (char*)end() - (char*)pos);

			new (pos) T(element);
			memcpy(pos + 1, temp, (char*)end() - (char*)pos);
			dealloc(temp);
			m_Tail++;
			m_Size++;
			m_Capacity--;
		}
		template<typename ...Args>
		void emplace_back(Args&& ...args)
		{
			if (m_Capacity == 0)
				expand(ME_DEFAULT_VECTOR_SIZE);

			new (m_Tail) T(args...);
			m_Tail++;
			m_Size++;
			m_Capacity--;
		}

		template<typename ...Args>
		void emplace(T* pos, Args&& ...args)
		{
			ME_MEM_ERROR(belongs(pos), "VECTOR: Position out of scope");

			if (m_Capacity == 0)
				expand(ME_DEFAULT_VECTOR_SIZE);

			T* temp = alloc<T>(end() - pos);
			memcpy(temp, pos, (char*)end() - (char*)pos);

			new (pos) T(args...);
			memcpy(pos + 1, temp, (char*)end() - (char*)pos);
			dealloc(temp);
			m_Tail++;
			m_Size++;
			m_Capacity--;
		}

		void reserve(const size_t& size)
		{
			if (m_Capacity < size)
				expand(size);
		}

		void erase(T* pos)
		{
			ME_MEM_ERROR(belongs(pos), "VECTOR: Position out of scope");

			pos++;
			T* temp = alloc<T>(end() - pos);
			memcpy(temp, pos, (char*)end() - (char*)pos);
			memcpy(pos - 1, temp, (char*)end() - (char*)pos);
			dealloc(temp);
			m_Tail--;
			m_Size--;
			m_Capacity++;
		}

		T& at(size_t index)
		{
			if (index >= m_Size)
				ME_MEM_ERROR(true, "VERTOR: Index out of range!!")

				return *(m_Head + index);
		}
		T& operator[](size_t index)
		{
			if (index >= m_Size)
				ME_MEM_ERROR(true, "VECTOR: Index out of range!!")

				return *(m_Head + index);
		}

		void clear()
		{
			auto it = m_Head;
			while (it != m_Tail)
			{
				it->~T();
				it++;
			}
			m_Tail = m_Head;
			m_Capacity += m_Size;
			m_Size = 0;

		}
		void release()
		{
			auto it = begin();
			while (it != end())
			{
				it->~T();
				it++;
			}

			m_Tail = m_Head;
			upstreammemory::stref->deallocate(m_Head, std::string("VECTOR: Deinitializing for ") + typeid(T).name());
			m_Size = 0;
			m_Capacity = 0;
		}

		size_t size() const noexcept { return m_Size; }
		T* begin() noexcept { return m_Head; }
		T* end() noexcept { return m_Tail; }
		const T* begin() const noexcept { return m_Head; }
		const T* end() const noexcept { return m_Tail; }

	private:
		void expand(const size_t& count)
		{
			T* ptr = (T*)upstreammemory::stref->reallocate((void*&)m_Head, count * sizeof(T), std::string("VECTOR: Expanding for") + typeid(T).name());
			m_Head = ptr;
			m_Tail = ptr + m_Size;
			m_Capacity += count;
		}

		bool belongs(T* ptr) { return (ptr - begin() >= 0) && (end() - ptr >= 0); }
		T* m_Head, * m_Tail;
		size_t m_Capacity, m_Size;
	};

	template<typename T> using vector = Vector<T, ME::alloc_dealloc_UpstreamMemory>;


	template<typename T, typename upstreammemory> class Vector<T*, upstreammemory>
	{
	public:
		Vector()
			:m_Capacity(ME_DEFAULT_VECTOR_SIZE), m_Size(0)
		{
			m_Head = (T*)upstreammemory::stref->allocate(sizeof(T) * m_Capacity, std::string("VECTOR: Initialization for ") + typeid(T*).name());
			m_Tail = m_Head;
		}
		Vector(const std::initializer_list<const char*>& list)
			:m_Capacity(ME_DEFAULT_VECTOR_SIZE), m_Size(list.size())
		{
			m_Head = (T**)upstreammemory::stref->allocate(sizeof(char*) * (m_Size + m_Capacity), std::string("VECTOR: Initialization using list for ") + typeid(char*).name());
			for (int i = 0; i < list.size(); i++) {
				T* str = *(list.begin() + i);
				size_t len = strlen(str);
				m_Head[i] = (char*)upstreammemory::stref->allocate(sizeof(char) * (len + 1), std::string("VECTOR: Allocating string for const char*"));
				memcpy(m_Head[i], str, sizeof(char) * len);
				*(m_Head[i] + len) = '\0';
			}
			m_Tail = m_Head + m_Size;
		}
		Vector(const Vector& vector)
			:m_Capacity(ME_DEFAULT_VECTOR_SIZE), m_Size(vector.size())
		{
			m_Head = (char**)upstreammemory::stref->allocate(sizeof(char*) * (m_Size + m_Capacity), std::string("VECTOR: Initialization using list for ") + typeid(char*).name());
			for (int i = 0; i < vector.size(); i++) {
				size_t len = strlen(vector[i]);
				m_Head[i] = (char*)upstreammemory::stref->allocate(sizeof(char) * (len + 1), std::string("VECTOR: Allocating string for const char*"));
				memcpy(m_Head[i], vector[i], sizeof(char) * len);
				*(m_Head[i] + len) = '\0';
			}
			m_Tail = m_Head + m_Size;
		}

		Vector& operator=(const Vector& right)
		{
			upstreammemory::stref->deallocate(m_Head, std::string("VECTOR: Reinitialising for") + typeid(char*).name());

			m_Size = right.size();
			m_Capacity = ME_DEFAULT_VECTOR_SIZE;
			m_Head = (char**)upstreammemory::stref->allocate(sizeof(char*) * (m_Size + m_Capacity), std::string("VECTOR: Initialization using list for ") + typeid(char*).name());
			for (int i = 0; i < right.size(); i++) {
				size_t len = strlen(right[i]);
				m_Head[i] = (char*)upstreammemory::stref->allocate(sizeof(char) * (len + 1), std::string("VECTOR: Allocating string for const char*"));
				memcpy(m_Head[i], right[i], sizeof(char) * len);
				*(m_Head[i] + len) = '\0';
			}
			m_Tail = m_Head + m_Size;
		}

		~Vector() {
			if (m_Head != m_Tail) {
				auto it = m_Head;
				while (it != m_Tail)
					upstreammemory::stref->deallocate(*it++, "VECTOR: Deallocating const char*");

				upstreammemory::stref->deallocate(m_Head, std::string("VECTOR: Deinitializing for const char*") + std::string(" | Deallocation Size: ") + std::to_string(m_Size * sizeof(char*)));
			}
		}

		void push_back(const char* element)
		{
			if (m_Capacity == 0)
				expand(ME_DEFAULT_VECTOR_SIZE);

			size_t len = strlen(element);
			*m_Tail = (char*)upstreammemory::stref->allocate(sizeof(char) * (len + 1), std::string("VECTOR: Allocating string for const char*"));
			memcpy(*m_Tail, element, sizeof(char) * len);
			*(*m_Tail + len) = '\0';

			m_Tail++;
			m_Size++;
			m_Capacity--;
		}

		// DONT USE IT under maintainace...
		void push(char** pos, const char*& element)
		{
			ME_MEM_ERROR(belongs(pos), "VECTOR: Position out of scope");

			if (m_Capacity == 0)
				expand(ME_DEFAULT_VECTOR_SIZE);

			char** temp = alloc<char*>(end() - pos);
			memcpy(*temp, pos, (char**)end() - (char**)pos);

			
			//memcpy(pos + 1, temp, (char*)end() - (char*)pos);
			dealloc(temp);
			m_Tail++;
			m_Size++;
			m_Capacity--;
		}

		void emplace_back(const char* element)
		{
			if (m_Capacity == 0)
				expand(ME_DEFAULT_VECTOR_SIZE);

			size_t len = strlen(element);
			*m_Tail = (char*)upstreammemory::stref->allocate(sizeof(char) * (len + 1), std::string("VECTOR: Allocating string for const char*"));
			memcpy(*m_Tail, element, sizeof(char) * len);
			*(*m_Tail + len) = '\0';

			m_Tail++;
			m_Size++;
			m_Capacity--;
		}

		// DONT USE IT ...
		void emplace(char** pos, const char* element)
		{
			ME_MEM_ERROR(belongs(pos), "VECTOR: Position out of scope");

			size_t len = strlen(element);

			if (m_Capacity == 0)
				expand(ME_DEFAULT_VECTOR_SIZE);

			char** temp = (char**)upstreammemory::stref->allocate((end() - pos) * sizeof(char*));

			memcpy(temp, pos, (char**)end() - (char**)pos);
			memcpy(pos + 1, temp, (char**)end() - (char**)pos);

			*pos = (char*)upstreammemory::stref->allocate(len, "VECTOR: Allocating string for const char*");
			memcpy(*pos, element, len * sizeof(char));

			dealloc(temp);

			m_Tail++;
			m_Size++;
			m_Capacity--;
		}

		void reserve(const size_t& size)
		{
			if (m_Capacity < size)
				expand(size);
		}

		void erase(char** pos)
		{
			ME_MEM_ERROR(belongs(pos), "VECTOR: Position out of scope");

			pos++;
			upstreammemory::stref->deallocate(pos, std::string("VECTOR: Deinitializing for const char*"));
			char** temp = alloc<char*>(end() - pos);
			memcpy(temp, pos, (char*)end() - (char*)pos);

			memcpy(pos - 1, temp, (char*)end() - (char*)pos);
			dealloc(temp);

			m_Tail--;
			m_Size--;
			m_Capacity++;
		}

		char* at(size_t index)
		{
			if (index >= m_Size)
				ME_MEM_ERROR(true, "VERTOR: Index out of range!!")

				return *(m_Head + index);
		}
		char* operator[](size_t index)
		{
			if (index >= m_Size)
				ME_MEM_ERROR(true, "VECTOR: Index out of range!!")

				return *(m_Head + index);
		}

		void clear()
		{
			auto it = m_Head;
			while (it != m_Tail)
			{
				upstreammemory::stref->deallocate(*it, "VECTOR: Deallocating const char*");
				it++;
			}
			m_Tail = m_Head;
			m_Capacity += m_Size;
			m_Size = 0;

		}
		void release()
		{
			clear();
			upstreammemory::stref->deallocate(m_Head, std::string("VECTOR: Releasing for ") + typeid(char*).name());
			m_Size = 0;
			m_Capacity = 0;
		}

		size_t size() const noexcept { return m_Size; }
		char** begin() noexcept { return m_Head; }
		char** end() noexcept { return m_Tail; }
		const char** begin() const noexcept { return m_Head; }
		const char** end() const noexcept { return m_Tail; }

	private:
		void expand(const size_t& count)
		{
			char** ptr = (char**)upstreammemory::stref->reallocate((void*&)m_Head, count * sizeof(char*), "VECTOR: Expanding for const char*");
			m_Tail = m_Head + m_Size;
			m_Capacity += count;
		}

		bool belongs(char** ptr) { return (ptr - begin() >= 0) && (end() - ptr >= 0); }
		char** m_Head, ** m_Tail;
		size_t m_Capacity, m_Size;
	};
}