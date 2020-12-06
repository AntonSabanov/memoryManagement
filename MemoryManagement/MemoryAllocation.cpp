#include "pch.h"
//#include <iostream>
#include "cassert"
#include "windows.h"

#define PAGESIZE 4096
#define MEGABYTE 1024*1024

class FixedSizeAllocator //Page
{
public:
	struct BlockHeader
	{
		//BlockHeader* nextFreeBlock = nullptr;		// ��������� ��������� ����
		int nextFreeBlockIndex;	//������ ���������� ���������� �����
	};

	struct PageHeader
	{
		BlockHeader* headFL = nullptr;	// ������ �� ������ ��������� ����
		PageHeader* nextPage = nullptr;	//������ �� ��������� ��������
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
		if (isInit == true) //���� ��������� �� ��� ����������������� �������
		{
			DestoyFSA();
		}
	}

	void InitFSA()
	{
		mainPage = AddNewPage(); // ��������� �������� � ������������� ����������
		mainPage->headFL = (BlockHeader*)((char*)mainPage + blockSize);	// �������� ��������� �� ������ ��������� ���� ��� ����� ������� ����. ��.
		BlockHeadersInit(mainPage);	// ������������� ���������� ������ 
	}

	void DestoyFSA() //����� �������� ��� ����������
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
			assert(isReleased);  // �������� �� ������������
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
			assert(isReleased);  // �������� �� ������������
#endif // DEBUG	
		}

		isInit = false; //��������� �����������������
	}

	void* GetFreeBlock()
	{	
		//void* currentFreeBlock;
		auto currentPage = mainPage; // ���������� ������� �������� ������� � �������

		//while (currentPage->headFL->nextFreeBlockIndex < 0) //���� �������� �� ����������� �������
		//{
		//	if (currentPage->nextPage != nullptr)
		//	{
		//		//currentFreeBlock = (char*)currentPage->headFL + sizeof(int); //
		//		currentPage = currentPage->nextPage; //��������� � ��������� ��������
		//	}
		//	else
		//	{
		//		auto newPage = AddNewPage();	//��������� ����� ��������
		//		newPage->headFL = (BlockHeader*)((char*)newPage + blockSize);	//��������� ������ ������ � ����� ��������
		//		BlockHeadersInit(newPage);	//�������������� ����� � ����� ��������
		//		currentPage->nextPage = newPage;	//���������� ����� �������� � next ��� ������� ��������		
		//		currentPage = currentPage->nextPage; //��������� � ��������� ��������
		//	}				
		//}			

		while (currentPage->headFL == nullptr) //���� �������� �� ����������� �������
		{
			currentPage = currentPage->nextPage; // ���� � ���� �������� ��� ��������� ������, ��������� �� ���������
		}

		auto currentFreeBlock = (char*)currentPage->headFL + sizeof(int);			//����� �������� ������� ����� � ������ ������� ����������� (����� � ������� � �����)
		int currentNextIndex = currentPage->headFL->nextFreeBlockIndex;				//������ ���������� ����� ����� ��������
		//currentPage->headFL = (BlockHeader*)((char*)currentPage + (currentNextIndex * blockSize) + blockSize); // ������� �� ��������� ��������� ���� �� ��� ������� (��������� ��� ����� ������� ���� ��.)
		
		if (currentNextIndex > 0)
		{
			currentPage->headFL = (BlockHeader*)((char*)currentPage + (currentNextIndex * blockSize) + blockSize);// ������� �� ��������� ��������� ���� �� ��� ������� (��������� ��� ����� ������� ���� ��.)		
		}		
		else
		{
			auto newPage = AddNewPage();											//��������� ����� ��������
			newPage->headFL = (BlockHeader*)((char*)newPage + blockSize);			//��������� ������ ������ � ����� ��������
			BlockHeadersInit(newPage);												//�������������� ����� � ����� ��������
			currentPage->nextPage = newPage;										//���������� ����� �������� � next ��� ������� ��������		
			currentPage->headFL = nullptr;
		}

		//if (currentNextIndex > 0)
		//{
		//	currentPage->headFL = (BlockHeader*)((char*)currentPage + (currentNextIndex * blockSize) + blockSize);// ������� �� ��������� ��������� ���� �� ��� ������� (��������� ��� ����� ������� ���� ��.)
		//}		
		//else
		//{
		//	auto newPage = AddNewPage();											//��������� ����� ��������
		//	newPage->headFL = (BlockHeader*)((char*)newPage + blockSize);			//��������� ������ ������ � ����� ��������
		//	BlockHeadersInit(newPage);												//�������������� ����� � ����� ��������
		//	currentPage->nextPage = newPage;										//���������� ����� �������� � next ��� ������� ��������		
		//}

		return currentFreeBlock;
	}

	//void SetFreeBlock(PageHeader* curPage, void *p)	// � ������� ���������� ������� ��������, � ������� ��������� ��������� � ��� ���������
	void SetFreeBlock(void *p)
	{
		auto curPage = mainPage;
		while (curPage != nullptr)
		{
			if ((char*)p > (char*)curPage && (char*)p < (char*)curPage + PAGESIZE)
				//&& ((char*)p - (char*)curPage) % alloc16.blockSize == 0) // �������� �� ��������� � �������� � �� ������������ �� �������� �����
			{
				// �������� ������ � ������ ������
				auto currentBlockindex = ((char*)curPage->headFL - (char*)curPage) / blockSize - 1;	// ������ ������� �������� ���������� ��������
				curPage->headFL = (BlockHeader*)((char*)p - sizeof(int));
				curPage->headFL->nextFreeBlockIndex = currentBlockindex;
				break;
			}
			else
			{
				curPage = curPage->nextPage;
			}
		}
		//auto currentBlockindex = ((char*)mainPage->headFL - (char*)mainPage) / blockSize - 1; // ������ ������� �������� ���������� ��������
		//mainPage->headFL = (BlockHeader*)((char*)p - sizeof(int));
		//mainPage->headFL->nextFreeBlockIndex = currentBlockindex;
		
		//auto currentBlockindex = ((char*)curPage->headFL - (char*)curPage) / blockSize - 1;	// ������ ������� �������� ���������� ��������
		//curPage->headFL = (BlockHeader*)((char*)p - sizeof(int));
		//curPage->headFL->nextFreeBlockIndex = currentBlockindex;
	}

	PageHeader* AddNewPage()	//��������� ����� ��������
	{
		auto newPage = (PageHeader*)VirtualAlloc(
			NULL,
			PAGESIZE,
			MEM_COMMIT,	//�������������� ��������
			PAGE_READWRITE);
		isInit = true; // �������� ��������, ��������� ������������������

#ifdef _DEBUG
		assert(newPage != NULL);	// �������� �� ������������� ��������
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
			blocksCount++; // ������� ���������� ������ �����
		}
		temp->nextFreeBlockIndex = -1; // ��������� ���� �� ����� ����������
		blocksCount++;
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
};

