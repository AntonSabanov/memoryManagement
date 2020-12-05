#include "pch.h"
//#include <iostream>
#include "cassert"
#include "windows.h"

#define PAGESIZE 4096

class FixedSizeAllocator //Page
{
public:
	struct BlockHeader
	{
		//BlockHeader* nextFreeBlock = nullptr;		// следующий свободный блок
		int nextFreeBlockIndex;	//индекс следующего свободного блока
	};

	struct PageHeader
	{
		BlockHeader* headFL = nullptr;	// Ссылка на первый свободный блок
		PageHeader* nextPage = nullptr;	//Ссылка на следующую страницу
	};

	size_t blockSize;
	PageHeader* mainPage = nullptr;
	bool isInit = false;
	int blocksCount = 0;


	FixedSizeAllocator(size_t blockSize)
	{	
		this->blockSize = blockSize;
	}

	~FixedSizeAllocator()
	{
		if (isInit == true) //если аллокатор не был деинициализирован вручную
		{
			DestoyFSA();
		}
		//если дестрой не был вызван, то выполнить очитску
//		PageHeader* tempPage = mainPage;
//		bool isReleased = false;
//		while (tempPage->nextPage != nullptr)
//		{
//			PageHeader* tempNextPage = tempPage->nextPage;
//			isReleased = VirtualFree(
//				(LPVOID)tempPage,
//				0,
//				MEM_RELEASE);
//
//#ifdef _DEBUG
//			assert(isReleased);  // проверка на освобождение
//#endif // DEBUG		
//
//			tempPage = tempNextPage;
//		}
//
//		if (tempPage != nullptr)
//		{
//			isReleased = VirtualFree(
//				(LPVOID)tempPage,
//				0,
//				MEM_RELEASE);
//
//#ifdef _DEBUG
//			assert(isReleased);  // проверка на освобождение
//#endif // DEBUG	
//		}
	}

	void InitFSA()
	{
		mainPage = AddNewPage();
		//auto newPage = AddNewPage();
		//if (mainPage == nullptr)
		//{
		//	mainPage = newPage;
		//	mainPage->nextPage = nullptr;
		//}			
		//else
		//{
		//	mainPage->nextPage = newPage;
		//}

		mainPage->headFL = (BlockHeader*)((char*)mainPage + blockSize);	// смещение указателя на первый свободный блок без учета индекса след. эл.	

		//mainPage->headFL->currentBlockIndex = 0; //индекс текущего блока
		//mainPage->headFL->nextFreeBlockIndex++; //индекс следующего свободного блока

		BlockHeadersInit(mainPage);	// инициализация заголовков блоков 

		//auto temp = mainPage->headFL;
		//int i = 1;
		//while ((char*)temp < (char*)mainPage + PAGESIZE - blockSize) 
		//{
		//	temp->nextFreeBlockIndex = i;
		//	i++;
		//	temp = (BlockHeader*)((char*)temp + blockSize);
		//}
		//temp->nextFreeBlockIndex = -1; // последний блок не имеет следующего
	}

	void DestoyFSA() //вызов АллокФри для аллокатора
	{
		PageHeader* tempPage = mainPage;
		bool isReleased = false;
		while (tempPage->nextPage != nullptr)
		{
			PageHeader* tempNextPage = tempPage->nextPage;
			isReleased = VirtualFree(
				(LPVOID)tempPage,
				0,
				MEM_RELEASE);			

#ifdef _DEBUG
			assert(isReleased);  // проверка на освобождение
#endif // DEBUG		

			tempPage = tempNextPage;
		}

		if (tempPage != nullptr)
		{
			isReleased = VirtualFree(
				(LPVOID)tempPage,
				0,
				MEM_RELEASE);

#ifdef _DEBUG
			assert(isReleased);  // проверка на освобождение
#endif // DEBUG	

			isInit = false; //аллокатор деинициализирован
		}
	}

