#include "File.h"

FS::File::File(Inode* _inode)
	:inode(_inode), data(_inode->getBlockCnt()*BLOCKSIZE), blockIDList() {
}

FS::File::File(const File & _Right)
	:inode(_Right.inode),data(_Right.data),blockIDList(_Right.blockIDList){
}

FS::Inode & FS::File::operator()() {
	return std::ref(*inode);
}


char * FS::File::cstr(size_t pos) {
	return &data[pos];
}

void FS::File::resize(tFileSize _filesize) {
	data.resize(_filesize);
}
