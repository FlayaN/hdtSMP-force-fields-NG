#include "memory.h"

namespace Skyrim
{
	void* alloc(std::size_t count, std::int32_t al)
	{
		if (auto heap = RE::MemoryManager::GetSingleton(); heap)
		{
			if (void* ptr = heap->Allocate(count, al, al > __STDCPP_DEFAULT_NEW_ALIGNMENT__))
			{
				return ptr;
			}
			throw std::bad_alloc();
		}
		return nullptr;
	}

	void free(void* ptr, std::size_t al)
	{
		if (auto heap = RE::MemoryManager::GetSingleton(); heap)
		{
			heap->Deallocate(ptr, al > __STDCPP_DEFAULT_NEW_ALIGNMENT__);
		}
	}
}