	void* GetFreeBlock()
	{	
		//void* currentFreeBlock;
		auto currentPage = mainPage; // запоминаем текущую страницу начиная с главной

		//while (currentPage->headFL->nextFreeBlockIndex < 0) //ищем страницу со свободныими блоками
		//{
		//	if (currentPage->nextPage != nullptr)
		//	{
		//		//currentFreeBlock = (char*)currentPage->headFL + sizeof(int); //
		//		currentPage = currentPage->nextPage; //переходим к следующей странице
		//	}
		//	else
		//	{
		//		auto newPage = AddNewPage();	//добавляем новую страницу
		//		newPage->headFL = (BlockHeader*)((char*)newPage + blockSize);	//добавляем голову списка в новую страницу
		//		BlockHeadersInit(newPage);	//инициализируем блоки в новой странице
		//		currentPage->nextPage = newPage;	//записываем новую страницу в next для текущей страницы		
		//		currentPage = currentPage->nextPage; //переходим к следующей странице
		//	}				
		//}			

		while (currentPage->headFL == nullptr) //ищем страницу со свободныими блоками
		{
			currentPage = currentPage->nextPage; // если в этой странице нет свободных блоков, переходим на следующую
		}

		auto currentFreeBlock = (char*)currentPage->headFL + sizeof(int);			//адрес текущего пустого блока с учетом индекса предыдущего (часть с данными в блоке)
		int currentNextIndex = currentPage->headFL->nextFreeBlockIndex;				//индекс следующего блока после текущего
		//currentPage->headFL = (BlockHeader*)((char*)currentPage + (currentNextIndex * blockSize) + blockSize); // переход на следующий свободный блок по его индексу (указатель без учета индекса след эл.)
		
		if (currentNextIndex > 0)
		{
			currentPage->headFL = (BlockHeader*)((char*)currentPage + (currentNextIndex * blockSize) + blockSize);// переход на следующий свободный блок по его индексу (указатель без учета индекса след эл.)		
		}		
		else
		{
			auto newPage = AddNewPage();											//добавляем новую страницу
			newPage->headFL = (BlockHeader*)((char*)newPage + blockSize);			//добавляем голову списка в новую страницу
			BlockHeadersInit(newPage);												//инициализируем блоки в новой странице
			currentPage->nextPage = newPage;										//записываем новую страницу в next для текущей страницы		
			currentPage->headFL = nullptr;
		}

		//if (currentNextIndex > 0)
		//{
		//	currentPage->headFL = (BlockHeader*)((char*)currentPage + (currentNextIndex * blockSize) + blockSize);// переход на следующий свободный блок по его индексу (указатель без учета индекса след эл.)
		//}		
		//else
		//{
		//	auto newPage = AddNewPage();											//добавляем новую страницу
		//	newPage->headFL = (BlockHeader*)((char*)newPage + blockSize);			//добавляем голову списка в новую страницу
		//	BlockHeadersInit(newPage);												//инициализируем блоки в новой странице
		//	currentPage->nextPage = newPage;										//записываем новую страницу в next для текущей страницы		
		//}

		return currentFreeBlock;
	}

	//void SetFreeBlock(PageHeader* curPage, void *p)	// в функцию передается текущая страница, в которой находится указатель и сам указатель
	void SetFreeBlock(void *p)
	{
		auto curPage = mainPage;
		while (curPage != nullptr)
		{
			if ((char*)p > (char*)curPage && (char*)p < (char*)curPage + PAGESIZE)
				//&& ((char*)p - (char*)curPage) % alloc16.blockSize == 0) // проверка на попадание в интервал и на выравнивание по размерам ячеек
			{
				// помещаем ячейку в голову списка
				auto currentBlockindex = ((char*)curPage->headFL - (char*)curPage) / blockSize - 1;	// расчет индекса текущего свободного элемента
				curPage->headFL = (BlockHeader*)((char*)p - sizeof(int));
				curPage->headFL->nextFreeBlockIndex = currentBlockindex;
				break;
			}
			else
			{
				curPage = curPage->nextPage;
			}
		}
		//auto currentBlockindex = ((char*)mainPage->headFL - (char*)mainPage) / blockSize - 1; // расчет индекса текущего свободного элемента
		//mainPage->headFL = (BlockHeader*)((char*)p - sizeof(int));
		//mainPage->headFL->nextFreeBlockIndex = currentBlockindex;
		
		//auto currentBlockindex = ((char*)curPage->headFL - (char*)curPage) / blockSize - 1;	// расчет индекса текущего свободного элемента
		//curPage->headFL = (BlockHeader*)((char*)p - sizeof(int));
		//curPage->headFL->nextFreeBlockIndex = currentBlockindex;
	}

	PageHeader* AddNewPage()	//выделение новой страницы
	{
		auto newPage = (PageHeader*)VirtualAlloc(
			NULL,
			PAGESIZE,
			MEM_COMMIT,	//инициализирует страницу
			PAGE_READWRITE);
		isInit = true; // страница выделена, аллокатор проинициализирован

#ifdef _DEBUG
		assert(newPage != NULL);	// проверка на инициализацию страницы
		assert(isInit);
#endif // DEBUG	

		return newPage;
	}
	
	void BlockHeadersInit(PageHeader* page)
	{
		auto temp = page->headFL;
		int i = 1;
		while ((char*)temp < (char*)page + PAGESIZE - blockSize)
		{
			temp->nextFreeBlockIndex = i;
			i++;
			temp = (BlockHeader*)((char*)temp + blockSize);
			blocksCount++; // подсчет количества блоков всего
		}
		temp->nextFreeBlockIndex = -1; // последний блок не имеет следующего
		blocksCount++;
	}
};

