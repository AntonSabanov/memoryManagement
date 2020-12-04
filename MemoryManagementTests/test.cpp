#include "pch.h"
#include "../MemoryManagement/MemoryAllocation.cpp"

TEST(TestCaseName, TestName) 
{
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
}

TEST(Test, Test) 
{
	MemoryAllocator allocator;
	allocator.Init();
	int *p1 = (int*)allocator.Alloc(sizeof(int));
	*p1 = 5;
	int *p2 = (int*)allocator.Alloc(sizeof(int));
	*p2 = 7;
	int *p3 = (int*)allocator.Alloc(sizeof(int));
	*p3 = 3;
	EXPECT_EQ(*p1, 5);
	EXPECT_EQ(*p2, 7);
	EXPECT_EQ(*p3, 3);
	allocator.Free(p2);
	int *p4 = (int*)allocator.Alloc(sizeof(int));
	*p4 = 8;
	int *p5 = (int*)allocator.Alloc(sizeof(int));
	*p5 = 9;
	//EXPECT_EQ(*p2, 7);
	EXPECT_TRUE(true);
}