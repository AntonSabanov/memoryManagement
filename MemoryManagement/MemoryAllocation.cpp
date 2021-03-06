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
		if (isInit == true) //���� ��������� �� ��� ����������������� �������
		{
			DestoyFSA();
		}
	}

	void InitFSA()
	{
		mainPage = AddNewPage(); // ��������� �������� � ������������� ����������
		mainPage->headFL = (BlockHeader*)((char*)mainPage + blockSize);	// �������� ��������� �� ������ ��������� ���� ��� ����� ������� ����. ��.
		BlockHeadersInit(mainPage);	// ������������� ���������� ������ � ���������� ���� ������
	}

	void DestoyFSA() //����� �������� ��� ����������
	{
		PageHeader* tempPage = mainPage;
		bool isReleased = false;
		while (tempPage->nextPage != nullptr)
		{
			PageHeader* tempNextPage = tempPage->nextPage;
			isReleased = VirtualFree((LPVOID)tempPage, 0, MEM_RELEASE);	

#ifdef _DEBUG
			assert(isReleased);  // �������� �� ������������
#endif//DEBUG		

			tempPage = tempNextPage;
		}

		if (tempPage != nullptr)
		{
			isReleased = VirtualFree((LPVOID)tempPage, 0, MEM_RELEASE);
#ifdef _DEBUG
			assert(isReleased);  // �������� �� ������������
#endif//DEBUG	
		}

#ifdef _DEBUG
		blocksCount = 0;
		freeBlocksCount = 0;
#endif//DEBUG

		isInit = false; //��������� �����������������
	}

	void* GetFreeBlock()
	{	
		auto currentPage = mainPage; // ���������� ������� �������� ������� � �������	

		while (currentPage->headFL == nullptr) //���� �������� �� ����������� �������
		{
			currentPage = currentPage->nextPage; // ���� � ���� �������� ��� ��������� ������, ��������� �� ���������
		}

		auto currentFreeBlock = (char*)currentPage->headFL + sizeof(int);			//����� �������� ������� ����� � ������ ������� ����������� (����� � ������� � �����)
		int currentNextIndex = currentPage->headFL->nextFreeBlockIndex;				//������ ���������� ����� ����� ��������
		
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

#ifdef _DEBUG
		freeBlocksCount--; //���������� ���������� ��������� ������
#endif//DEBUG

		return currentFreeBlock;
	}

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
#ifdef _DEBUG
				freeBlocksCount++; // ���������� ���������� ��������� ������
#endif//DEBUG
				break;
			}
			else
			{
				curPage = curPage->nextPage;
			}
		}
	}

	PageHeader* AddNewPage()	//��������� ����� ��������
	{
		auto newPage = (PageHeader*)VirtualAlloc(NULL, PAGESIZE, MEM_COMMIT, PAGE_READWRITE); //�������������� �������� 
		isInit = true; // �������� ��������, ��������� ������������������

#ifdef _DEBUG
		assert(newPage != NULL);	// �������� �� ������������� ��������
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
		}
		temp->nextFreeBlockIndex = -1; // ��������� ���� �� ����� ����������

