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
		//���� ������� �� ��� ������, �� ��������� �������
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
//			assert(isReleased);  // �������� �� ������������
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
//			assert(isReleased);  // �������� �� ������������
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

		mainPage->headFL = (BlockHeader*)((char*)mainPage + blockSize);	// �������� ��������� �� ������ ��������� ���� ��� ����� ������� ����. ��.	

		//mainPage->headFL->currentBlockIndex = 0; //������ �������� �����
		//mainPage->headFL->nextFreeBlockIndex++; //������ ���������� ���������� �����

		BlockHeadersInit(mainPage);	// ������������� ���������� ������ 

		//auto temp = mainPage->headFL;
		//int i = 1;
		//while ((char*)temp < (char*)mainPage + PAGESIZE - blockSize) 
		//{
		//	temp->nextFreeBlockIndex = i;
		//	i++;
		//	temp = (BlockHeader*)((char*)temp + blockSize);
		//}
		//temp->nextFreeBlockIndex = -1; // ��������� ���� �� ����� ����������
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

			isInit = false; //��������� �����������������
		}
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

	MemoryAllocator() : alloc16(16 * sizeof(char)),			//������� �������������������� ���������
						alloc32(32 * sizeof(char))
						//alloc64(64 * sizeof(char)), 
						//alloc128(128 * sizeof(char)), 
						//alloc256(256 * sizeof(char)), 
						//alloc512(512 * sizeof(char))*/		
	{
		isInit = false;
		//������� �������������������� ���������
	}


	virtual ~MemoryAllocator() 
	{
		//destroy
		//�������� �� �������, ���� ������� �� ��� ������ ������� �������� (�������� � ����� ����������, � ��� �� ����)
	}

	virtual void Init()			//��������� ������������� ����������, ���������� ����������� �������� ������ � ��
	{
		isInit = true;
		// ������������� ���� �����������
		alloc16.InitFSA();
		alloc32.InitFSA();
		//alloc64.InitFSA();
		//alloc128.InitFSA();
		//alloc256.InitFSA();
		//alloc512.InitFSA();

		//VirtualAlloc(
		//	page,
		//	PAGESIZE,
		//	MEM_COMMIT,			//�������������� �������� � ����� ����������������� �������
		//	PAGE_READWRITE);
	}

	virtual void Destroy()
	{
		//������� �������� ��� ���� �����������
		if (isInit == true) // ���� ���������� ���� �������������������� 
		{
			alloc16.DestoyFSA();
			// destroy all
			isInit == false;
		}		
	}

	virtual void* Alloc(size_t size)
	{
		//����� � ����������� �� ����������
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
		//		//&& ((char*)p - (char*)curPage) % alloc16.blockSize == 0) // �������� �� ��������� � �������� � �� ������������ �� �������� �����
		//	{
		//		//alloc16.SetFreeBlock(curPage, p); // �������� ������ � ������ ������
		//		alloc16.SetFreeBlock(p); // �������� ������ � ������ ������
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

		//����� � ����������� �� ����������
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

