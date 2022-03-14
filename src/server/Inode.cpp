#include "Inode.h"

FS::Inode::Inode() {
	reset();
}

FS::Inode::Inode(const Inode & _Right) {
	memcpy(this, &_Right, CLASS_SIZE);
}

void FS::Inode::reset() {
	memset(this, 0, CLASS_SIZE);
	memset(this->blockX1, 0xff, 12 * sizeof(tBlockID));
}

bool FS::Inode::testUsed() {
	return bits.test(0);
}

void FS::Inode::setUsed(bool _Flag) {
	bits.set(0, _Flag);
}

bool FS::Inode::testWrite(tUid _Uid) {
	if(uid == _Uid)
		return bits.test(1);
	else
		return bits.test(3);
}

bool FS::Inode::testRead(tUid _Uid) {
	if(uid == _Uid)
		return bits.test(2);
	else
		return bits.test(4);
}

std::string FS::Inode::getModeString() {
	std::string _Mode("----");
	for(int i = 1; i <= 4; ++i) {
		if(bits.test(i)) {
			_Mode[i - 1] = i % 2 ? 'w' : 'r';
		}
	}
	return _Mode;
}

void FS::Inode::setWriteU(bool _Flag) {
	bits.set(1, _Flag);
}

void FS::Inode::setWrite(bool _Flag) {
	bits.set(3, _Flag);
}

void FS::Inode::setReadU(bool _Flag) {
	bits.set(2, _Flag);
}

void FS::Inode::setRead(bool _Flag) {
	bits.set(4, _Flag);
}

bool FS::Inode::setRW(const std::string & _au) {
	if(_au.size() < 4)
		return false;

	setWriteU(_au[0] == 'w' ? true : false);
	setReadU(_au[1] == 'r' ? true : false);
	setWrite(_au[2] == 'w' ? true : false);
	setRead(_au[3] == 'r' ? true : false);
	return true;
}

//FS::tInodeID FS::Inode::getID() {
//	return id;
//}
//
//void FS::Inode::setID(tInodeID _ID) {
//	id = _ID;
//}

FS::tUid FS::Inode::getUid() {
	return uid;
}

void FS::Inode::setUid(tUid _Uid) {
	uid = _Uid;
}

FS::tLinkCnt FS::Inode::getLinkCnt() {
	return linkCnt;
}

bool FS::Inode::setLinkUp() {
	if(linkCnt == MAX_LINK_CNT)
		return false;
	linkCnt++;
	return true;
}

bool FS::Inode::setLinkDown() {
	if(linkCnt == MIN_LINK_CNT)
		return false;
	linkCnt--;
	return true;
}

void FS::Inode::setLink(tLinkCnt _Val) {
	linkCnt = _Val;
}

FS::tFileSize FS::Inode::getFileSize() {
	return filesize;
}

void FS::Inode::setFileSize(tFileSize _Size) {
	filesize = _Size;
}

FS::tBlockID FS::Inode::getBlockCnt() {
	return blockCnt;
}

void FS::Inode::setBlockCnt(tBlockID _Val) {
	blockCnt = _Val;
}

std::vector<FS::tBlockID> FS::Inode::getBlockX1() {
	return std::vector<tBlockID>(blockX1, blockX1 + 10);
}

const FS::tBlockID * FS::Inode::getBlockX1_p() {
	return blockX1;
}

FS::tBlockID FS::Inode::getBlockX1(int _Pos) {
	return blockX1[_Pos];
}

FS::tBlockID FS::Inode::getBlockX2() {
	return blockX2;
}

FS::tBlockID FS::Inode::getBlockX3() {
	return blockX3;
}

void FS::Inode::setBlockX1(int _Pos, tBlockID _Val) {
	blockX1[_Pos] = _Val;
}

void FS::Inode::setBlockX2(tBlockID _Val) {
	blockX2 = _Val;
}

void FS::Inode::setBlockX3(tBlockID _Val) {
	blockX3 = _Val;
}