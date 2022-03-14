#include "BasicFileIO.h"

void FS::BasicFileIO::getBlockCstr(tBlockID _ID, char * _dest) {
	size_t tpos = superBlock.firstBlockPos + _ID * BLOCKSIZE;
	ioFile.seekg(tpos);
	ioFile.read(_dest, BLOCKSIZE);
}

void FS::BasicFileIO::writeBlock(tBlockID _ID, char * _blockStr, size_t cnt) {
	size_t tpos = superBlock.firstBlockPos + _ID * BLOCKSIZE;
	ioFile.seekp(tpos);
	ioFile.write(_blockStr, cnt);
}

//return BAD_BLOCK_ID stand for unused
FS::tBlockID FS::BasicFileIO::getAvlBlockID() {
	tBlockID posNow = superBlock.freeBlockPointer;
	while(blockBitset.test(superBlock.freeBlockPointer)) {
		++superBlock.freeBlockPointer;
		if(superBlock.freeBlockPointer == BLOCK_MAX_SIZE)
			superBlock.freeBlockPointer = 0;
		if(superBlock.freeBlockPointer == posNow)
			throw std::string("run out of block");
	}
	return superBlock.freeBlockPointer;
}

//return 0 stand for unused
FS::tInodeID FS::BasicFileIO::getAvlInodeID() {
	tInodeID posNow = superBlock.freeInodePointer;
	while(inodeBitset.test(superBlock.freeInodePointer)) {
		++superBlock.freeInodePointer;
		if(superBlock.freeInodePointer == INODE_MAX_SIZE)
			superBlock.freeInodePointer = 0;
		if(superBlock.freeInodePointer == posNow)
			throw std::string("run out of inode");
	}
	return superBlock.freeInodePointer;
}

FS::Inode & FS::BasicFileIO::getInode(tInodeID _ID) {
	return std::ref(inodeList[_ID]);
}

void FS::BasicFileIO::saveSuperBlock() {
	ioFile.seekp(std::ios::beg);
	ioFile.write((char*)&superBlock, sizeof(superBlock));
}

void FS::BasicFileIO::saveInodeBitset() {
	ioFile.seekp(superBlock.inodeBitsetPos);
	ioFile.write((char *)&inodeBitset, sizeof(inodeBitset));
}

void FS::BasicFileIO::saveBlockBitset() {
	ioFile.seekp(superBlock.blockBitsetPos);
	ioFile.write((char *)&blockBitset, sizeof(blockBitset));
}

void FS::BasicFileIO::saveInodeList() {
	ioFile.seekp(superBlock.firstInodePos);
	ioFile.write((char *)&inodeList, sizeof(inodeList));
}

void FS::BasicFileIO::save() {
	saveSuperBlock();
	saveInodeBitset();
	saveBlockBitset();
	saveInodeList();
}

FS::BasicFileIO::BasicFileIO(const char* const _discName):
	ioFile(_discName, std::ios::in | std::ios::out | std::ios::binary) {
	if(!ioFile.is_open())
		throw std::string("can not find virtual disc file");

	ioFile.read((char*)&superBlock, sizeof(superBlock));
	if(superBlock.testNum != SBlock::TEST_NUM) {
		formatting();
		return;
	}

	ioFile.seekg(superBlock.inodeBitsetPos);
	ioFile.read((char*)&inodeBitset, sizeof(inodeBitset));
	ioFile.seekg(superBlock.blockBitsetPos);
	ioFile.read((char*)&blockBitset, sizeof(blockBitset));
	ioFile.seekg(superBlock.firstInodePos);
	ioFile.read((char*)&inodeList, sizeof(inodeList));

}

FS::BasicFileIO::~BasicFileIO() {
	save();
	ioFile.close();
}

void FS::BasicFileIO::formatting() {
	superBlock.testNum = SBlock::TEST_NUM;
	superBlock.inodeBitsetPos = FS::BLOCKSIZE;
	superBlock.blockBitsetPos = superBlock.inodeBitsetPos + sizeof(inodeBitset);
	superBlock.freeInodePointer = 1;
	inodeBitset.set(0, true);
	superBlock.freeBlockPointer = 1;
	superBlock.freeInodeCnt = INODE_MAX_SIZE;
	superBlock.freeBlockCnt = BLOCK_MAX_SIZE;
	superBlock.inodeSize = INODE_MAX_SIZE;
	superBlock.blockSize = BLOCK_MAX_SIZE;
	superBlock.firstInodePos = superBlock.blockBitsetPos + sizeof(blockBitset);
	superBlock.firstBlockPos = superBlock.firstInodePos + sizeof(inodeList);

	superBlock.rootInode = INODE_MAX_SIZE;
	superBlock.userInode = 0;

	save();
}