class CoalesceAllocator
{
public:
	CoalesceAllocator()
	{
		//
	}
};

class MemoryAllocator
{
public:
	FixedSizeAllocator alloc16;
	FixedSizeAllocator alloc32;
	//FixedSizeAllocator alloc64;
	//FixedSizeAllocator alloc128;
	//FixedSizeAllocator alloc256;
	//FixedSizeAllocator alloc512;
	//CoalesceAllocator coalAlloc;

	bool isInit = false;

	//LPVOID page = NULL;

	MemoryAllocator() : alloc16(16 * sizeof(char)),			//Создает неинициализированный аллокатор
						alloc32(32 * sizeof(char))
						//alloc64(64 * sizeof(char)), 
						//alloc128(128 * sizeof(char)), 
						//alloc256(256 * sizeof(char)), 
						//alloc512(512 * sizeof(char))*/		
	{
		isInit = false;
		//Создает неинициализированный аллокатор
	}


	virtual ~MemoryAllocator() 
	{
		//destroy
		//проверка на дестрой, если дестрой не был вызван вызвать АллокФри (возможно в самом аллокаторе, а тут не надо)
	}

	virtual void Init()			//Выполняет инициализацию аллокатора, запрашивая необходимые страницы памяти у ОС
	{
		isInit = true;
		// инициализация всех аллокаторов
		alloc16.InitFSA();
		alloc32.InitFSA();
		//alloc64.InitFSA();
		//alloc128.InitFSA();
		//alloc256.InitFSA();
		//alloc512.InitFSA();

		//VirtualAlloc(
		//	page,
		//	PAGESIZE,
		//	MEM_COMMIT,			//инициализирует страницу в ранее зарезервированную область
		//	PAGE_READWRITE);
	}

	virtual void Destroy()
	{
		//вызвать АллокФри для всех аллокаторов
		if (isInit == true) // если аллокаторы были проинициализированны 
		{
			alloc16.DestoyFSA();
			// destroy all
			isInit == false;
		}		
	}

	virtual void* Alloc(size_t size)
	{
		//вызов в зависимости от аллокатора
		//return alloc16.GetFreeBlock();
		if (size <= 16)
		{
			return alloc16.GetFreeBlock();
		}
		else///////////////////////////////////////////////
		{
			return alloc32.GetFreeBlock();
		}
		//
		//if (size > 16 && size <= 32)
		//{
		//	//
		//}
		//
		//if (size > 32 && size <= 64)
		//{
		//	//
		//}
		//
		//if (size > 64 && size <= 128)
		//{
		//	//
		//}
		//
		//if (size > 128 && size <= 256)
		//{
		//	//
		//}
		//
		//if (size > 256 && size <= 512)
		//{
		//	//
		//}
		//
		//if (size > 512 && size <= 128)
		//{
		//	//
		//}
	}

	virtual void Free(void *p)
	{
		//auto curPage = alloc16.mainPage;
		//while (curPage != nullptr)
		//{
		//	if ((char*)p > (char*)curPage && (char*)p < (char*)curPage + PAGESIZE)
		//		//&& ((char*)p - (char*)curPage) % alloc16.blockSize == 0) // проверка на попадание в интервал и на выравнивание по размерам ячеек
		//	{
		//		//alloc16.SetFreeBlock(curPage, p); // помещаем ячейку в голову списка
		//		alloc16.SetFreeBlock(p); // помещаем ячейку в голову списка
		//		break;
		//	}
		//	else
		//	{
		//		curPage = curPage->nextPage;
		//	}
		//}
		if ((char*)p > (char*)alloc16.mainPage && (char*)p < (char*)alloc16.mainPage + PAGESIZE)
		{
			alloc16.SetFreeBlock(p);
		}
		else if ((char*)p > (char*)alloc32.mainPage && (char*)p < (char*)alloc32.mainPage + PAGESIZE)
		{
			alloc32.SetFreeBlock(p);
		}
		else return;

		//вызов в зависимости от аллокатора
		//alloc16.SetFreeBlock(p);

		//auto currentBlockindex = ((char*)curPage->headFL - (char*)curPage) / blockSize - 1;
		//auto tmp = ((char*)p - sizeof(int));
		//
		//if (alloc16.mainPage)
		//if (*(tmp + 16 * sizeof(char)) == *tmp)
	}

	virtual void DumpStat() const
	{
		//
	}

	virtual void DumpBlocks() const
	{
		//
	}
};

