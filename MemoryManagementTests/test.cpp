#include "pch.h"
#include "../MemoryManagement/MemoryAllocation.cpp"

TEST(TestCaseName, TestName) 
{
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
}

TEST(AllocTypeTest, Test) // проверка, что вызываютс€ нужные аллокаторы
{
	MemoryAllocator allocator;
	allocator.Init();
	int *ptr_1 = (int*)allocator.Alloc(3 * sizeof(int));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA16);
	int *ptr_2 = (int*)allocator.Alloc(5 * sizeof(int));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA32);
	int *ptr_3 = (int*)allocator.Alloc(9 * sizeof(int));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA64);
	int *ptr_4 = (int*)allocator.Alloc(25 * sizeof(int));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA128);
	int *ptr_5 = (int*)allocator.Alloc(50 * sizeof(int));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA256);
	int *ptr_6 = (int*)allocator.Alloc(100 * sizeof(int));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA512);
	int *ptr_7 = (int*)allocator.Alloc(200 * sizeof(int));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), CA);
	int *ptr_8 = (int*)allocator.Alloc(1024 * 1024 * 8);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), CA);
	int *ptr_9 = (int*)allocator.Alloc(1024 * 1024 * 11);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), OS);
}

TEST(FreeTypeTest, Test) // проверка, что вызываютс€ методы free дл€ нужных аллокаторов
{
	MemoryAllocator allocator;
	allocator.Init();
	int *ptr_1 = (int*)allocator.Alloc(3 * sizeof(int));
	int *ptr_2 = (int*)allocator.Alloc(5 * sizeof(int));
	int *ptr_3 = (int*)allocator.Alloc(9 * sizeof(int));
	int *ptr_4 = (int*)allocator.Alloc(25 * sizeof(int));
	int *ptr_5 = (int*)allocator.Alloc(50 * sizeof(int));
	int *ptr_6 = (int*)allocator.Alloc(100 * sizeof(int));
	int *ptr_7 = (int*)allocator.Alloc(200 * sizeof(int));
	int *ptr_8 = (int*)allocator.Alloc(1024 * 1024 * 8);
	int *ptr_9 = (int*)allocator.Alloc(1024 * 1024 * 11);
	allocator.Free(ptr_1);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA16);
	allocator.Free(ptr_2);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA32);
	allocator.Free(ptr_3);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA64);
	allocator.Free(ptr_4);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA128);
	allocator.Free(ptr_5);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA256);
	allocator.Free(ptr_6);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA512);
	allocator.Free(ptr_7);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), CA);
	allocator.Free(ptr_8);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), CA);
	allocator.Free(ptr_9);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), OS);
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

TEST(Test1, Test)
{
	MemoryAllocator allocator;
	allocator.Init();
	for (int i = 0; i < 254; i++)
	{
		int *p = (int*)allocator.Alloc(sizeof(int));
		*p = i;
	}
	int *p = (int*)allocator.Alloc(sizeof(int));
	*p = 3;
	EXPECT_EQ(*p, 3);
	allocator.Free(p);
	*p = 7;
	EXPECT_EQ(*p, 7);
}

TEST(Test2, Test)
{
	MemoryAllocator allocator;
	allocator.Init();
	for (int i = 0; i < 254; i++)
	{
		int *p = (int*)allocator.Alloc(sizeof(int));
		*p = i;
	}
	int *p = (int*)allocator.Alloc(32);
	*p = 3;
	EXPECT_EQ(*p, 3);
	allocator.Free(p);
	*p = 7;
	EXPECT_EQ(*p, 7);
}

TEST(CoalesceAllocatorTest, AllocTest)
{
	MemoryAllocator allocator;
	allocator.Init();	
	int *p = (int*)allocator.Alloc(1024 * 1024 * 5);
	*p = 1024 * 1024;
	EXPECT_EQ(*p, 1024 * 1024);
	//allocator.Free(p);
}

TEST(CoalesceAllocatorTest, FreeTest_1)
{
	MemoryAllocator allocator;
	allocator.Init();
	int *p1 = (int*)allocator.Alloc(1024 * 1024 * 5);
	*p1 = 1024 * 1024;
	EXPECT_EQ(*p1, 1024 * 1024);
	allocator.Free(p1);
	int *p2 = (int*)allocator.Alloc(1024 * 1024 * 5);
	*p2 = 1024;
	EXPECT_EQ(*p1, 1024);
}

TEST(CoalesceAllocatorTest, FreeTest_2)
{
	MemoryAllocator allocator;
	allocator.Init();
	//выдел€ютс€ 5 страниц с одним блоком на 5 мб
	int *ptr_1 = (int*)allocator.Alloc(1024 * 1024 * 5); //выдел€ю пам€ть по 5 мб
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
	///////////////////////
	int *testPtr_2 = (int*)allocator.Alloc(1024 * 1024 * 5);
	*testPtr_2 = 222;
	EXPECT_EQ(*ptr_3, 222);
	///////////////////////
	int *testPtr_3 = (int*)allocator.Alloc(1024 * 1024 * 5);
	*testPtr_3 = 333;
	EXPECT_EQ(*ptr_4, 333);
}

TEST(CoalesceAllocatorTest, FreeTest_3)
{
	MemoryAllocator allocator;
	allocator.Init();
	int *ptr_1 = (int*)allocator.Alloc(1024 * 1024 * 2);//выдел€ю блоки по 2 мб
	int *ptr_2 = (int*)allocator.Alloc(1024 * 1024 * 2);
	int *ptr_3 = (int*)allocator.Alloc(1024 * 1024 * 2);
	int *ptr_4 = (int*)allocator.Alloc(1024 * 1024 * 2);
	int *ptr_5 = (int*)allocator.Alloc(1024 * 1024 * 2);
	//выдел€етс€ 2 страницы, в 1 странице 4 зан€тых блока(ptr_1 - .._4), во 2 странице 1 зан€тый блок (ptr_5)
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
	allocator.Free(ptr_5);

	int *testPtr_1 = (int*)allocator.Alloc(1024 * 1024 * 2); // 2 мб
	*testPtr_1 = 1111;
	EXPECT_EQ(*ptr_3, 1111); // данные записываютс€ в птр3 так как дл€ птр1 требуетс€ запас дл€ разделени€ €чейки
	///////////////////////
	int *testPtr_2 = (int*)allocator.Alloc(1024 * 1024 * 1); // 1 мб
	*testPtr_2 = 2222;
	EXPECT_EQ(*ptr_1, 2222); // данные помещаютс€ в освобожденный блок птр1
	/////////////////////////
	int *testPtr_3 = (int*)allocator.Alloc(1024 * 1024 * 8); //8 мб
	*testPtr_3 = 3333;
	EXPECT_EQ(*ptr_5, 3333);
}