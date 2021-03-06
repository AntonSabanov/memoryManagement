#include "pch.h"
#include <iostream>
#include "../MemoryManagement/MemoryAllocation.cpp"

int main()
{
	MemoryAllocator allocator;
	allocator.Init();
	int *ptrFSA_16_1 = (int*)allocator.Alloc(3 * sizeof(int));
	int *ptrFSA_16_2 = (int*)allocator.Alloc(2 * sizeof(int));
	int *ptrFSA_16_3 = (int*)allocator.Alloc(sizeof(int));
	///
	int *ptrFSA_32_1 = (int*)allocator.Alloc(5 * sizeof(int));
	int *ptrFSA_32_2 = (int*)allocator.Alloc(6 * sizeof(int));
	///
	int *ptrFSA_64_1 = (int*)allocator.Alloc(9 * sizeof(int));
	///
	int *ptrFSA_128_1 = (int*)allocator.Alloc(25 * sizeof(int));
	///
	int *ptrFSA_256_1 = (int*)allocator.Alloc(50 * sizeof(int));
	///
	for (int i = 0; i < 7; i++)
	{
		int *ptrFSA_512 = (int*)allocator.Alloc(100 * sizeof(int));
	}
	///
	int *ptrCA_1 = (int*)allocator.Alloc(200 * sizeof(int));
	int *ptrCA_2 = (int*)allocator.Alloc(1024 * 1024 * 8);
	int *ptrCA_3 = (int*)allocator.Alloc(1024 * 1024 * 8);
	///
	int *ptrOS_1 = (int*)allocator.Alloc(1024 * 1024 * 11);
	///
	allocator.DumpStat();
	allocator.DumpBlocks();
    //std::cout << "Hello World!\n"; 
}

