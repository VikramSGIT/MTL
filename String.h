#pragma once

#ifndef ME_STRING
#define ME_STRING
#endif


#include "Memory/MemoryManager.h"

namespace ME
{
	template <typename upstreammemory = MEUpstreamMemory> class String
	{
	public:	

		String()
			:m_Size(0), m_String(nullptr), m_Capacity(0) {}

		String(const char* string)
			:m_Size(strlen(string))
		{
			m_Capacity = m_Size;
			if (string == nullptr) return;

			m_String = (char*)upstreammemory::stref->allocate(m_Capacity + 1);
			upstreammemory::stref->message("STRING: Allocating String");
			memcpy(m_String, string, sizeof(char) * (m_Size + 1));
			*(m_String + m_Size) = '\0';
		}
		String(const char& string)
			:m_Size(1)
		{
			m_Capacity = m_Size;
			m_String = (char*)upstreammemory::stref->allocate(m_Capacity + 1);
			upstreammemory::stref->message("STRING: Allocating String");
			*m_String = string;
			*(m_String + m_Size) = '\0';
		}
		String(const String& string)
			: m_Size(string.m_Size)
		{
			if (string.m_String == nullptr) return;

			m_Capacity = m_Size;
			m_String = (char*)upstreammemory::stref->allocate(m_Capacity + 1);
			upstreammemory::stref->message("STRING: Allocating String");
			memcpy(m_String, string.m_String, m_Size);
			*(m_String + m_Size) = '\0';
		}


		~String() {
			if(m_Capacity != 0){ 
				upstreammemory::stref->deallocate(m_String);
				upstreammemory::stref->message("STRING: Deallocating String");
			}
		}

		inline const char* c_str() const { return m_String; }

		String operator+(const char& right)
		{
			String out(1 + m_Size);

			memcpy(out.m_String, m_String, m_Size);
			*(out.m_String + m_Size) = right;
			*(out.m_String + m_Size + 1) = '\0';
			return out;
		}

		String operator+(const char* right)
		{
			size_t r_size = strlen(right);
			String out(r_size + m_Size);

			memcpy(out.m_String, m_String, m_Size);
			memcpy(out.m_String + m_Size, right, r_size);
			*(out.m_String + m_Size + r_size) = '\0';
			return out;
		}

		String operator+(const String& right)
		{
			String out(m_Size + right.m_Size);

			memcpy(out.m_String, m_String, m_Size);
			memcpy(out.m_String + m_Size, right.m_String, right.m_Size);
			*(out.m_String + m_Size + right.m_Size) = '\0';
			return out;
		}

		String operator=(const String& right) {
			m_Size = right.m_Size;
			if (m_Capacity < m_Size) {
				upstreammemory::stref->deallocate(m_String);
				upstreammemory::stref->message("STRING: Deallocating old String");
				m_String = (char*)upstreammemory::stref->allocate(m_Size + 1);
				upstreammemory::stref->message("STRING: Allocating String");
				m_Capacity = m_Size;
			}
			else {
				upstreammemory::stref->message("STRING: Skipping reallocation, string has enough capacity");
			}
			memcpy(m_String, right.m_String, m_Size);
			*(m_String + m_Size) = '\0';
			return *this;
		}

		String operator=(const char* right) {
			m_Size = strlen(right);
			if (m_Size > m_Capacity) {
				upstreammemory::stref->deallocate(m_String);
				upstreammemory::stref->message("STRING: Deallocating old String");
				m_String = (char*)upstreammemory::stref->allocate(m_Size + 1);
				upstreammemory::stref->message("STRING: Allocating String");
				m_Capacity = m_Size;
			}
			else {
				upstreammemory::stref->message("STRING: Skipping reallocation, string has enough capacity");
			}
			memcpy(m_String, right, m_Size);
			*(m_String + m_Size) = '\0';
			return *this;
		}

		String operator=(const char right) {
			m_Size = 1;
			if (m_Capacity < 1) {
				upstreammemory::stref->deallocate(m_String);
				upstreammemory::stref->message("STRING: Deallocating old String");
				m_String = (char*)upstreammemory::stref->allocate(m_Size + 1);
				upstreammemory::stref->message("STRING: Allocating String");
				m_Capacity = m_Size;
			}
			else {
				upstreammemory::stref->message("STRING: Skipping reallocation, string has enough capacity");
			}
			m_String[0] = right;
			*(m_String + m_Size) = '\0';
			return *this;
		}

		bool operator== (const String& right) const
		{
			size_t l = 0, r = 0;
			if (m_Size != right.m_Size)
				return false;

			while (l < m_Size)
				if (m_String[l++] != right.m_String[r++])
					return false;

			return true;
		}

		bool operator== (const char* right) const
		{
			size_t len = strlen(right);
			if (m_Size != len)
				return false;

			size_t l = 0, r = 0;
			while (l < m_Size)
				if (m_String[l++] != right[r++])
					return false;

			return true;
		}

		bool operator!=(const String& right) const
		{
			size_t l = 0, r = 0;
			if (m_Size != right.m_Size)
				return true;

			while (l < m_Size)
				if (m_String[l++] != right.m_String[r++])
					return true;

			return false;
		}

		bool operator!=(const char* right) const
		{
			size_t len = strlen(right);
			if (m_Size != len)
				return true;

			size_t l = 0, r = 0;
			while (l < m_Size)
				if (m_String[l++] != right[r++])
					return true;

			return false;
		}

		String operator+=(const char& right)
		{
			if (m_Capacity - m_Size < 1) {
				upstreammemory::stref->reallocate((void*&)m_String, 1);
				upstreammemory::stref->message("STRING: Expanding String");
				m_Capacity++;
			}
#ifdef ME_MEM_DEBUG
			else {
				upstreammemory::stref->message("STRING: Skipping reallocation, string has enough capacity");
			}
#endif

			*(m_String + m_Size) = right;
			*(m_String + m_Size + 1) = '\0';
			m_Size++;
			return *this;
		}

		String operator+=(const char* right)
		{
			size_t r_size = strlen(right);
			if (m_Capacity - m_Size < r_size) {
				upstreammemory::stref->reallocate((void*&)m_String, r_size);
				upstreammemory::stref->message("STRING: Expanding String");
				m_Capacity += r_size;
			}
#ifdef ME_MEM_DEBUG
			else {
				upstreammemory::stref->message("STRING: Skipping reallocation, string has enough capacity");
			}
#endif

			memcpy(m_String + m_Size, right, r_size);
			m_Size += r_size;
			*(m_String + m_Size) = '\0';
			return *this;
		}

		String operator+=(const String& right)
		{
			if (m_Capacity - m_Size < right.m_Size) {
				upstreammemory::stref->reallocate((void*&)m_String, right.m_Size);
				upstreammemory::stref->message("STRING: Expanding String");
				m_Capacity += right.m_Size;
			}
#ifdef ME_MEM_DEBUG
			else {
				upstreammemory::stref->message("STRING: Skipping reallocation, string has enough capacity");
			}
#endif

			memcpy(m_String + m_Size, right.m_String, right.m_Size);
			m_Size += right.m_Size;
			*(m_String + m_Size) = '\0';
			return *this;
		}

		void erase(char* pos) {

			ME_MEM_ERROR(belongs(pos), "STRING: Provided pointer is out of bound!");

			memmove(pos, pos + 1, end() - pos);
			m_Size--;
			*(m_String + m_Size) = '\0';
		}

		// Erases the underlaying string 
		//NOTE: Memory is not released, use release to remove the memory
		void clear() { 
			memset(m_String, 0, m_Size);
			m_String[0] = '\0';
			m_Size = 0;
		}

		// Removes the underlaying memory
		void release() {
			if (m_Capacity != 0) {
				upstreammemory::stref->deallocate(m_String);
				m_Capacity = 0;
				m_Size = 0;
			}
		}

		size_t size() const { return m_Size; }
		size_t capacity() const { return m_Capacity; }

		char* begin() { return m_String; }
		char* end() { return m_String + m_Size; }
		const char* begin() const { return m_String; }
		const char* end() const { return m_String + m_Size; }
	private:
		char* m_String;
		size_t m_Size, m_Capacity;
		
		bool belongs(const char* ptr) {
			if (ptr >= m_String && ptr < (m_String + m_Size)) return true;
			return false;
		}

		String (const size_t& size)
			:m_Size(size), m_String((char*)upstreammemory::stref->allocate(size + 1)) {
			upstreammemory::stref->message("STRING: Initiating String");
			*(m_String + m_Size) = '\0';
			m_Capacity = size;
		}
	};
	using string = String<MEUpstreamMemory>;
}