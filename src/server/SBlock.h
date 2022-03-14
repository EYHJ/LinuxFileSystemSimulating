#ifndef SBLOCK_H
#define SBLOCK_H
#include "BasDef.h"

namespace FS {
	class SBlock {
	public:
		bool setFreeBlockCntUp();
		bool setFreeBlockCntDown();
		bool setFreeInodeCntUp();
		bool setFreeInodeCntDown();

		ull testNum;
		ull inodeBitsetPos;
		ull blockBitsetPos;
		tInodeID freeInodePointer;
		tBlockID freeBlockPointer;
		ull freeInodeCnt;
		ull freeBlockCnt;
		ull inodeSize;
		ull blockSize;
		ull firstInodePos;
		ull firstBlockPos;

		tInodeID rootInode;
		tInodeID userInode;

		static constexpr ull TEST_NUM = 0xAAAAAAAAAAAAAAAA;
	};
}

#endif