class CoalesceAllocator
{
public:
	struct BlockHeader
	{
		BlockHeader* nextFreeBlock = nullptr;	//������ ���������� ���������� �����
		BlockHeader* prevFreeBlock = nullptr;	//������ ���������� ���������� �����
		BlockHeader* nextBlock = nullptr;	//������ ���������� ���������� �����
		BlockHeader* prevBlock = nullptr;	//������ ���������� ���������� �����
		size_t blockSize; //������ �������� �����
		bool isFree = true; // �������� �� ������ ����
	};

	struct PageHeader
	{
		BlockHeader* headFL = nullptr;	// ������ �� ������ ��������� ����
		PageHeader* nextPage = nullptr;	//������ �� ��������� ��������
	};

	size_t pageSize;
	PageHeader* mainPage = nullptr;
	bool isInit = false;
	int blocksCount = 0;

	CoalesceAllocator()
	{
		//pageSize = (size_t)(1024 * 1024 * 10) + sizeof(BlockHeader); //��������� �������� � �������
		pageSize = (size_t)(1024 * 1024 * 10); //��������� ��������
	}

	~CoalesceAllocator()
	{
		if (isInit == true) //���� ��������� �� ��� ����������������� �������
		{
			DestoyCA();
		}
	}

	void InitCA()
	{
		mainPage = AddNewPage();
		mainPage->headFL = (BlockHeader*)((char*)mainPage + sizeof(PageHeader));	// �������� ��������� �� ������ ��������� ���� (����� �� �������� ��������� ��������)	
		mainPage->headFL->blockSize = pageSize - sizeof(PageHeader); // ������ ������� ����� � ������ ���������� ����� � ��� ����� ��������� ��������
		mainPage->headFL->isFree = true;
	}

