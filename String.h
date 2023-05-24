#pragma once

#include "Memory Manager/MemoryManager.h"

namespace ME
{
	template <typename upstreammemory = alloc_dealloc_UpstreamMemory> class String
	{
	public:	

		String()
			:m_Size(0), m_String((char*)upstreammemory::stref->allocate(1, "STRING: Initiating String")) 
		{
			memcpy(m_String, "\0", sizeof(char));
		}

		String(const char* string)
			:m_Size(strlen(string)), m_String(nullptr)
		{
			m_String = (char*)upstreammemory::stref->allocate(m_Size + 1, "STRING: Allocating String");
			memcpy(m_String, string, sizeof(char) * (m_Size + 1));
			*(m_String + m_Size) = '\0';
		}
		String(const char& string)
			:m_Size(1), m_String(nullptr)
		{
			m_String = (char*)upstreammemory::stref->allocate(m_Size + 1, "STRING: Allocating String");
			*m_String = string;
			*(m_String + m_Size) = '\0';
		}
		String(const String& string)
			:m_String(nullptr), m_Size(string.m_Size)
		{
			m_String = (char*)upstreammemory::stref->allocate(m_Size + 1, "STRING: Allocating String");
			memcpy(m_String, string.m_String, m_Size);
			*(m_String + m_Size) = '\0';
		}


		~String()
		{
			upstreammemory::stref->deallocate(m_String, "STRING: Deallocating String | Deallocating size: " + std::to_string(m_Size + 1));
		}

		inline const char* c_str() const { return m_String; }

		String& operator+(const char& right)
		{
			String out(1 + m_Size);

			memcpy(out.m_String, m_String, m_Size);
			*(out.m_String + m_Size) = right;
			*(out.m_String + m_Size + 1) = '/0';
			return out;
		}

		String& operator+(const char* right)
		{
			size_t r_size = strlen(right);
			String out(r_size + m_Size);

			memcpy(out.m_String, m_String, m_Size);
			memcpy(out.m_String + m_Size, right, r_size);
			*(out.m_String + m_Size + r_size) = '\0';
			return out;
		}

		String& operator+(const String& right)
		{
			String out(m_Size + right.m_Size);

			memcpy(out.m_String, m_String, m_Size);
			memcpy(out.m_String + m_Size, right.m_String, right.m_Size);
			*(out.m_String + m_Size + right.m_Size) = '\0';
			return out;
		}

		String& operator=(const String& right)
		{
			upstreammemory::stref->deallocate(m_String, "STRING: Deallocating old String");
			
			m_Size = right.m_Size;
			m_String = (char*)upstreammemory::stref->allocate(m_Size + 1, "STRING: Allocating String");
			memcpy(m_String, right.m_String, m_Size);
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

		String& operator+=(const char& right)
		{
			upstreammemory::stref->reallocate((void*&)m_String, 1, "STRING: Expanding String");

			*(m_String + m_Size) = right;
			*(m_String + m_Size + 1) = '/0';
			m_Size++;
			return *this;
		}

		String& operator+=(const char* right)
		{
			size_t r_size = strlen(right);
			upstreammemory::stref->reallocate((void*&)m_String, r_size, "STRING: Expanding String");

			memcpy(m_String + m_Size, right, r_size);
			m_Size += r_size;
			*(m_String + m_Size) = '\0';
			return *this;
		}

		String& operator+=(const String& right)
		{
			upstreammemory::stref->reallocate((void*&)m_String, right.m_Size, "STRING: Expanding String");

			memcpy(m_String + m_Size, right.m_String, right.m_Size);
			m_Size += right.m_Size;
			*(m_String + m_Size) = '\0';
			return *this;
		}

		char* begin() { return m_String; }
		char* end() { return m_String + m_Size; }
		const char* begin() const { return m_String; }
		const char* end() const { return m_String + m_Size; }
	private:
		char* m_String;
		size_t m_Size = 0;

		String (const size_t& size)
			:m_Size(size), m_String((char*)upstreammemory::stref->allocate(size + 1, "STRING: Initiating String")) 
		{
			*(m_String + m_Size) = '\0';
		}
	};
	using string = String<alloc_dealloc_UpstreamMemory>;
}