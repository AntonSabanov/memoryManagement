#include "pch.h"
#include <iostream>
#include "../MemoryManagement/MemoryAllocation.cpp"

int main()
{
	MemoryAllocator allocator;
	allocator.Init();
	allocator.DumpStat();
    std::cout << "Hello World!\n"; 
}

