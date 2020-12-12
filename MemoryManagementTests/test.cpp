#include "pch.h"
#include "../MemoryManagement/MemoryAllocation.cpp"

class TestClass
{
public:
	std::string name = "test";
	int id = 0;

	TestClass(std::string strName, int idNumber)
	{
		name = strName;
		id = idNumber;
	}
};

TEST(DefaultTest, Test) 
{
	MemoryAllocator allocator;
	allocator.Init();
	int *pi = (int*)allocator.Alloc(sizeof(int));
	double *pd = (double*)allocator.Alloc(sizeof(double));
	int *pa = (int*)allocator.Alloc(10 * sizeof(int));
	allocator.DumpStat();
	allocator.DumpBlocks();
	allocator.Free(pa);
	allocator.Free(pd);
	allocator.Free(pi);
	allocator.Destroy();
	EXPECT_EQ(1, 1);
	EXPECT_TRUE(true);
}

TEST(FSA_Test, AllocTest_16) // проверка, что вызываютс€ нужные аллокаторы
{
	MemoryAllocator allocator;
	allocator.Init();
	int *ptr_1 = (int*)allocator.Alloc(3 * sizeof(int));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA16);
	int *ptr_2 = (int*)allocator.Alloc(10);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA16);
	double *ptr_3 = (double*)allocator.Alloc(sizeof(double));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA16);
	allocator.Destroy();
}

TEST(FSA_Test, AllocTest_32) // проверка, что вызываютс€ нужные аллокаторы
{
	MemoryAllocator allocator;
	allocator.Init();
	double *ptr_1 = (double*)allocator.Alloc(2 * sizeof(double));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA32);
	char *ptr_2 = (char*)allocator.Alloc(25);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA32);
	int *ptr_3 = (int*)allocator.Alloc(5 * sizeof(int));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA32);
	allocator.Destroy();
}

TEST(FSA_Test, AllocTest_64) // проверка, что вызываютс€ нужные аллокаторы
{
	MemoryAllocator allocator;
	allocator.Init();
	int *ptr_1 = (int*)allocator.Alloc(9 * sizeof(int));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA64);
	double *ptr_2 = (double*)allocator.Alloc(7 * sizeof(double));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA64);
	int *ptr_3 = (int*)allocator.Alloc(60);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA64);
}

TEST(FSA_Test, AllocTest_128) // проверка, что вызываютс€ нужные аллокаторы
{
	MemoryAllocator allocator;
	allocator.Init();
	int *ptr_1 = (int*)allocator.Alloc(25 * sizeof(int));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA128);
	float *ptr_2 = (float*)allocator.Alloc(25 * sizeof(float));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA128);
	char *ptr_3 = (char*)allocator.Alloc(124 * sizeof(char));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA128);
}

TEST(FSA_Test, AllocTest_256) // проверка, что вызываютс€ нужные аллокаторы
{
	MemoryAllocator allocator;
	allocator.Init();
	int *ptr_1 = (int*)allocator.Alloc(50 * sizeof(int));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA256);
	double *ptr_2 = (double*)allocator.Alloc(25 * sizeof(double));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA256);
	int *ptr_3 = (int*)allocator.Alloc(252);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA256);
}

TEST(FSA_Test, AllocTest_512) // проверка, что вызываютс€ нужные аллокаторы
{
	MemoryAllocator allocator;
	allocator.Init();
	int *ptr_1 = (int*)allocator.Alloc(100 * sizeof(int));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA512);
	int *ptr_2 = (int*)allocator.Alloc(500);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA512);
	double *ptr_3 = (double*)allocator.Alloc(63 * sizeof(double));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA512);
}

TEST(CA_Test, AllocTest) // проверка, что вызываютс€ нужные аллокаторы
{
	MemoryAllocator allocator;
	allocator.Init();
	int *ptr_1 = (int*)allocator.Alloc(200 * sizeof(int));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), CA);
	int *ptr_2 = (int*)allocator.Alloc(1024 * 1024 * 8);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), CA);
}

TEST(OS_Test, AllocTest) // проверка, что вызываютс€ нужные аллокаторы
{
	MemoryAllocator allocator;
	allocator.Init();
	int *ptr_1 = (int*)allocator.Alloc(1024 * 1024 * 11);
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), OS);
}

