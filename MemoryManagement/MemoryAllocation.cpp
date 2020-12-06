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
	struct BlockHeader
	{
		BlockHeader* nextFreeBlock = nullptr;	//индекс следующего свободного блока
		BlockHeader* prevFreeBlock = nullptr;	//индекс следующего свободного блока
		BlockHeader* nextBlock = nullptr;	//индекс следующего свободного блока
		BlockHeader* prevBlock = nullptr;	//индекс следующего свободного блока
		size_t blockSize; //размер текущего блока
		bool isFree = true; // свободен ли данный блок
	};

	struct PageHeader
	{
		BlockHeader* headFL = nullptr;	// Ссылка на первый свободный блок
		PageHeader* nextPage = nullptr;	//Ссылка на следующую страницу
	};

	size_t pageSize;
	PageHeader* mainPage = nullptr;
	bool isInit = false;
	int blocksCount = 0;

	CoalesceAllocator()
	{
		//pageSize = (size_t)(1024 * 1024 * 10) + sizeof(BlockHeader); //выделение страницы с запасом
		pageSize = (size_t)(1024 * 1024 * 10); //выделение страницы
	}

	~CoalesceAllocator()
	{
		//
	}

	void InitCA()
	{
		mainPage = AddNewPage();
		mainPage->headFL = (BlockHeader*)((char*)mainPage + sizeof(PageHeader));	// смещение указателя на первый свободный блок (сдвиг на велечину заголовка страницы)	
		mainPage->headFL->blockSize = pageSize - sizeof(PageHeader); // размер первого блока с учетом метаданных блока и без учета заголовка страницы
		mainPage->headFL->isFree = true;
		//mainPage->headFL->nextFreeBlockIndex++; //индекс следующего свободного блока
		//BlockHeadersInit(mainPage);	// инициализация заголовков блоков 
	}

	PageHeader* AddNewPage()
	{
		auto newPage = (PageHeader*)VirtualAlloc(
			NULL,
			pageSize,
			MEM_COMMIT,	//инициализирует страницу
			PAGE_READWRITE);
		isInit = true; // страница выделена, аллокатор проинициализирован

#ifdef _DEBUG
		assert(newPage != NULL);	// проверка на инициализацию страницы
		assert(isInit);
#endif // DEBUG	

		return newPage;
	}

	//void BlockHeadersInit(PageHeader* page)
	//{
		//
	//}

	void DestoyCA()
	{
		PageHeader* tempPage = mainPage;
		bool isReleased = false;
		while (tempPage->nextPage != nullptr)
		{
			PageHeader* tempNextPage = tempPage->nextPage;
			isReleased = VirtualFree((LPVOID)tempPage, 0, MEM_RELEASE);

#ifdef _DEBUG
			assert(isReleased);  // проверка на освобождение
#endif // DEBUG		

			tempPage = tempNextPage;
		}

		if (tempPage != nullptr)
		{
			isReleased = VirtualFree((LPVOID)tempPage, 0, MEM_RELEASE);

#ifdef _DEBUG
			assert(isReleased);  // проверка на освобождение
#endif // DEBUG	

			isInit = false; //аллокатор деинициализирован
		}
	}

	void* FindFreeBlock(PageHeader* page, size_t dataSize) // выделение свободного блока и установка связей между блоками
	{
		void* returnedFreeBlock = nullptr;
		auto currentFreeBlock = page->headFL;
		while (currentFreeBlock != nullptr) // ищем блок до тех пор, пока не достигнем конца фри-листа
		{
			auto test1 = currentFreeBlock->blockSize;
			auto test2 = dataSize + sizeof(BlockHeader);
			if (currentFreeBlock->blockSize > dataSize + sizeof(BlockHeader)) //если данные помещаются в блок памяти c запасом под заголовок
			{
				//freeBlock = (char*)currentFreeBlock;	//адрес возвращаемого пустого блока
				auto newBlock = (BlockHeader*)((char*)currentFreeBlock + sizeof(BlockHeader) + dataSize); // выделяем новый блок (делим текущий)
				newBlock->nextBlock = currentFreeBlock->nextBlock; // для нового блока следующим становится следующий для текущего
				newBlock->prevBlock = currentFreeBlock; //запоминаем предыдущий блок для следующего		
				newBlock->nextFreeBlock = currentFreeBlock->nextFreeBlock; // для нового блока запоминаем следующий свободный блок
				newBlock->prevFreeBlock = currentFreeBlock->prevFreeBlock;
				if (newBlock->nextFreeBlock != nullptr)
				{
					newBlock->nextFreeBlock->prevFreeBlock = newBlock;				
				}			
				currentFreeBlock->nextFreeBlock = nullptr; //разрыв старой связи
				
				newBlock->blockSize = currentFreeBlock->blockSize - sizeof(BlockHeader) - dataSize ; // вычисляем размер нового получившегося блока
				currentFreeBlock->blockSize = dataSize + sizeof(BlockHeader); // запоминаем новый размер текущей ячейки
				
				currentFreeBlock->nextBlock = newBlock; //запоминаем следующий блок для текущего				
				if (currentFreeBlock->prevFreeBlock != nullptr)
				{
					currentFreeBlock->prevFreeBlock->nextFreeBlock = newBlock; //установка следующего блока для предыдущего свободного блока					
				}						
				currentFreeBlock->prevFreeBlock = nullptr; //разрыв старой связи
				
				//prevBlock для текущего блока остается неизменным
				
				currentFreeBlock->isFree = false; // блок становится занятым (зарезервированым)
				newBlock->isFree = true; //новый блок свободен

				///////////////////////
				if (newBlock->prevFreeBlock == nullptr) // если новый элемент находится в голове/
				{
					page->headFL = newBlock; //переопределяем голову в новый, образовавшийся в ходе разделения блок
				}

				returnedFreeBlock = (char*)currentFreeBlock + sizeof(BlockHeader);
				break;
			}
			else // если данные не помещаются в блок
			{
				currentFreeBlock = currentFreeBlock->nextFreeBlock; //переходим к следующему блоку в списке
			}
		}
		return returnedFreeBlock;
		//void* freeBlock = nullptr;
		//auto currentFreeBlock = page->headFL;
		//while (currentFreeBlock != nullptr) // ищем блок до тех пор, пока не достигнем конца фри-листа
		//{
		//	if (currentFreeBlock->blockSize - sizeof(BlockHeader) > dataSize + sizeof(BlockHeader)) //если данные помещаются в блок памяти c запасом под заголовок
		//	{
		//		freeBlock = (char*)currentFreeBlock + sizeof(BlockHeader);	//адрес возвращаемого пустого блока (часть блока с данными)
		//		auto newBlock = (BlockHeader*)((char*)freeBlock + dataSize); // выделяем новый блок (делим текущий)
		//		newBlock->nextBlock = currentFreeBlock->nextBlock; // для нового блока следующим становится следующий для текущего
		//		newBlock->nextFreeBlock = currentFreeBlock->nextFreeBlock; // для нового блока запоминаем следующий свободный блок
		//		newBlock->prevFreeBlock = currentFreeBlock->prevFreeBlock;
		//		newBlock->nextFreeBlock->prevFreeBlock = newBlock;
		//		newBlock->blockSize = currentFreeBlock->blockSize - dataSize;
		//		currentFreeBlock->nextBlock = newBlock; //запоминаем следующий блок для текущего
		//		currentFreeBlock->isFree = false; // блок становится занятым (зарезервированым)
		//		currentFreeBlock->prevFreeBlock->nextFreeBlock = newBlock; //установка следующего блока для предыдущего свободного блока
		//		currentFreeBlock->prevFreeBlock = nullptr;
		//		currentFreeBlock->nextFreeBlock = nullptr;
		//		
		//		currentFreeBlock->blockSize = dataSize + sizeof(BlockHeader); // запоминаем новый размер текущей ячейки
		//		//prev для текущего блока остается неизменным
		//		newBlock->prevBlock = currentFreeBlock; //запоминаем предыдущий блок для следующего		
		//		newBlock->isFree = true; //новый блок свободен
		//		
		//		if (newBlock->nextFreeBlock != nullptr)
		//		{
		//			page->headFL = newBlock; //переопределяем голову в новый, образовавшийся в ходе разделения блок
		//		}
		//		
		//		
		//	}
		//	else // если данные не помещаются в блок
		//	{
		//		currentFreeBlock = currentFreeBlock->nextFreeBlock; //переходим к следующему блоку в списке
		//	}
		//}		
		//return freeBlock;
	}

	void* GetFreeBlock(size_t dataSize) //dataSize - объем хранимых данных, передаваемый в аллок
	{
		auto currentPage = mainPage; // запоминаем текущую страницу начиная с главной	

		//while (currentPage->headFL == nullptr) //ищем страницу со свободными блоками
		//{
		//	currentPage = currentPage->nextPage; // если в этой странице нет свободных блоков, переходим на следующую
		//}
		void* currentFreeBlock;
		while (true)
		{	
			currentFreeBlock = FindFreeBlock(currentPage, dataSize);//поиск свободного блока в странице
			if (currentFreeBlock == nullptr) // свободный блок не найден
			{
				if (currentPage->nextPage != nullptr) //если есть следующая страница
				{
					currentPage = currentPage->nextPage; //то переходим в нее и ищем там
				}
				else // если следующей страницы нет, то создаем новую страницу
				{
					auto newPage = AddNewPage(); //добавляем новую страницу
					newPage->headFL = (BlockHeader*)((char*)mainPage + sizeof(PageHeader)); //добавляем голову списка в новую страницу
					currentPage->nextPage = newPage; //записываем новую страницу в next для текущей страницы	
					currentPage = newPage; // переходим в новую страницу и ищем там
				}
			}
			else
			{
				break;
			}
				
		}

		return currentFreeBlock;

		//if (currentPage->headFL->blockSize - sizeof(BlockHeader) > dataSize + sizeof(BlockHeader)) //если данные помещаются в блок памяти c запасом под заголовок
		//{
		//	currentFreeBlock = (char*)currentPage->headFL + sizeof(BlockHeader);	//адрес текущего пустого блока (часть с данными в блоке)
		//	auto tmp = (BlockHeader*)((char*)currentFreeBlock + dataSize); // выделяем новый блок (делим текущий)
		//	currentPage->headFL->nextBlock = tmp; //запоминаем следующий блок для текущего
		//	currentPage->headFL = tmp; //переопределяем голову		
		//	
		//	//if (currentPage->headFL->nextFreeBlock == nullptr) // если у текущего элемента нет следующего свободного
		//	//{
		//	//	currentPage->headFL = tmp; //переопределяем голову		
		//	//}
		//	//else
		//	//{
		//	//	currentPage->headFL = currentPage->headFL->nextFreeBlock;
		//	//}		
		//}
		//else if(currentPage->headFL->nextFreeBlock != nullptr)// если данные не помещаются в блок
		//{
		//	currentPage->headFL = currentPage->headFL->nextFreeBlock;
		//}

		//
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
		//	currentPage->headFL = nullptr;
		//}
		//return currentFreeBlock;
	}

	void SetFreeBlock(void *p)
	{
		//
	}

	size_t GetPageSize()
	{
		return pageSize;
	}
};