	PageHeader* AddNewPage()
	{
		auto newPage = (PageHeader*)VirtualAlloc(
			NULL,
			pageSize,
			MEM_COMMIT,	//�������������� ��������
			PAGE_READWRITE);
		isInit = true; // �������� ��������, ��������� ������������������

#ifdef _DEBUG
		assert(newPage != NULL);	// �������� �� ������������� ��������
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
			assert(isReleased);  // �������� �� ������������
#endif // DEBUG		

			tempPage = tempNextPage;
		}

		if (tempPage != nullptr)
		{
			isReleased = VirtualFree((LPVOID)tempPage, 0, MEM_RELEASE);

#ifdef _DEBUG
			assert(isReleased);  // �������� �� ������������
#endif // DEBUG	
		
		}

		isInit = false; //��������� �����������������
	}

	void* FindFreeBlock(PageHeader* page, size_t dataSize) // ��������� ���������� ����� � ��������� ������ ����� �������
	{
		void* returnedFreeBlock = nullptr;
		auto currentFreeBlock = page->headFL;
		while (currentFreeBlock != nullptr) // ���� ���� �� ��� ���, ���� �� ��������� ����� ���-�����
		{
			auto test1 = currentFreeBlock->blockSize;
			auto test2 = dataSize + sizeof(BlockHeader);
			if (currentFreeBlock->blockSize > dataSize + sizeof(BlockHeader)) //���� ������ ���������� � ���� ������ c ������� ��� ���������
			{
				//freeBlock = (char*)currentFreeBlock;	//����� ������������� ������� �����
				auto newBlock = (BlockHeader*)((char*)currentFreeBlock + sizeof(BlockHeader) + dataSize); // �������� ����� ���� (����� �������)
				newBlock->nextBlock = currentFreeBlock->nextBlock; // ��� ������ ����� ��������� ���������� ��������� ��� ��������
				newBlock->prevBlock = currentFreeBlock; //���������� ���������� ���� ��� ����������		
				newBlock->nextFreeBlock = currentFreeBlock->nextFreeBlock; // ��� ������ ����� ���������� ��������� ��������� ����
				newBlock->prevFreeBlock = currentFreeBlock->prevFreeBlock;
				if (newBlock->nextFreeBlock != nullptr)
				{
					newBlock->nextFreeBlock->prevFreeBlock = newBlock;				
				}			
				currentFreeBlock->nextFreeBlock = nullptr; //������ ������ �����
				
				newBlock->blockSize = currentFreeBlock->blockSize - sizeof(BlockHeader) - dataSize ; // ��������� ������ ������ ������������� �����
				currentFreeBlock->blockSize = dataSize + sizeof(BlockHeader); // ���������� ����� ������ ������� ������
				
				currentFreeBlock->nextBlock = newBlock; //���������� ��������� ���� ��� ��������				
				if (currentFreeBlock->prevFreeBlock != nullptr)
				{
					currentFreeBlock->prevFreeBlock->nextFreeBlock = newBlock; //��������� ���������� ����� ��� ����������� ���������� �����					
				}						
				currentFreeBlock->prevFreeBlock = nullptr; //������ ������ �����
				
				//prevBlock ��� �������� ����� �������� ����������
				
				currentFreeBlock->isFree = false; // ���� ���������� ������� (����������������)
				newBlock->isFree = true; //����� ���� ��������

				///////////////////////
				if (newBlock->prevFreeBlock == nullptr) // ���� ����� ������� ��������� � ������/
				{
					page->headFL = newBlock; //�������������� ������ � �����, �������������� � ���� ���������� ����
				}

				returnedFreeBlock = (char*)currentFreeBlock + sizeof(BlockHeader);//����������� ���� ��� ��������� ����
				break;
			}
			else // ���� ������ �� ���������� � ����
			{
				currentFreeBlock = currentFreeBlock->nextFreeBlock; //��������� � ���������� ����� � ������
			}
		}
		return returnedFreeBlock;
		//void* freeBlock = nullptr;
		//auto currentFreeBlock = page->headFL;
		//while (currentFreeBlock != nullptr) // ���� ���� �� ��� ���, ���� �� ��������� ����� ���-�����
		//{
		//	if (currentFreeBlock->blockSize - sizeof(BlockHeader) > dataSize + sizeof(BlockHeader)) //���� ������ ���������� � ���� ������ c ������� ��� ���������
		//	{
		//		freeBlock = (char*)currentFreeBlock + sizeof(BlockHeader);	//����� ������������� ������� ����� (����� ����� � �������)
		//		auto newBlock = (BlockHeader*)((char*)freeBlock + dataSize); // �������� ����� ���� (����� �������)
		//		newBlock->nextBlock = currentFreeBlock->nextBlock; // ��� ������ ����� ��������� ���������� ��������� ��� ��������
		//		newBlock->nextFreeBlock = currentFreeBlock->nextFreeBlock; // ��� ������ ����� ���������� ��������� ��������� ����
		//		newBlock->prevFreeBlock = currentFreeBlock->prevFreeBlock;
		//		newBlock->nextFreeBlock->prevFreeBlock = newBlock;
		//		newBlock->blockSize = currentFreeBlock->blockSize - dataSize;
		//		currentFreeBlock->nextBlock = newBlock; //���������� ��������� ���� ��� ��������
		//		currentFreeBlock->isFree = false; // ���� ���������� ������� (����������������)
		//		currentFreeBlock->prevFreeBlock->nextFreeBlock = newBlock; //��������� ���������� ����� ��� ����������� ���������� �����
		//		currentFreeBlock->prevFreeBlock = nullptr;
		//		currentFreeBlock->nextFreeBlock = nullptr;
		//		
		//		currentFreeBlock->blockSize = dataSize + sizeof(BlockHeader); // ���������� ����� ������ ������� ������
		//		//prev ��� �������� ����� �������� ����������
		//		newBlock->prevBlock = currentFreeBlock; //���������� ���������� ���� ��� ����������		
		//		newBlock->isFree = true; //����� ���� ��������
		//		
		//		if (newBlock->nextFreeBlock != nullptr)
		//		{
		//			page->headFL = newBlock; //�������������� ������ � �����, �������������� � ���� ���������� ����
		//		}
		//		
		//		
		//	}
		//	else // ���� ������ �� ���������� � ����
		//	{
		//		currentFreeBlock = currentFreeBlock->nextFreeBlock; //��������� � ���������� ����� � ������
		//	}
		//}		
		//return freeBlock;
	}

	void* GetFreeBlock(size_t dataSize) //dataSize - ����� �������� ������, ������������ � �����
	{
		auto currentPage = mainPage; // ���������� ������� �������� ������� � �������	

		//while (currentPage->headFL == nullptr) //���� �������� �� ���������� �������
		//{
		//	currentPage = currentPage->nextPage; // ���� � ���� �������� ��� ��������� ������, ��������� �� ���������
		//}
		void* currentFreeBlock;
		while (true)
		{	
			currentFreeBlock = FindFreeBlock(currentPage, dataSize);//����� ���������� ����� � ��������
			if (currentFreeBlock == nullptr) // ��������� ���� �� ������
			{
				if (currentPage->nextPage != nullptr) //���� ���� ��������� ��������
				{
					currentPage = currentPage->nextPage; //�� ��������� � ��� � ���� ���
				}
				else // ���� ��������� �������� ���, �� ������� ����� ��������
				{
					auto newPage = AddNewPage(); //��������� ����� ��������
					newPage->headFL = (BlockHeader*)((char*)mainPage + sizeof(PageHeader)); //��������� ������ ������ � ����� ��������
					newPage->headFL->blockSize = pageSize - sizeof(PageHeader);
					currentPage->nextPage = newPage; //���������� ����� �������� � next ��� ������� ��������	
					currentPage = newPage; // ��������� � ����� �������� � ���� ���
				}
			}
			else
			{
				break;
			}
				
		}

		return currentFreeBlock;

		//if (currentPage->headFL->blockSize - sizeof(BlockHeader) > dataSize + sizeof(BlockHeader)) //���� ������ ���������� � ���� ������ c ������� ��� ���������
		//{
		//	currentFreeBlock = (char*)currentPage->headFL + sizeof(BlockHeader);	//����� �������� ������� ����� (����� � ������� � �����)
		//	auto tmp = (BlockHeader*)((char*)currentFreeBlock + dataSize); // �������� ����� ���� (����� �������)
		//	currentPage->headFL->nextBlock = tmp; //���������� ��������� ���� ��� ��������
		//	currentPage->headFL = tmp; //�������������� ������		
		//	
		//	//if (currentPage->headFL->nextFreeBlock == nullptr) // ���� � �������� �������� ��� ���������� ����������
		//	//{
		//	//	currentPage->headFL = tmp; //�������������� ������		
		//	//}
		//	//else
		//	//{
		//	//	currentPage->headFL = currentPage->headFL->nextFreeBlock;
		//	//}		
		//}
		//else if(currentPage->headFL->nextFreeBlock != nullptr)// ���� ������ �� ���������� � ����
		//{
		//	currentPage->headFL = currentPage->headFL->nextFreeBlock;
		//}

		//
		//if (currentNextIndex > 0)
		//{
		//	currentPage->headFL = (BlockHeader*)((char*)currentPage + (currentNextIndex * blockSize) + blockSize);// ������� �� ��������� ��������� ���� �� ��� ������� (��������� ��� ����� ������� ���� ��.)		
		//}
		//else
		//{
		//	auto newPage = AddNewPage();											//��������� ����� ��������
		//	newPage->headFL = (BlockHeader*)((char*)newPage + blockSize);			//��������� ������ ������ � ����� ��������
		//	BlockHeadersInit(newPage);												//�������������� ����� � ����� ��������
		//	currentPage->nextPage = newPage;										//���������� ����� �������� � next ��� ������� ��������		
		//	currentPage->headFL = nullptr;
		//}
		//return currentFreeBlock;
	}

	void SetFreeBlock(PageHeader* page, void *p) //page ��� �������� � ������� ��� ������ ���������
	{
		auto currentPage = page;

		BlockHeader* currentBlock = (BlockHeader*)((char*)p - sizeof(BlockHeader)); // ��������� � ���������� �����
		if (currentBlock->prevBlock !=nullptr && currentBlock->prevBlock->isFree)//���� ������� ����� �����
		{
			auto leftBlock = currentBlock->prevBlock; //����� �����
			leftBlock->nextBlock = currentBlock->nextBlock;//
			if (currentBlock->nextBlock != nullptr)
				currentBlock->nextBlock->prevBlock = leftBlock;//
			leftBlock->blockSize += currentBlock->blockSize;
			currentBlock = leftBlock;
		}

		if (currentBlock->nextBlock != nullptr && currentBlock->nextBlock->isFree)
		{
			auto rightBlock = currentBlock->nextBlock; //����� ������
			currentBlock->nextBlock = rightBlock->nextBlock; //��������� ���� ��� ��������
			if (rightBlock->nextBlock != nullptr)
				rightBlock->nextBlock->prevBlock = currentBlock; //���������� ���� ��� ����������
			currentBlock->blockSize += rightBlock->blockSize;

			currentBlock->nextFreeBlock = rightBlock->nextFreeBlock;
			currentBlock->prevFreeBlock = rightBlock->prevFreeBlock;
			if (currentPage->headFL == rightBlock)
			{
				currentPage->headFL = currentBlock;
			}
		}

		if (currentBlock->nextBlock != nullptr && currentBlock->prevBlock != nullptr && currentBlock->nextBlock->isFree == false && currentBlock->prevBlock->isFree == false 
			|| currentBlock->nextBlock == nullptr && currentBlock->prevBlock != nullptr && currentBlock->prevBlock->isFree == false
			|| currentBlock->nextBlock != nullptr && currentBlock->prevBlock == nullptr && currentBlock->nextBlock->isFree == false)
		{
			if (currentPage->headFL != nullptr)
				currentPage->headFL->prevFreeBlock = currentBlock;
			currentBlock->nextFreeBlock = currentPage->headFL;
			currentBlock->prevFreeBlock = nullptr;
			currentPage->headFL = currentBlock;
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

	PageHeader* IsAllocatorContainPointer(void* p)
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

	LPVOID OS_mainPage = NULL; //�������� ����������� ��������������� � ��
	LPVOID OS_nextPage = NULL;

	MemoryAllocator() : alloc16(16 * sizeof(char)),			//������� �������������������� ���������
						alloc32(32 * sizeof(char)),
						alloc64(64 * sizeof(char)), 
						alloc128(128 * sizeof(char)), 
						alloc256(256 * sizeof(char)), 
						alloc512(512 * sizeof(char)),
						coalAlloc()
	{
		isInit = false;	// ��������� �� ���������������
	}


	virtual ~MemoryAllocator() 
	{
		if (isInit == true) //���� ������� �� ��� ������
		{
			Destroy();
		}

		//OS_mainPage = VirtualAlloc(
		//	OS_mainPage,
		//	PAGESIZE,
		//	MEM_RESERVE,			//����������� ������
		//	PAGE_READWRITE);
	}

	virtual void Init()			//��������� ������������� ����������, ���������� ����������� �������� ������ � ��
	{
		// ������������� ���� �����������
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
		//	MEM_COMMIT,			//�������������� �������� � ����� ����������������� �������
		//	PAGE_READWRITE);

		isInit = true; //��������� ������������������
	}

	virtual void Destroy()
	{
		//������� �������� ��� ���� �����������
		if (isInit == true) // ���� ���������� ���� �������������������� 
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
			isInit = false; //��������� �����������������
		}		
	}

	virtual void* Alloc(size_t size)
	{
		if (isInit == false) // ���� ���������� �� ���� �������������������� 
		{
			assert(isInit);
			//return nullptr;
		}

		size_t FSA_metaDataSize = 1 * sizeof(int); // ������ ���������� � ������ ����� ��� FSA ����������
		size_t CA_metaDataSize = 24; // ������ ���������� � ����� ��� CA ����������

		//����� � ����������� �� ����������
		if (size <= 16 - FSA_metaDataSize) // ������ 12 ���� � ������ ��������� ������ � �����
		{
			return alloc16.GetFreeBlock();
		}		
		else if (size > 16 - FSA_metaDataSize && size <= 32 - FSA_metaDataSize) //�� 12 �� 28 ����
		{
			return alloc32.GetFreeBlock();
		}	
		else if (size > 32 - FSA_metaDataSize && size <= 64 - FSA_metaDataSize) //�� 28 �� 60 ����
		{
			return alloc64.GetFreeBlock();
		}		
		else if (size > 64 - FSA_metaDataSize && size <= 128 - FSA_metaDataSize) //�� 60 �� 124 ����
		{
			return alloc128.GetFreeBlock();
		}	
		else if (size > 128 - FSA_metaDataSize && size <= 256 - FSA_metaDataSize) //�� 124 �� 254 ����
		{
			return alloc256.GetFreeBlock();
		}
		else if (size > 256 - FSA_metaDataSize && size <= 512 - FSA_metaDataSize) //�� 254 �� 508 ����
		{
			return alloc512.GetFreeBlock();
		}
		else if (size > 512 - FSA_metaDataSize && size <= (1024 * 1024 * 10) - CA_metaDataSize) //�� 506 ���� �� 10 �����
		{
			return coalAlloc.GetFreeBlock(size); //������� ������ ��������� ������ �����������
		}
		else if (size > (1024 * 1024 * 10) - CA_metaDataSize) //������ 10 �����
		{
			OS_mainPage = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);

#ifdef _DEBUG
			assert(OS_mainPage != NULL);	// �������� �� ������������� ��������
#endif // DEBUG	

			return OS_mainPage;
		}
	}

	virtual void Free(void *p)
	{
		if (isInit == false) // ���� ���������� �� ���� �������������������� 
		{
			return;
		}
		
		//����� � ����������� �� ����������
		if (alloc16.IsAllocatorContainPointer(p))
		{
			alloc16.SetFreeBlock(p);
		}
		else if (alloc32.IsAllocatorContainPointer(p))
		{
			alloc32.SetFreeBlock(p);
		}
		else if (alloc64.IsAllocatorContainPointer(p))
		{
			alloc64.SetFreeBlock(p);
		}
		else if (alloc128.IsAllocatorContainPointer(p))
		{
			alloc128.SetFreeBlock(p);
		}
		else if (alloc256.IsAllocatorContainPointer(p))
		{
			alloc256.SetFreeBlock(p);
		}
		else if (alloc512.IsAllocatorContainPointer(p))
		{
			alloc512.SetFreeBlock(p);
		}
		else 
		{
			auto page = coalAlloc.IsAllocatorContainPointer(p);
			if (page != nullptr)
			{
				coalAlloc.SetFreeBlock(page, p);
				return;
			}			
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
//			assert(isReleased);  // �������� �� ������������
//#endif // DEBUG	
//		}
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

