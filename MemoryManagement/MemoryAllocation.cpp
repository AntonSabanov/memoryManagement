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
		int nextFreeBlockIndex;
		//int currentBlockIndex;
	};

	struct PageHeader
	{
		BlockHeader* headFL = nullptr;		// ������ �� ������ ��������� ����
		PageHeader* nextPage = nullptr;	//������ �� ��������� ��������
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
			assert(isReleased);  // �������� �� ������������
#endif // DEBUG		
			tempPage = tempNextPage;
		}
		isReleased = VirtualFree(
			(LPVOID)tempPage,
			0,
			MEM_RELEASE);
#ifdef _DEBUG
		assert(isReleased);  // �������� �� ������������
#endif // DEBUG	
	}

	void* GetFreeBlock()
	{
		// ���� ���� head, ������ ���� ��������� ����, �� ���������� ������ �� ����
		// ���� ��� ��� � ������� ��������, �� ������� ����� ��������
		// ���� ��������� �������� ��� �������, �� ��������� � ��� � ���� ���
		
		//auto 
	/*	while (currentPage->headFL->nextFreeBlockIndex < 0)
		{

		}*/
			

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
		}

		//currentPage->headFL->nextFreeBlockIndex = ++currentNextIndex;
		//auto index2 = ((char*)currentPage->headFL - (char*)currentPage) / blockSize - 1; // ������ ������� �������� ���������� ��������

		return currentFreeBlock;
	}

	void SetFreeBlock(PageHeader* curPage, void *p)	// � ������� ���������� ������� ��������, � ������� ��������� ��������� � ��� ���������
	{
		//auto currentBlockindex = ((char*)currentPage->headFL - (char*)currentPage) / blockSize - 1; // ������ ������� �������� ���������� ��������
		//currentPage->headFL = (BlockHeader*)((char*)p - sizeof(int));
		//currentPage->headFL->nextFreeBlockIndex = currentBlockindex;
		auto currentBlockindex = ((char*)curPage->headFL - (char*)curPage) / blockSize - 1;	// ������ ������� �������� ���������� ��������
		curPage->headFL = (BlockHeader*)((char*)p - sizeof(int));
		curPage->headFL->nextFreeBlockIndex = currentBlockindex;
	}

	PageHeader* AddNewPage()	//��������� ����� ��������
	{
		auto newPage = (PageHeader*)VirtualAlloc(
			NULL,
			PAGESIZE,
			MEM_COMMIT,	//�������������� ��������
			PAGE_READWRITE);
#ifdef _DEBUG
		assert(newPage != NULL);	// �������� �� ������������� ��������
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
			
		currentPage->headFL = (BlockHeader*)((char*)currentPage + blockSize);	// �������� ��������� �� ������ ��������� ���� ��� ����� ������� ����. ��.	
		
		//currentPage->headFL->currentBlockIndex = 0; //������ �������� �����
		//currentPage->headFL->nextFreeBlockIndex++; //������ ���������� ���������� �����
			
		BlockHeadersInit(currentPage);	// ������������� ���������� ������ 

		//auto temp = currentPage->headFL;
		//int i = 1;
		//while ((char*)temp < (char*)currentPage + PAGESIZE - blockSize) 
		//{
		//	temp->nextFreeBlockIndex = i;
		//	i++;
		//	temp = (BlockHeader*)((char*)temp + blockSize);
		//}
		//temp->nextFreeBlockIndex = -1; // ��������� ���� �� ����� ����������
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

	MemoryAllocator() : alloc16(16 * sizeof(char))			//������� �������������������� ���������
		/*				alloc32(32 * sizeof(char)),
						alloc64(64 * sizeof(char)), 
						alloc128(128 * sizeof(char)), 
						alloc256(256 * sizeof(char)), 
						alloc512(512 * sizeof(char))*/		
	{
		//������� �������������������� ���������
	}


	virtual ~MemoryAllocator() 
	{
		
	}

	virtual void Init()			//��������� ������������� ����������, ���������� ����������� �������� ������ � ��
	{
		// ������������� ���� �����������
		alloc16.InitFSA();
		//alloc32.InitFSA();
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
				//&& ((char*)p - (char*)curPage) % alloc16.blockSize == 0) // �������� �� ��������� � �������� � �� ������������ �� �������� �����
			{
				alloc16.SetFreeBlock(curPage, p); // �������� ������ � ������ ������
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