TEST(AllocAllTypeTest, Test) // проверка, что вызываютс€ нужные аллокаторы
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

TEST(FreeAllTypeTest, Test) // проверка, что вызываютс€ методы free дл€ нужных аллокаторов
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

TEST(FSA_Test, FreeList_Test) 
{
	MemoryAllocator allocator;
	allocator.Init();
	int *ptr_1 = (int*)allocator.Alloc(sizeof(int));
	*ptr_1 = 5;
	int *ptr_2 = (int*)allocator.Alloc(sizeof(int));
	*ptr_2 = 7;
	int *ptr_3 = (int*)allocator.Alloc(sizeof(int));
	*ptr_3 = 3;
	EXPECT_EQ(*ptr_1, 5);
	EXPECT_EQ(*ptr_2, 7);
	EXPECT_EQ(*ptr_3, 3);
	/////////////////////
	allocator.Free(ptr_2);
	int *ptr_4 = (int*)allocator.Alloc(sizeof(int));
	*ptr_4 = 8;
	EXPECT_EQ(*ptr_2, 8);
	/////////////////////
	allocator.Free(ptr_3);
	allocator.Free(ptr_1);
	int *ptr_5 = (int*)allocator.Alloc(sizeof(int));
	*ptr_5 = 9;
	int *ptr_6 = (int*)allocator.Alloc(sizeof(int));
	*ptr_6 = 10;
	EXPECT_EQ(*ptr_1, 9);
	EXPECT_EQ(*ptr_3, 10);
}

TEST(FSA_Test, AddNewPage_Test_1)
{
	MemoryAllocator allocator;
	allocator.Init();
	for (int i = 0; i < 254; i++) // заполнение страницы полностью
	{
		int *ptr = (int*)allocator.Alloc(sizeof(int));
		*ptr = i;
	}
	int *ptr = (int*)allocator.Alloc(sizeof(int));
	*ptr = 3;
	EXPECT_EQ(*ptr, 3);

	allocator.Free(ptr);
	int *ptr_1 = (int*)allocator.Alloc(sizeof(int));
	*ptr_1 = 7;
	EXPECT_EQ(*ptr, 7);
}

TEST(FSA_Test, AddNewPage_Test_2)
{
	MemoryAllocator allocator;
	allocator.Init();
	for (int i = 0; i < 7; i++) // заполнение страницы полностью
	{
		int *ptrFSA_512 = (int*)allocator.Alloc(100 * sizeof(int));
		*ptrFSA_512 = i * 111;
		EXPECT_EQ(*ptrFSA_512, i * 111);
	}
}

TEST(CA_Test, FreeTest_1)
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

TEST(CA_Test, FreeTest_2)
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

TEST(CA_Test, FreeTest_3)
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

TEST(CA_Test, AllocTest_Class)
{
	MemoryAllocator allocator;
	allocator.Init();
	TestClass *p = (TestClass*)allocator.Alloc(20 * sizeof(TestClass));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), CA);
	new(p)TestClass("class_test", 1);
	EXPECT_EQ(p->id, 1);
	EXPECT_EQ(p->name, "class_test");
	allocator.Free(p);
	////////////////////////////////
	*p = TestClass("class_test", 2);
	EXPECT_EQ(p->id, 2);
	EXPECT_EQ(p->name, "class_test");
	allocator.Free(p);
	allocator.Destroy();
}

TEST(FSA_Test, AllocTest_Class)
{
	MemoryAllocator allocator;
	allocator.Init();
	TestClass *p = (TestClass*)allocator.Alloc(sizeof(TestClass));
	EXPECT_EQ(allocator.GetCurrentAllocatorType(), FSA64);
	new(p)TestClass("class_test", 1);
	EXPECT_EQ(p->id, 1);
	EXPECT_EQ(p->name, "class_test");
	allocator.Free(p);
	////////////////////////////////
	*p = TestClass("class_test", 2);
	EXPECT_EQ(p->id, 2);
	EXPECT_EQ(p->name, "class_test");
	allocator.Free(p);
	allocator.Destroy();
}