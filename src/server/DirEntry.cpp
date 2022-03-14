//#define _CRT_SECURE_NO_WARNINGS
#include "DirEntry.h"

FS::DirEntry::DirEntry() {
	reset();
}

FS::DirEntry::DirEntry(const DirEntry & _Right) {
	memcpy(this, &_Right, CLASS_SIZE);
}

void FS::DirEntry::reset() {
	std::memset(this, 0, CLASS_SIZE);
}

bool FS::DirEntry::testUsed() {
	return UsedFlag;
}

void FS::DirEntry::setUsed(bool _Right) {
	UsedFlag = _Right;
}

bool FS::DirEntry::testDir() {
	return dirFlag;
}

bool FS::DirEntry::testFile() {
	return !dirFlag;
}

void FS::DirEntry::setDir() {
	dirFlag = true;
}

void FS::DirEntry::setFile() {
	dirFlag = false;
}

FS::tInodeID FS::DirEntry::getInodeID() {
	return inodeID;
}

void FS::DirEntry::setInodeID(tInodeID _InodeID) {
	inodeID = _InodeID;
}

std::string FS::DirEntry::getFilename() {
	return std::string(filename);
}

const char * FS::DirEntry::getFilename_p() {
	return filename;
}

void FS::DirEntry::setFilename(const std::string& _Name) {
	size_t cnt = NAME_BUF_LENGTH - 1;
	if(_Name.size() < cnt)
		cnt = _Name.size();
	memcpy(filename, _Name.c_str(), cnt);
	filename[NAME_BUF_LENGTH - 1] = '\0';
}