#ifdef _DEBUG
		blocksCount += i; // ���������� ������ � ���������� �����
		freeBlocksCount += i; // ���������� ��������� ������ � ���������� �����
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

	void PrintAllBusyBlocks() const // ����� ������� ������
	{
		std::cout << "----------------------------------- \n";
		std::cout << "FSAx" << blockSize << " busy blocks: \n";
		auto currentPage = mainPage;
		auto currentBlock = (BlockHeader*)((char*)currentPage + blockSize);
		while (currentPage != nullptr)
		{
			while (currentBlock->nextFreeBlockIndex != -1)
			{
				auto head = currentPage->headFL;
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
				currentBlock = (BlockHeader*)((char*)currentBlock + blockSize);
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

#ifdef _DEBUG
	int blocksCount;
	int freeBlocksCount;
#endif//DEBUG

	CoalesceAllocator()
	{
		pageSize = (size_t)(1024 * 1024 * 10); //������ ���������� �������

#ifdef _DEBUG
		blocksCount = 0;
		freeBlocksCount = 0;
#endif//DEBUG
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

#ifdef _DEBUG
		blocksCount = 0;
		freeBlocksCount = 0;
#endif//DEBUG

		isInit = false; //��������� �����������������
	}

	void* FindFreeBlock(PageHeader* page, size_t dataSize) // ��������� ���������� ����� � ��������� ������ ����� �������
	{
		void* returnedFreeBlock = nullptr;
		auto currentFreeBlock = page->headFL;
		while (currentFreeBlock != nullptr) // ���� ���� �� ��� ���, ���� �� ��������� ����� ���-�����
		{
			if (currentFreeBlock->blockSize > dataSize + sizeof(BlockHeader)) //���� ������ ���������� � ���� ������ c ������� ��� ���������
			{
				//���������� �����
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

				if (newBlock->prevFreeBlock == nullptr) // ���� ����� ������� ��������� � ������/
				{
					page->headFL = newBlock; //�������������� ������ � �����, �������������� � ���� ���������� ����
				}

				returnedFreeBlock = (char*)currentFreeBlock + sizeof(BlockHeader);//����������� ���� ��� ��������� ����

#ifdef _DEBUG
				blocksCount++; // ��������� ����� ����
#endif//DEBUG
				
				break;
			}
			else // ���� ������ �� ���������� � ����
			{
				currentFreeBlock = currentFreeBlock->nextFreeBlock; //��������� � ���������� ����� � ������
			}
		}
		return returnedFreeBlock; 
	}

	void* GetFreeBlock(size_t dataSize) //dataSize - ����� �������� ������, ������������ � �����
	{
		auto currentPage = mainPage; // ���������� ������� �������� ������� � �������	

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
					newPage->headFL = (BlockHeader*)((char*)newPage + sizeof(PageHeader)); //��������� ������ ������ � ����� ��������
					newPage->headFL->blockSize = pageSize - sizeof(PageHeader);
					currentPage->nextPage = newPage; //���������� ����� �������� � next ��� ������� ��������	
					currentPage = newPage; // ��������� � ����� �������� � ���� ���

#ifdef _DEBUG
					blocksCount++; //
					freeBlocksCount++; // ��������� ��������� ����
#endif//DEBUG
				}
			}
			else
			{
				break;
			}				
		}
		return currentFreeBlock;
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
			currentBlock->isFree = true;

#ifdef _DEBUG
			blocksCount--; // ���������� ���������� ������ �.�. ����� ������������
			if (currentBlock->nextBlock != nullptr && currentBlock->nextBlock->isFree == true)
				freeBlocksCount--;
#endif//DEBUG
		}

		if (currentBlock->nextBlock != nullptr && currentBlock->nextBlock->isFree) //���� ������� ����� ������
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
			currentBlock->isFree = true;

#ifdef _DEBUG
			blocksCount--; // ���������� ���������� ������ �.�. ����� ������������
#endif//DEBUG
		}

		// ���� ���� ��������� ����� ����� �������� �������, ���� ����� ����� � ������ ������� ����, ���� ������ ����� � ����� ������� ����
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
			freeBlocksCount++; // ���������� ���������� ��������� ������
