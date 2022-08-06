#ifndef ME_BUDDYGPUALLOCATOR
#define ME_BUDDYGPUALLOCATOR

#pragma once

#include "GPUMemoryManager.h"
#include "Core/Utilites/Vector.h"

#define MAX_VERTEX_BUFFER 1024
#define MAX_INDEX_BUFFER 1000

namespace ME
{
	/*
	* v_begin tracks vertex buffer
	* i_begin tracks triplet chuck of index buffer, this is to maintain triangular inegrety
	*/
	struct MemoryChunk
	{
		unsigned int size, filled;
		MemoryChunk* next;
	};
	class BuddyAllocator : public GPUMemoryManager
	{
	public:
		BuddyAllocator();
		virtual ~BuddyAllocator();
		virtual GPUResponse Allocate(const size_t& v_size, const size_t& i_size) override;
		virtual void Deallocate(const GPUResponse& resouce, const size_t& v_size, const size_t& i_size) override;

		virtual size_t QueryUsedMemory();
		virtual size_t QueryTotalMemory();
		virtual double QueryFragmentation();
	private:

		void detachnextnode(MemoryChunk* prev);
		void heal(MemoryChunk* before); // Combines continious like types.

		Vector<unsigned int> vertex_id, index_id;
		Vector<MemoryChunk*> v_head, i_head;

		size_t m_Size;
		unsigned short v_bound, i_bound;
		friend class OpenGLResource;
	};
}

#endif
