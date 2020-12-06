#include "pch.h"
#include "../MemoryManagement/MemoryAllocation.cpp"

//TEST(TestCaseName, TestName) 
//{
//  EXPECT_EQ(1, 1);
//  EXPECT_TRUE(true);
//}
//
//TEST(Test, Test) 
//{
//	MemoryAllocator allocator;
//	allocator.Init();
//	int *p1 = (int*)allocator.Alloc(sizeof(int));
//	*p1 = 5;
//	int *p2 = (int*)allocator.Alloc(sizeof(int));
//	*p2 = 7;
//	int *p3 = (int*)allocator.Alloc(sizeof(int));
//	*p3 = 3;
//	EXPECT_EQ(*p1, 5);
//	EXPECT_EQ(*p2, 7);
//	EXPECT_EQ(*p3, 3);
//	allocator.Free(p2);
//	int *p4 = (int*)allocator.Alloc(sizeof(int));
//	*p4 = 8;
//	int *p5 = (int*)allocator.Alloc(sizeof(int));
//	*p5 = 9;
//	//EXPECT_EQ(*p2, 7);
//	EXPECT_TRUE(true);
//}

//TEST(Test1, Test)
//{
//	MemoryAllocator allocator;
//	allocator.Init();
//	for (int i = 0; i < 254; i++)
//	{
//		int *p = (int*)allocator.Alloc(sizeof(int));
//		*p = i;
//	}
//	int *p = (int*)allocator.Alloc(sizeof(int));
//	*p = 3;
//	EXPECT_EQ(*p, 3);
//	allocator.Free(p);
//	*p = 7;
//	EXPECT_EQ(*p, 7);
//}

//TEST(Test2, Test)
//{
//	MemoryAllocator allocator;
//	allocator.Init();
//	for (int i = 0; i < 254; i++)
//	{
//		int *p = (int*)allocator.Alloc(sizeof(int));
//		*p = i;
//	}
//	int *p = (int*)allocator.Alloc(32);
//	*p = 3;
//	EXPECT_EQ(*p, 3);
//	allocator.Free(p);
//	*p = 7;
//	EXPECT_EQ(*p, 7);
//}

//TEST(CoalesceAllocatorTest, AllocTest)
//{
//	MemoryAllocator allocator;
//	allocator.Init();	
//	int *p = (int*)allocator.Alloc(1024 * 1024 * 5);
//	*p = 1024 * 1024;
//	EXPECT_EQ(*p, 1024 * 1024);
//	//allocator.Free(p);
//}

//TEST(CoalesceAllocatorTest, FreeTest_1)
//{
//	MemoryAllocator allocator;
//	allocator.Init();
//	int *p1 = (int*)allocator.Alloc(1024 * 1024 * 5);
//	*p1 = 1024 * 1024;
//	EXPECT_EQ(*p1, 1024 * 1024);
//	allocator.Free(p1);
//	int *p2 = (int*)allocator.Alloc(1024 * 1024 * 5);
//	*p2 = 1024;
//	EXPECT_EQ(*p1, 1024);
//}

TEST(CoalesceAllocatorTest, FreeTest_2)
{
	MemoryAllocator allocator;
	allocator.Init();
	int *ptr_1 = (int*)allocator.Alloc(1024 * 1024 * 5);
	int *ptr_2 = (int*)allocator.Alloc(1024 * 1024 * 5);
	int *ptr_3 = (int*)allocator.Alloc(1024 * 1024 * 5);
	int *ptr_4 = (int*)allocator.Alloc(1024 * 1024 * 5);
	int *ptr_5 = (int*)allocator.Alloc(1024 * 1024 * 5);
	*ptr_1 = 1000000;
	*ptr_2 = 2000000;
	*ptr_3 = 3000000;
	*ptr_4 = 4000000;
	*ptr_5 = 5000000;
	EXPECT_EQ(*ptr_1, 1000000);
	EXPECT_EQ(*ptr_2, 2000000);
	EXPECT_EQ(*ptr_3, 3000000);
	EXPECT_EQ(*ptr_4, 4000000);
	EXPECT_EQ(*ptr_5, 5000000);


	allocator.Free(ptr_3);

	allocator.Free(ptr_4);

	allocator.Free(ptr_1);

	int *testPtr_1 = (int*)allocator.Alloc(1024 * 1024 * 5);
	*testPtr_1 = 111;
	EXPECT_EQ(*ptr_1, 111);
	EXPECT_EQ(*ptr_4, 111);///?
	EXPECT_EQ(*ptr_3, 111);///?

	//int *testPtr_2 = (int*)allocator.Alloc(1024 * 1024 * 5);
	//*testPtr_2 = 222;
	//EXPECT_EQ(*ptr_4, 222);
}