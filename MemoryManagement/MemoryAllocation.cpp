#include "pch.h"
#include <iostream>
#include "cassert"
#include "windows.h"

#define PAGESIZE 4096
#define MEGABYTE 1024*1024

enum AllocatorType
{
	Uncalled = 0,
	FSA16 = 1,
	FSA32 = 2,
	FSA64 = 3,
	FSA128 = 4,
	FSA256 = 5,
	FSA512 = 6,
	CA = 7,
	OS = 8
};

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

#ifdef _DEBUG
	int blocksCount;
	int freeBlocksCount;
#endif//DEBUG

	FixedSizeAllocator(size_t blockSize)
	{	
		this->blockSize = blockSize;
#ifdef _DEBUG
		blocksCount = 0;
		freeBlocksCount = 0;
#endif//DEBUG
	}

	~FixedSizeAllocator()
	{
		if (isInit == true) //если аллокатор не был деинициализирован вручную
		{
			DestoyFSA();
		}
	}

	void InitFSA()
	{
		mainPage = AddNewPage(); // выделение страницы и инициализация аллокатора
		mainPage->headFL = (BlockHeader*)((char*)mainPage + blockSize);	// смещение указателя на первый свободный блок без учета индекса след. эл.
		BlockHeadersInit(mainPage);	// инициализация заголовков блоков и количества всех блоков
	}

	void DestoyFSA() //вызов АллокФри для аллокатора
	{
		PageHeader* tempPage = mainPage;
		bool isReleased = false;
		while (tempPage->nextPage != nullptr)
		{
			PageHeader* tempNextPage = tempPage->nextPage;
			isReleased = VirtualFree((LPVOID)tempPage, 0, MEM_RELEASE);			
#ifdef _DEBUG
			assert(isReleased);  // проверка на освобождение
#endif//DEBUG		
			tempPage = tempNextPage;
		}

		if (tempPage != nullptr)
		{
			isReleased = VirtualFree((LPVOID)tempPage, 0, MEM_RELEASE);
#ifdef _DEBUG
			assert(isReleased);  // проверка на освобождение
#endif//DEBUG	
		}

#ifdef _DEBUG
		blocksCount = 0;
		freeBlocksCount = 0;
#endif//DEBUG

		isInit = false; //аллокатор деинициализирован
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

#ifdef _DEBUG
		freeBlocksCount--; //уменьшение количества свободных блоков
#endif//DEBUG

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
#ifdef _DEBUG
				freeBlocksCount++; // увеличение количества свободных блоков
#endif//DEBUG
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
		auto newPage = (PageHeader*)VirtualAlloc(NULL, PAGESIZE, MEM_COMMIT, PAGE_READWRITE); //инициализирует страницу 
		isInit = true; // страница выделена, аллокатор проинициализирован
#ifdef _DEBUG
		assert(newPage != NULL);	// проверка на инициализацию страницы
		assert(isInit);
#endif//DEBUG	

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
			//blocksCount++; // подсчет количества блоков всего
		}
		temp->nextFreeBlockIndex = -1; // последний блок не имеет следующего
		//blocksCount++;

#ifdef _DEBUG
		blocksCount += i; // количество блоков в аллокаторе всего
		freeBlocksCount += i; // количество свободных блоков в аллокаторе всего
#endif//DEBUG

	}

	bool IsAllocatorContainPointer(void* p)
	{
		auto currentPage = mainPage;
		bool isContaine = false;
		while (currentPage != nullptr)
		{
			if ((char*)p > (char*)currentPage && (char*)p < (char*)currentPage + PAGESIZE)
			{
				isContaine = true;
			}
			currentPage = currentPage->nextPage;
		}	
		return isContaine;
	}

#ifdef _DEBUG
	const int GetFreeBlocksCount() const
	{
		return freeBlocksCount;
	}

	const int GetAllBlocksCount() const
	{
		return blocksCount;
	}

	void PrintAllBusyBlocks() const // поиск занятых блоков
	{
		std::cout << "----------------------------------- \n";
		std::cout << "FSAx" << blockSize << "busy blocks: \n";
		auto currentPage = mainPage;
		//auto currentBlock = (BlockHeader*)((char*)currentPage);
		auto currentBlock = (BlockHeader*)((char*)currentPage + blockSize);
		while (currentPage != nullptr)
		{
			while (currentBlock->nextFreeBlockIndex != -1)
			{
				auto head = currentPage->headFL;
				currentBlock = (BlockHeader*)((char*)currentBlock + blockSize);
				bool isBusy = true;
				while (head != nullptr &&  head->nextFreeBlockIndex != -1)
				{
					if (head != currentBlock)
						head = (BlockHeader*)((char*)currentPage + (head->nextFreeBlockIndex * blockSize) + blockSize);
					else
					{
						isBusy = false;
						break;
					}
				}
				if (isBusy == true)
					std::cout << "block: " << currentBlock << "; size: " << blockSize << "\n";			
			}
			currentPage = currentPage->nextPage;
		}	
	}
