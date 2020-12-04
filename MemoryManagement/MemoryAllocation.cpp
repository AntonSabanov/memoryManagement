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
		int nextFreeBlockIndex;
		//int currentBlockIndex;
	};

	struct PageHeader
	{
		BlockHeader* headFL = nullptr;		// Ссылка на первый свободный блок
		PageHeader* nextPage = nullptr;	//Ссылка на следующую страницу
	};

	size_t blockSize;
	PageHeader* currentPage = nullptr;


	FixedSizeAllocator(size_t blockSize)
	{	
		this->blockSize = blockSize;
	}

	~FixedSizeAllocator()
	{
		PageHeader* tempPage = currentPage;
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
		isReleased = VirtualFree(
			(LPVOID)tempPage,
			0,
			MEM_RELEASE);
#ifdef _DEBUG
		assert(isReleased);  // проверка на освобождение
#endif // DEBUG	
	}

	void* GetFreeBlock()
	{
		// если есть head, тоесть есть свободный блок, то возвращаем ссылку на него
		// если его нет в текущей странице, то создаем новую страницу
		// если следующая страница уже создана, то переходим в нее и ищем там
		
		//auto 
	/*	while (currentPage->headFL->nextFreeBlockIndex < 0)
		{

		}*/
			

		auto currentFreeBlock = (char*)currentPage->headFL + sizeof(int);			//адрес текущего пустого блока с учетом индекса предыдущего (часть с данными в блоке)
		int currentNextIndex = currentPage->headFL->nextFreeBlockIndex;				//индекс следующего блока после текущего
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
		}

		//currentPage->headFL->nextFreeBlockIndex = ++currentNextIndex;
		//auto index2 = ((char*)currentPage->headFL - (char*)currentPage) / blockSize - 1; // расчет индекса текущего свободного элемента

		return currentFreeBlock;
	}

	void SetFreeBlock(PageHeader* curPage, void *p)	// в функцию передается текущая страница, в которой находится указатель и сам указатель
	{
		//auto currentBlockindex = ((char*)currentPage->headFL - (char*)currentPage) / blockSize - 1; // расчет индекса текущего свободного элемента
		//currentPage->headFL = (BlockHeader*)((char*)p - sizeof(int));
		//currentPage->headFL->nextFreeBlockIndex = currentBlockindex;
		auto currentBlockindex = ((char*)curPage->headFL - (char*)curPage) / blockSize - 1;	// расчет индекса текущего свободного элемента
		curPage->headFL = (BlockHeader*)((char*)p - sizeof(int));
		curPage->headFL->nextFreeBlockIndex = currentBlockindex;
	}

	PageHeader* AddNewPage()	//выделение новой страницы
	{
		auto newPage = (PageHeader*)VirtualAlloc(
			NULL,
			PAGESIZE,
			MEM_COMMIT,	//инициализирует страницу
			PAGE_READWRITE);
#ifdef _DEBUG
		assert(newPage != NULL);	// проверка на инициализацию страницы
#endif // DEBUG	


		return newPage;
	}

	void InitFSA()
	{		
		currentPage = AddNewPage();
		//auto newPage = AddNewPage();
		//if (currentPage == nullptr)
		//{
		//	currentPage = newPage;
		//	currentPage->nextPage = nullptr;
		//}			
		//else
		//{
		//	currentPage->nextPage = newPage;
		//}
			
		currentPage->headFL = (BlockHeader*)((char*)currentPage + blockSize);	// смещение указателя на первый свободный блок без учета индекса след. эл.	
		
		//currentPage->headFL->currentBlockIndex = 0; //индекс текущего блока
		//currentPage->headFL->nextFreeBlockIndex++; //индекс следующего свободного блока
			
		BlockHeadersInit(currentPage);	// инициализация заголовков блоков 

		//auto temp = currentPage->headFL;
		//int i = 1;
		//while ((char*)temp < (char*)currentPage + PAGESIZE - blockSize) 
		//{
		//	temp->nextFreeBlockIndex = i;
		//	i++;
		//	temp = (BlockHeader*)((char*)temp + blockSize);
		//}
		//temp->nextFreeBlockIndex = -1; // последний блок не имеет следующего
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
		}
		temp->nextFreeBlockIndex = -1; // последний блок не имеет следующего
	}
};

class CoalesceAllocator
{
public:
	CoalesceAllocator()
	{

	}

};

class MemoryAllocator
{
public:
	FixedSizeAllocator alloc16;
	//FixedSizeAllocator alloc32;
	//FixedSizeAllocator alloc64;
	//FixedSizeAllocator alloc128;
	//FixedSizeAllocator alloc256;
	//FixedSizeAllocator alloc512;
	//CoalesceAllocator coalAlloc;

	//LPVOID page = NULL;

	MemoryAllocator() : alloc16(16 * sizeof(char))			//Создает неинициализированный аллокатор
		/*				alloc32(32 * sizeof(char)),
						alloc64(64 * sizeof(char)), 
						alloc128(128 * sizeof(char)), 
						alloc256(256 * sizeof(char)), 
						alloc512(512 * sizeof(char))*/		
	{
		//Создает неинициализированный аллокатор
	}


	virtual ~MemoryAllocator() 
	{
		
	}

	virtual void Init()			//Выполняет инициализацию аллокатора, запрашивая необходимые страницы памяти у ОС
	{
		// инициализация всех аллокаторов
		alloc16.InitFSA();
		//alloc32.InitFSA();
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

	}

	virtual void* Alloc(size_t size)
	{
		return alloc16.GetFreeBlock();
		//if (size <= 16)
		//{
		//	return alloc16.GetFreeBlock();
		//}
	
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
		auto curPage = alloc16.currentPage;
		while (curPage != nullptr)
		{
			if ((char*)p > (char*)curPage && (char*)p < (char*)curPage + PAGESIZE)
				//&& ((char*)p - (char*)curPage) % alloc16.blockSize == 0) // проверка на попадание в интервал и на выравнивание по размерам ячеек
			{
				alloc16.SetFreeBlock(curPage, p); // помещаем ячейку в голову списка
				break;
			}
			else
			{
				curPage = curPage->nextPage;
			}
		}
	}

	virtual void DumpStat() const
	{

	}

	virtual void DumpBlocks() const
	{

	}
};