class MemoryAllocator
{
public:
	FixedSizeAllocator alloc16;
	FixedSizeAllocator alloc32;
	FixedSizeAllocator alloc64;
	FixedSizeAllocator alloc128;
	FixedSizeAllocator alloc256;
	FixedSizeAllocator alloc512;
	CoalesceAllocator coalAlloc;

	bool isInit = false;

	//LPVOID page = NULL;

	MemoryAllocator() : alloc16(16 * sizeof(char)),			//Создает неинициализированный аллокатор
						alloc32(32 * sizeof(char)),
						alloc64(64 * sizeof(char)), 
						alloc128(128 * sizeof(char)), 
						alloc256(256 * sizeof(char)), 
						alloc512(512 * sizeof(char)),
						coalAlloc()
	{
		isInit = false;	// аллокатор не инициализирован
	}


	virtual ~MemoryAllocator() 
	{
		if (isInit == true) //если дестрой не был вызван
		{
			Destroy();
		}
	}

	virtual void Init()			//Выполняет инициализацию аллокатора, запрашивая необходимые страницы памяти у ОС
	{
		// инициализация всех аллокаторов
		alloc16.InitFSA();
		alloc32.InitFSA();
		alloc64.InitFSA();
		alloc128.InitFSA();
		alloc256.InitFSA();
		alloc512.InitFSA();
		////////////////////
		coalAlloc.InitCA();
		isInit = true; //аллокатор проинициализирован

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
			// destroy all
			alloc16.DestoyFSA();
			alloc32.DestoyFSA();
			alloc64.DestoyFSA();
			alloc128.DestoyFSA();
			alloc256.DestoyFSA();
			alloc512.DestoyFSA();			
			//////////////////
			coalAlloc.DestoyCA();
			isInit = false; //аллокатор деинициализирован
		}		
	}

	virtual void* Alloc(size_t size)
	{
		if (isInit == false) // если аллокаторы не были проинициализированны 
		{
			assert(isInit);
			//return nullptr;
		}

		size_t FSA_metaDataSize = 1 * sizeof(int); // размер метаданных в каждом блоке для FSA аллокатора
		size_t CA_metaDataSize = 24; // размер метаданных в блоке для CA аллокатора

		//вызов в зависимости от аллокатора
		if (size <= 16 - FSA_metaDataSize) // меньше 12 байт с учетом служебных данных в блоке
		{
			return alloc16.GetFreeBlock();
		}		
		else if (size > 16 - FSA_metaDataSize && size <= 32 - FSA_metaDataSize) //от 12 до 28 байт
		{
			return alloc32.GetFreeBlock();
		}	
		else if (size > 32 - FSA_metaDataSize && size <= 64 - FSA_metaDataSize) //от 28 до 60 байт
		{
			return alloc64.GetFreeBlock();
		}		
		else if (size > 64 - FSA_metaDataSize && size <= 128 - FSA_metaDataSize) //от 60 до 124 байт
		{
			return alloc128.GetFreeBlock();
		}	
		else if (size > 128 - FSA_metaDataSize && size <= 256 - FSA_metaDataSize) //от 124 до 254 байт
		{
			return alloc256.GetFreeBlock();
		}
		else if (size > 256 - FSA_metaDataSize && size <= 512 - FSA_metaDataSize) //от 254 до 508 байт
		{
			return alloc512.GetFreeBlock();
		}
		else if (size > 512 - FSA_metaDataSize && size <= (1024 * 1024 * 10) - CA_metaDataSize) //от 506 байт до 10 Мбайт
		{
			return coalAlloc.GetFreeBlock(size); //возврат памяти вделенной коалес аллокатором
		}
		else if (size > (1024 * 1024 * 10) - CA_metaDataSize) //больше 10 Мбайт
		{
			auto newPage = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);

#ifdef _DEBUG
			assert(newPage != NULL);	// проверка на инициализацию страницы
#endif // DEBUG	

			return newPage;
		}
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
		
		if (isInit == false) // если аллокаторы не были проинициализированны 
		{
			return;
		}
		
		//вызов в зависимости от аллокатора
		if ((char*)p > (char*)alloc16.mainPage && (char*)p < (char*)alloc16.mainPage + PAGESIZE)
		{
			alloc16.SetFreeBlock(p);
		}
		else if ((char*)p > (char*)alloc32.mainPage && (char*)p < (char*)alloc32.mainPage + PAGESIZE)
		{
			alloc32.SetFreeBlock(p);
		}
		else if ((char*)p > (char*)alloc64.mainPage && (char*)p < (char*)alloc64.mainPage + PAGESIZE)
		{
			alloc64.SetFreeBlock(p);
		}
		else if ((char*)p > (char*)alloc128.mainPage && (char*)p < (char*)alloc128.mainPage + PAGESIZE)
		{
			alloc128.SetFreeBlock(p);
		}
		else if ((char*)p > (char*)alloc256.mainPage && (char*)p < (char*)alloc256.mainPage + PAGESIZE)
		{
			alloc256.SetFreeBlock(p);
		}
		else if ((char*)p > (char*)alloc512.mainPage && (char*)p < (char*)alloc512.mainPage + PAGESIZE)
		{
			alloc512.SetFreeBlock(p);
		}
		else if ((char*)p > (char*)coalAlloc.mainPage && (char*)p < (char*)coalAlloc.mainPage + coalAlloc.GetPageSize())
		{
			coalAlloc.SetFreeBlock(p);
		}
		else return;/////

	}


#ifdef _DEBUG
	virtual void DumpStat() const
	{
		//
	}

	virtual void DumpBlocks() const
	{
		//
	}
#endif // DEBUG	

};