#endif//DEBUG
		}		
	}

	PageHeader* IsAllocatorContainPointer(void* p) // ����������, ��������� �� ��������� � ������ ���������� � ���� ��, �� ��������� ��������, � ������� �� ���������
	{
		auto currentPage = mainPage;
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

	void PrintAllBusyBlocks() const // ����� ������� ������
	{
		std::cout << "----------------------------------- \n";
		std::cout << "CA busy blocks: \n";
		auto currentPage = mainPage;	
		while (currentPage != nullptr)
		{
			auto currentBlock = (BlockHeader*)((char*)currentPage + sizeof(PageHeader));
			while (currentBlock != nullptr)
			{
				if (currentBlock->isFree == false)
					std::cout << "block: " << currentBlock << "; size: " << currentBlock->blockSize << "\n";
				currentBlock = currentBlock->nextBlock;
			}
			currentPage = currentPage->nextPage;
		}		
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

	struct PageHeader
	{
		PageHeader* OS_nextPage = nullptr;//������ �� ��������� ��������
		size_t OS_blockSize; //������ �������� �����
	};
	PageHeader* OS_mainPage = nullptr; //�������� ����������� ��������������� � ��

#ifdef _DEBUG
	int OS_blockCount;
	AllocatorType currentAllocatorType = Uncalled;	// ����� ������ ���������� ��� ������
#endif//DEBUG	

	MemoryAllocator() : alloc16(16 * sizeof(char)),			//������� �������������������� ���������
						alloc32(32 * sizeof(char)),
						alloc64(64 * sizeof(char)), 
						alloc128(128 * sizeof(char)), 
						alloc256(256 * sizeof(char)), 
						alloc512(512 * sizeof(char)),
						coalAlloc()
	{
		isInit = false;	// ��������� �� ���������������

#ifdef _DEBUG
		OS_blockCount = 0;
#endif//DEBUG	
	}

	virtual ~MemoryAllocator() 
	{
		if (isInit == true) //���� ������� �� ��� ������
		{
			Destroy();
		}
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

#ifdef _DEBUG
			OS_blockCount = 0;
#endif//DEBUG

			isInit = false; //��������� �����������������
		}		
	}

	virtual void* Alloc(size_t size)
	{
		assert(isInit);

		size_t FSA_metaDataSize = 1 * sizeof(int); // ������ ���������� � ������ ����� ��� FSA ����������
		size_t CA_metaDataSize = sizeof(CoalesceAllocator::BlockHeader); // ������ ���������� � ����� ��� CA ����������

		//����� � ����������� �� ����������
		if (size <= 16 - FSA_metaDataSize) // ������ 12 ���� � ������ ��������� ������ � �����
		{
#ifdef _DEBUG
			currentAllocatorType = FSA16;
#endif//DEBUG			
			return alloc16.GetFreeBlock();
		}		
		else if (size > 16 - FSA_metaDataSize && size <= 32 - FSA_metaDataSize) //�� 12 �� 28 ����
		{
#ifdef _DEBUG
			currentAllocatorType = FSA32;
#endif//DEBUG				
			return alloc32.GetFreeBlock();
		}	
		else if (size > 32 - FSA_metaDataSize && size <= 64 - FSA_metaDataSize) //�� 28 �� 60 ����
		{
#ifdef _DEBUG
			currentAllocatorType = FSA64;
#endif//DEBUG			
			return alloc64.GetFreeBlock();
		}		
		else if (size > 64 - FSA_metaDataSize && size <= 128 - FSA_metaDataSize) //�� 60 �� 124 ����
		{
#ifdef _DEBUG
			currentAllocatorType = FSA128;
#endif // DEBUG	
			return alloc128.GetFreeBlock();
		}	
		else if (size > 128 - FSA_metaDataSize && size <= 256 - FSA_metaDataSize) //�� 124 �� 252 ����
		{
#ifdef _DEBUG
			currentAllocatorType = FSA256;
#endif//DEBUG	
			return alloc256.GetFreeBlock();
		}
		else if (size > 256 - FSA_metaDataSize && size <= 512 - FSA_metaDataSize) //�� 254 �� 508 ����
		{
#ifdef _DEBUG
			currentAllocatorType = FSA512;
#endif//DEBUG	
			return alloc512.GetFreeBlock();
		}
		else if (size > 512 - FSA_metaDataSize && size <= (1024 * 1024 * 10) - CA_metaDataSize) //�� 506 ���� �� 10 �����
		{
#ifdef _DEBUG
			currentAllocatorType = CA;
#endif//DEBUG	
			return coalAlloc.GetFreeBlock(size); //������� ������ ���������� ������ �����������
		}
		else if (size > (1024 * 1024 * 10) - CA_metaDataSize) //������ 10 �����
		{
			PageHeader* newPage = (PageHeader*)VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
			if (OS_mainPage == nullptr)
				OS_mainPage = newPage;
			PageHeader* currentPage = OS_mainPage;
			while (currentPage != nullptr)
			{
				currentPage = currentPage->OS_nextPage;
			}
			currentPage = newPage;
			currentPage->OS_blockSize = size;		

#ifdef _DEBUG
			OS_blockCount++;
			currentAllocatorType = OS;
			assert(newPage != nullptr);	// �������� �� ������������� ��������
#endif//DEBUG	

			return (char*)currentPage + sizeof(PageHeader);
		}
	}

	virtual void Free(void *p)
	{
		assert(isInit);
		
		//����� � ����������� �� ����������
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
		else
		{
			auto currentPage = OS_mainPage;
			PageHeader* nextBlock = nullptr;
			PageHeader* prevBlock = nullptr;
			while (currentPage != nullptr)
			{
				if ((char*)p >= (char*)currentPage && (char*)p < (char*)currentPage + currentPage->OS_blockSize) //���� �������� � ��������
				{
					if (currentPage->OS_nextPage != nullptr && prevBlock != nullptr) //���� ���� ����������, �� ��������� ����� ����� � ������ ����� �����
					{
						nextBlock = currentPage->OS_nextPage;
						prevBlock->OS_nextPage = nextBlock;
					}
					else if (currentPage->OS_nextPage != nullptr && prevBlock == nullptr) //���� ���� ����� ������
					{
						OS_mainPage = currentPage->OS_nextPage;
					}
					else if (prevBlock != nullptr) //���� ���� ����� �����
					{
						prevBlock->OS_nextPage = nullptr;
					}
					break;
				}
				else
				{
					prevBlock = currentPage;
					currentPage = currentPage->OS_nextPage;
				}
			}

			bool isReleased = VirtualFree((LPVOID)p, 0, MEM_RELEASE);

#ifdef _DEBUG
			OS_blockCount--;
			currentAllocatorType = OS;
			assert(isReleased);  // �������� �� ������������
#endif // DEBUG	
		}
	}

#ifdef _DEBUG
	AllocatorType GetCurrentAllocatorType()
	{
		return currentAllocatorType;
	}

	void PrintAllOSBusyBlocks() const // ����� ������� ������
	{
		std::cout << "----------------------------------- \n";
		std::cout << "OS busy blocks: \n";
		auto currentPage = OS_mainPage;
		while (currentPage != nullptr)
		{
			std::cout << "block: " << currentPage << "; size: " << currentPage->OS_blockSize << "\n";
			currentPage = currentPage->OS_nextPage;
		}
	}

	virtual void DumpStat() const
	{
		std::cout << "----------------------------------- \n";
		std::cout << "FSAx16 bloks count: " << alloc16.GetAllBlocksCount() << "; free bloks count: " << alloc16.GetFreeBlocksCount() << std::endl;
		std::cout << "FSAx32 bloks count: " << alloc32.GetAllBlocksCount() << "; free bloks count: " << alloc32.GetFreeBlocksCount() << std::endl;
		std::cout << "FSAx64 bloks count: " << alloc64.GetAllBlocksCount() << "; free bloks count: " << alloc64.GetFreeBlocksCount() << std::endl;
		std::cout << "FSAx128 bloks count: " << alloc128.GetAllBlocksCount() << "; free bloks count: " << alloc128.GetFreeBlocksCount() << std::endl;
		std::cout << "FSAx256 bloks count: " << alloc256.GetAllBlocksCount() << "; free bloks count: " << alloc256.GetFreeBlocksCount() << std::endl;
		std::cout << "FSAx512 bloks count: " << alloc512.GetAllBlocksCount() << "; free bloks count: " << alloc512.GetFreeBlocksCount() << std::endl;
		std::cout << "CA bloks count: " << coalAlloc.GetAllBlocksCount() << "; free bloks count: " << coalAlloc.GetFreeBlocksCount() << std::endl;
		std::cout << "OS bloks count: " << OS_blockCount << std::endl;
	}

	virtual void DumpBlocks() const
	{
		alloc16.PrintAllBusyBlocks();
		alloc32.PrintAllBusyBlocks();
		alloc64.PrintAllBusyBlocks();
		alloc128.PrintAllBusyBlocks();
		alloc256.PrintAllBusyBlocks();
		alloc512.PrintAllBusyBlocks();
		coalAlloc.PrintAllBusyBlocks();
		PrintAllOSBusyBlocks();
	}
#endif // DEBUG	
};