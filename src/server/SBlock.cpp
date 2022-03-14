#include "SBlock.h"

bool FS::SBlock::setFreeBlockCntUp() {
	if(freeBlockCnt == blockSize)
		return false;
	freeBlockCnt++;
	return true;
}

bool FS::SBlock::setFreeBlockCntDown() {
	if(freeBlockCnt == 0)
		return false;
	freeBlockCnt--;
	return true;

}

bool FS::SBlock::setFreeInodeCntUp() {
	if(freeInodeCnt == inodeSize)
		return false;
	freeInodeCnt++;
	return true;
}

bool FS::SBlock::setFreeInodeCntDown() {
	if(freeInodeCnt == 0)
		return false;
	freeInodeCnt--;
	return true;
}