#endif//DEBUG	

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

#ifdef _DEBUG
	int blocksCount;
	int freeBlocksCount;
#endif//DEBUG

	CoalesceAllocator()
	{
		pageSize = (size_t)(1024 * 1024 * 10); //размер выделяемых страниц

#ifdef _DEBUG
		blocksCount = 0;
		freeBlocksCount = 0;
#endif//DEBUG
	}

	~CoalesceAllocator()
	{
		if (isInit == true) //если аллокатор не был деинициализирован вручную
		{
			DestoyCA();
		}
	}

	void InitCA()
	{
		mainPage = AddNewPage();
		mainPage->headFL = (BlockHeader*)((char*)mainPage + sizeof(PageHeader));	// смещение указателя на первый свободный блок (сдвиг на велечину заголовка страницы)	
		mainPage->headFL->blockSize = pageSize - sizeof(PageHeader); // размер первого блока с учетом метаданных блока и без учета заголовка страницы
		mainPage->headFL->isFree = true;
#ifdef _DEBUG
		blocksCount++;
		freeBlocksCount++;
#endif//DEBUG
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
		
		}

#ifdef _DEBUG
		blocksCount = 0;
		freeBlocksCount = 0;
#endif//DEBUG

		isInit = false; //аллокатор деинициализирован
	}

	void* FindFreeBlock(PageHeader* page, size_t dataSize) // выделение свободного блока и установка связей между блоками
	{
		void* returnedFreeBlock = nullptr;
		auto currentFreeBlock = page->headFL;
		while (currentFreeBlock != nullptr) // ищем блок до тех пор, пока не достигнем конца фри-листа
		{
	/*		auto test1 = currentFreeBlock->blockSize;
			auto test2 = dataSize + sizeof(BlockHeader);*/
			if (currentFreeBlock->blockSize > dataSize + sizeof(BlockHeader)) //если данные помещаются в блок памяти c запасом под заголовок
			{
				//freeBlock = (char*)currentFreeBlock;	//адрес возвращаемого пустого блока
				//разделение блока
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

				returnedFreeBlock = (char*)currentFreeBlock + sizeof(BlockHeader);//возвращемый блок без служебной инфы
#ifdef _DEBUG
				blocksCount++; // добавляем новый блок
#endif//DEBUG
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
					newPage->headFL = (BlockHeader*)((char*)newPage + sizeof(PageHeader)); //добавляем голову списка в новую страницу
					newPage->headFL->blockSize = pageSize - sizeof(PageHeader);
					currentPage->nextPage = newPage; //записываем новую страницу в next для текущей страницы	
					currentPage = newPage; // переходим в новую страницу и ищем там
#ifdef _DEBUG
					blocksCount++; //
					freeBlocksCount++; // добавляем свободный блок
#endif//DEBUG
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

	void SetFreeBlock(PageHeader* page, void *p) //page это страница в которой был найден указатель
	{
		auto currentPage = page;

		BlockHeader* currentBlock = (BlockHeader*)((char*)p - sizeof(BlockHeader)); // переходим в метаданные блока
		if (currentBlock->prevBlock !=nullptr && currentBlock->prevBlock->isFree)//если сободен сосед слева
		{
			auto leftBlock = currentBlock->prevBlock; //сосед слева
			leftBlock->nextBlock = currentBlock->nextBlock;//
			if (currentBlock->nextBlock != nullptr)
				currentBlock->nextBlock->prevBlock = leftBlock;//
			leftBlock->blockSize += currentBlock->blockSize;
			currentBlock = leftBlock;
			currentBlock->isFree = true;
#ifdef _DEBUG
			blocksCount--; // уменьшение количества блоков т.к. блоки объединяются
			if (currentBlock->nextBlock != nullptr && currentBlock->nextBlock->isFree == true)
				freeBlocksCount--;
#endif//DEBUG
		}

		if (currentBlock->nextBlock != nullptr && currentBlock->nextBlock->isFree) //если сободен сосед справа
		{
			auto rightBlock = currentBlock->nextBlock; //сосед справа
			currentBlock->nextBlock = rightBlock->nextBlock; //следующий блок для текущего
			if (rightBlock->nextBlock != nullptr)
				rightBlock->nextBlock->prevBlock = currentBlock; //предыдущий блок для следующего
			currentBlock->blockSize += rightBlock->blockSize;

			currentBlock->nextFreeBlock = rightBlock->nextFreeBlock;
			currentBlock->prevFreeBlock = rightBlock->prevFreeBlock;
			if (currentPage->headFL == rightBlock)
			{
				currentPage->headFL = currentBlock;
			}
			currentBlock->isFree = true;
#ifdef _DEBUG
			blocksCount--; // уменьшение количества блоков т.к. блоки объединяются
#endif//DEBUG
		}

		// если блок находится между двумя занятыми блоками, либо слева пусто и справа занятый блок, либо справа пусто и слева занятый блок
		if (currentBlock->nextBlock != nullptr && currentBlock->prevBlock != nullptr && currentBlock->nextBlock->isFree == false && currentBlock->prevBlock->isFree == false 
			|| currentBlock->nextBlock == nullptr && currentBlock->prevBlock != nullptr && currentBlock->prevBlock->isFree == false
			|| currentBlock->nextBlock != nullptr && currentBlock->prevBlock == nullptr && currentBlock->nextBlock->isFree == false)
		{
			if (currentPage->headFL != nullptr)
				currentPage->headFL->prevFreeBlock = currentBlock;
			currentBlock->nextFreeBlock = currentPage->headFL;
			currentBlock->prevFreeBlock = nullptr;
			currentPage->headFL = currentBlock;
			currentBlock->isFree = true;
#ifdef _DEBUG
			freeBlocksCount++; // увеличение количества свободных блоков
#endif//DEBUG
		}		
	}

	size_t GetPageSize()
	{
		return pageSize;
	}

	//bool IsAllocatorContainPointer(void* p)
	//{
	//	auto currentPage = mainPage;
	//	bool isContaine = false;
	//	while (currentPage != nullptr)
	//	{
	//		if ((char*)p > (char*)currentPage && (char*)p < (char*)currentPage + pageSize)
	//		{
	//			isContaine = true;
	//		}
	//		currentPage = currentPage->nextPage;
	//	}
	//	return isContaine;
	//}

	PageHeader* IsAllocatorContainPointer(void* p) // определяет, находится ли указатель в данном аллокаторе и если да, то возвращет страницу, в которой он находится
	{
		auto currentPage = mainPage;
		//bool isContaine = false;
		while (currentPage != nullptr)
		{
			if ((char*)p > (char*)currentPage && (char*)p < (char*)currentPage + pageSize)
			{
				break;
			}
			currentPage = currentPage->nextPage;
		}
		return currentPage;
	}

#ifdef _DEBUG
	const int GetFreeBlocksCount() const
	{
		return freeBlocksCount;
	}

	const int GetAllBlocksCount() const
	{
		return blocksCount;
	}

	void PrintAllBusyBlocks() const // поиск занятых блоков
	{
		//std::cout << "----------------------------------- \n";
		//std::cout << "FSAx" << blockSize << "busy blocks: \n";
		//auto currentPage = mainPage;
		////auto currentBlock = (BlockHeader*)((char*)currentPage);
		//auto currentBlock = (BlockHeader*)((char*)currentPage + blockSize);
		//while (currentPage != nullptr)
		//{
		//	while (currentBlock->nextFreeBlockIndex != -1)
		//	{
		//		auto head = currentPage->headFL;
		//		currentBlock = (BlockHeader*)((char*)currentBlock + blockSize);
		//		bool isBusy = true;
		//		while (head != nullptr &&  head->nextFreeBlockIndex != -1)
		//		{
		//			if (head != currentBlock)
		//				head = (BlockHeader*)((char*)currentPage + (head->nextFreeBlockIndex * blockSize) + blockSize);
		//			else
		//			{
		//				isBusy = false;
		//				break;
		//			}
		//		}
		//		if (isBusy == true)
		//			std::cout << "block: " << currentBlock << "; size: " << blockSize << "\n";
		//	}
		//	currentPage = currentPage->nextPage;
		//}
	}
#endif//DEBUG	
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

	LPVOID OS_mainPage = NULL; //страница запрошенная непосредственно у ОС
	LPVOID OS_nextPage = NULL;

#ifdef _DEBUG
	AllocatorType currentAllocatorType = Uncalled;	// метод какого аллокатора был вызван
#endif//DEBUG	

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

		//OS_mainPage = VirtualAlloc(
		//	OS_mainPage,
		//	PAGESIZE,
		//	MEM_RESERVE,			//резервирует память
		//	PAGE_READWRITE);
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
		
		//OS_mainPage = VirtualAlloc(
		//	NULL,
		//	PAGESIZE,
		//	MEM_COMMIT,			//инициализирует страницу в ранее зарезервированную область
		//	PAGE_READWRITE);

		isInit = true; //аллокатор проинициализирован
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
		//if (isInit == false) // если аллокаторы не были проинициализированны 
		//{
		assert(isInit);
			//return nullptr;
		//}

		size_t FSA_metaDataSize = 1 * sizeof(int); // размер метаданных в каждом блоке для FSA аллокатора
		size_t CA_metaDataSize = sizeof(CoalesceAllocator::BlockHeader); // размер метаданных в блоке для CA аллокатора

		//вызов в зависимости от аллокатора
		if (size <= 16 - FSA_metaDataSize) // меньше 12 байт с учетом служебных данных в блоке
		{
#ifdef _DEBUG
			currentAllocatorType = FSA16;
#endif//DEBUG			
			return alloc16.GetFreeBlock();
		}		
		else if (size > 16 - FSA_metaDataSize && size <= 32 - FSA_metaDataSize) //от 12 до 28 байт
		{
#ifdef _DEBUG
			currentAllocatorType = FSA32;
#endif//DEBUG				
			return alloc32.GetFreeBlock();
		}	
		else if (size > 32 - FSA_metaDataSize && size <= 64 - FSA_metaDataSize) //от 28 до 60 байт
		{
#ifdef _DEBUG
			currentAllocatorType = FSA64;
#endif//DEBUG			
			return alloc64.GetFreeBlock();
		}		
		else if (size > 64 - FSA_metaDataSize && size <= 128 - FSA_metaDataSize) //от 60 до 124 байт
		{
#ifdef _DEBUG
			currentAllocatorType = FSA128;
#endif // DEBUG	
			return alloc128.GetFreeBlock();
		}	
		else if (size > 128 - FSA_metaDataSize && size <= 256 - FSA_metaDataSize) //от 124 до 254 байт
		{
#ifdef _DEBUG
			currentAllocatorType = FSA256;
#endif//DEBUG	
			return alloc256.GetFreeBlock();
		}
		else if (size > 256 - FSA_metaDataSize && size <= 512 - FSA_metaDataSize) //от 254 до 508 байт
		{
#ifdef _DEBUG
			currentAllocatorType = FSA512;
#endif//DEBUG	
			return alloc512.GetFreeBlock();
		}
		else if (size > 512 - FSA_metaDataSize && size <= (1024 * 1024 * 10) - CA_metaDataSize) //от 506 байт до 10 Мбайт
		{
#ifdef _DEBUG
			currentAllocatorType = CA;
#endif//DEBUG	
			return coalAlloc.GetFreeBlock(size); //возврат памяти выделенной коалес аллокатором
		}
		else if (size > (1024 * 1024 * 10) - CA_metaDataSize) //больше 10 Мбайт
		{
			OS_mainPage = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
#ifdef _DEBUG
			currentAllocatorType = OS;
			assert(OS_mainPage != NULL);	// проверка на инициализацию страницы
#endif//DEBUG	
			return OS_mainPage;
		}
	}

	virtual void Free(void *p)
	{
		//if (isInit == false) // если аллокаторы не были проинициализированны 
		//{
		//	return;
		//}
		assert(isInit);
		
		//вызов в зависимости от аллокатора
		auto page = coalAlloc.IsAllocatorContainPointer(p);
		if (page != nullptr)
		{
#ifdef _DEBUG
			currentAllocatorType = CA;
#endif//DEBUG	
			coalAlloc.SetFreeBlock(page, p);
			return;	
		}

		if (alloc16.IsAllocatorContainPointer(p))
		{
#ifdef _DEBUG
			currentAllocatorType = FSA16;
#endif//DEBUG
			alloc16.SetFreeBlock(p);
		}
		else if (alloc32.IsAllocatorContainPointer(p))
		{
#ifdef _DEBUG
			currentAllocatorType = FSA32;
#endif//DEBUG
			alloc32.SetFreeBlock(p);
		}
		else if (alloc64.IsAllocatorContainPointer(p))
		{
#ifdef _DEBUG
			currentAllocatorType = FSA64;
#endif//DEBUG
			alloc64.SetFreeBlock(p);
		}
		else if (alloc128.IsAllocatorContainPointer(p))
		{
#ifdef _DEBUG
			currentAllocatorType = FSA128;
#endif//DEBUG
			alloc128.SetFreeBlock(p);
		}
		else if (alloc256.IsAllocatorContainPointer(p))
		{
#ifdef _DEBUG
			currentAllocatorType = FSA256;
#endif//DEBUG
			alloc256.SetFreeBlock(p);
		}
		else if (alloc512.IsAllocatorContainPointer(p))
		{
#ifdef _DEBUG
			currentAllocatorType = FSA512;
#endif//DEBUG
			alloc512.SetFreeBlock(p);
		}
		//else 
		//{
		//	auto page = coalAlloc.IsAllocatorContainPointer(p);
		//	if (page != nullptr)
		//	{
		//		coalAlloc.SetFreeBlock(page, p);
		//		return;
		//	}			
		//}
		else
		{
			bool isReleased = VirtualFree((LPVOID)p, 0, MEM_RELEASE);
#ifdef _DEBUG
			currentAllocatorType = OS;
			assert(isReleased);  // проверка на освобождение
#endif // DEBUG	
		}


//		if ((char*)p > (char*)alloc16.mainPage && (char*)p < (char*)alloc16.mainPage + PAGESIZE)
//		{
//			alloc16.SetFreeBlock(p);
//		}
//		else if ((char*)p > (char*)alloc32.mainPage && (char*)p < (char*)alloc32.mainPage + PAGESIZE)
//		{
//			alloc32.SetFreeBlock(p);
//		}
//		else if ((char*)p > (char*)alloc64.mainPage && (char*)p < (char*)alloc64.mainPage + PAGESIZE)
//		{
//			alloc64.SetFreeBlock(p);
//		}
//		else if ((char*)p > (char*)alloc128.mainPage && (char*)p < (char*)alloc128.mainPage + PAGESIZE)
//		{
//			alloc128.SetFreeBlock(p);
//		}
//		else if ((char*)p > (char*)alloc256.mainPage && (char*)p < (char*)alloc256.mainPage + PAGESIZE)
//		{
//			alloc256.SetFreeBlock(p);
//		}
//		else if ((char*)p > (char*)alloc512.mainPage && (char*)p < (char*)alloc512.mainPage + PAGESIZE)
//		{
//			alloc512.SetFreeBlock(p);
//		}
//		else if ((char*)p > (char*)coalAlloc.mainPage && (char*)p < (char*)coalAlloc.mainPage + coalAlloc.GetPageSize())
//		{
//			coalAlloc.SetFreeBlock(p);
//		}
//		else
//		{
//			bool isReleased = VirtualFree((LPVOID)p, 0, MEM_RELEASE);
//
//#ifdef _DEBUG
//			assert(isReleased);  // проверка на освобождение
//#endif // DEBUG	
//		}
	}


#ifdef _DEBUG
	AllocatorType GetCurrentAllocatorType()
	{
		return currentAllocatorType;
	}

	virtual void DumpStat() const
	{
		//
		std::cout << "----------------------------------- \n";
		std::cout << "FSAx16 bloks count: " << alloc16.GetAllBlocksCount() << "; free bloks count: " << alloc16.GetFreeBlocksCount() << std::endl;
		std::cout << "FSAx32 bloks count: " << alloc32.GetAllBlocksCount() << "; free bloks count: " << alloc32.GetFreeBlocksCount() << std::endl;
		std::cout << "FSAx64 bloks count: " << alloc64.GetAllBlocksCount() << "; free bloks count: " << alloc64.GetFreeBlocksCount() << std::endl;
		std::cout << "FSAx128 bloks count: " << alloc128.GetAllBlocksCount() << "; free bloks count: " << alloc128.GetFreeBlocksCount() << std::endl;
		std::cout << "FSAx256 bloks count: " << alloc256.GetAllBlocksCount() << "; free bloks count: " << alloc256.GetFreeBlocksCount() << std::endl;
		std::cout << "FSAx512 bloks count: " << alloc512.GetAllBlocksCount() << "; free bloks count: " << alloc512.GetFreeBlocksCount() << std::endl;
		std::cout << "CA bloks count: " << coalAlloc.GetAllBlocksCount() << "; free bloks count: " << coalAlloc.GetFreeBlocksCount() << std::endl;
	}

	virtual void DumpBlocks() const
	{
		//
		//std::cout << "----------------------------------- \n";
		alloc16.PrintAllBusyBlocks();
		alloc32.PrintAllBusyBlocks();
		alloc64.PrintAllBusyBlocks();
		alloc128.PrintAllBusyBlocks();
		alloc256.PrintAllBusyBlocks();
		alloc512.PrintAllBusyBlocks();
	}
#endif // DEBUG	

};


