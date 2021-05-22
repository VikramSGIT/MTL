#ifndef STRING
#define STRING

#include "MemoryManager.h"

namespace ME
{
	template <typename upstreammemory = alloc_dealloc_UpstreamMemory>
	class String
	{
	public:
		String()
			:m_UpstreamMemory(new upstreammemory), string(nullptr), Size(0) {}

		String(const char* str)
		{
			Size = strlen(str);
			string = (char*)m_UpstreamMemory->allocate(Size, "STRING: Allocating string");
			memcpy(string, str, Size);
		}

		String(const String& str)
			:string(str.string), Size(str.Size), m_UpstreamMemory(str.m_UpstreamMemory) {}

		~String()
		{
			m_UpstreamMemory->deallocate(string, Size, "STRING: Deallocating String");
			delete m_UpstreamMemory;
		}

		const char* operator+(const char* str)
		{
			size_t size = strlen(str);
			char* temp = (char*)m_UpstreamMemory->allocate(Size + size, "STRING: Expanding string");
			memcpy(temp, str, Size);
			memcpy(temp + Size, string, size);
			m_UpstreamMemory->deallocate(string, Size, "STRING: Deallocating old string");
			string = temp;
			Size += size;
			return string;
		}

		String& operator+(String& left)
		{
			char* temp = (char*)m_UpstreamMemory->allocate(left.Size + Size, "STRING: Expanding string");
			memcpy(temp, string, Size);
			memcpy(temp + Size, left.string, left.Size);
			m_UpstreamMemory->deallocate(string, Size, "STRING: Deallocating old string");
			string = temp;
			Size += left.Size;
			return *this;
		}

		const char* c_char() { return string; }

	private:
		char* string;
		size_t Size;
		UpstreamMemory* m_UpstreamMemory;
	};
}
#endif