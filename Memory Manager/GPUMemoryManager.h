#ifndef ME_GPUMEMORYMANAGER
#define ME_GPUMEMORYMANAGER

#pragma once

#include "MarsHeader.h"
#include "Core/Utilites/Ref.h"

namespace ME
{
	struct GPUResponse
	{
		unsigned int indexoffset = 0, vertexoffset = 0, v_id = 0, i_id = 0;
	};
	class GPUMemoryManager
	{
	public:
		virtual ~GPUMemoryManager() = default;
		virtual GPUResponse Allocate(const size_t& v_size, const size_t& i_size) = 0;
		virtual void Deallocate(const GPUResponse& resouce, const size_t& v_size, const size_t& i_size) = 0;

		virtual size_t QueryUsedMemory() = 0;
		virtual size_t QueryTotalMemory() = 0;
		virtual double QueryFragmentation() = 0;
	};
}

#endif
