#include "FileSystem.h"

FS::FileSystem::FileSystem(const char * const _discName)
	:BasicFileIO(_discName){
	tInodeID rid = 0;
	if(rid != superBlock.rootInode) {
		superBlock.rootInode = 0;
		//set root inode
		Inode& inode = getInode(rid);
		inode.reset();
		inode.setUsed(true);
		//define . and .. as must 
		DirEntry must[2];
		must[0].setDir();
		must[0].setUsed(true);
		must[0].setInodeID(rid);
		must[0].setFilename("..");
		must[1].setDir();
		must[1].setUsed(true);
		must[1].setInodeID(rid);
		must[1].setFilename(".");
		//add must to file
		getPreFile(rid);
		File& file = getFile(rid);
		file().setWriteU(true);
		file().setWrite(true);
		file().setReadU(true);
		file().setRead(true);
		size_t size2entry = 2 * DirEntry::CLASS_SIZE;
		file.resize(size2entry);
		memcpy(file.cstr(), must, size2entry);
		writeFileBack(rid);
	}
	getPreFile(rid);
}


FS::Directory FS::FileSystem::openDir(const std::string& _path, Directory context) {
	auto pattern = _path.find('/');
	memFile[0];
	//root path begin with '/'
	if(pattern == 0) {
		Directory temdir(&getFile(superBlock.rootInode));
		return openDir(_path.substr(1), temdir);
	}
	//return context for empty path
	if(_path.empty()) {
		return context;
	}
	//
	if(pattern == std::string::npos) {
		if(context.list.count(_path)) {
			DirEntry& ans = context.list[_path];
			if(ans.testDir()) {
				getPreFile(ans.getInodeID());
				return Directory(&getFile(ans.getInodeID()));
			}
		}
		throw std::string("can not find such dir");
	}
	//recursion part
	std::string cdir = _path.substr(0, pattern);
	if(context.list.count(cdir)) {
		DirEntry& tem = context.list[cdir];
		if(tem.testDir()) {
			getPreFile(tem.getInodeID());
			File& file = getFile(tem.getInodeID());
			return openDir(_path.substr(pattern + 1), Directory(&file));
		}
	}
	throw std::string("can not find such dir");
}

//already get pre file
FS::tInodeID FS::FileSystem::openFile(std::string _path, Directory& context) {
	auto pattern = _path.rfind('/');
	if(pattern == std::string::npos) {	//if the path is relative
		if(context.list.count(_path)) {
			DirEntry& tem = context.list[_path];
			getPreFile(tem.getInodeID());
			return tem.getInodeID();
		}
	}
	else {	//if the path is absolute
		Directory dir = openDir(_path.substr(0, pattern), context);
		std::string filename = _path.substr(pattern + 1);
		if(dir.list.count(filename)) {
			DirEntry& tem = dir.list[filename];
			getPreFile(tem.getInodeID());
			return tem.getInodeID();
		}
	}
	throw std::string("can not find such file");
}

void FS::FileSystem::createDir(tUid _uid, Directory & _parent, const std::string _name) {
	if(_parent.list.count(_name)) {
		throw std::string("dir " + _name + " has already existed");
	}
	tInodeID tid = createFile();
	inodeBitset.set(tid, true);
	tInodeID tidp = _parent.list["."].getInodeID();
	//define . and .. as must 
	DirEntry must[2];
	must[0].setDir();
	must[0].setUsed(true);
	must[0].setInodeID(tidp);
	must[0].setFilename("..");
	must[1].setDir();
	must[1].setUsed(true);
	must[1].setInodeID(tid);
	must[1].setFilename(".");
	//add must to file
	File& file = getFile(tid);
	file().setUid(_uid);
	file().setWriteU(true);
	file().setReadU(true);
	size_t size2entry = 2 * DirEntry::CLASS_SIZE;
	file.resize(size2entry);
	memcpy(file.cstr(), must, size2entry);
	writeFileBack(tid);
	//add the new dir to its parent dir
	_parent.addFile(tid, _name, true);
	writeFileBack(tidp);
	//haven't close any file?
	closeFile(tid);
}

void FS::FileSystem::deleteDirInside(Directory& _dir) {
	for(auto& iter : _dir.list) {
		if(iter.second.getFilename() == "." || iter.second.getFilename() == "..")
			continue;
		if(iter.second.testDir()) {
			tInodeID tid = iter.second.getInodeID();
			getPreFile(tid);
			File& secDirFile = getFile(tid);
			Directory secDir(&secDirFile);
			deleteDirInside(secDir);
		}
		deleteFile(iter.second.getInodeID());
	}
}

//already get pre file
FS::tInodeID FS::FileSystem::createFile() {
	tInodeID tid = getAvlInodeID();
	superBlock.setFreeInodeCntDown();
	memFile.insert(std::pair<tInodeID,File>(tid, *new File(&inodeList[tid])));
	File& file = memFile[tid];
	file().reset();
	file().setUsed(true);
	return tid;
}

void FS::FileSystem::closeFile(tInodeID _id) {
	if(memFile.count(_id))
		memFile.erase(_id);
}

FS::tInodeID FS::FileSystem::getPreFile(tInodeID _id) {
	if(inodeList[_id].testUsed() == false)
		throw std::string("file doesn't exist");
	if(!memFile.count(_id)) {
		memFile.insert(std::pair<tInodeID,File>(_id, *new File(&inodeList[_id])));
		fillFileBlockID(memFile[_id]);
		fillFileBlocks(memFile[_id]);
	}
	return _id;
}

FS::File & FS::FileSystem::getFile(tInodeID _id) {
	return std::ref(memFile[_id]);
}

void FS::FileSystem::fillFileBlockID(File & _file) {
	_file.blockIDList.clear();
	for(auto t : _file().getBlockX1()) {
		if(t == BAD_BLOCK_ID)
			return;
		_file.blockIDList.emplace_back(t);
	}

	tBlockID tid = _file().getBlockX2();
	if(tid == BAD_BLOCK_ID)
		return;
	std::array<tBlockID, Inode::PAGE_CNT_INODE> bl;
	getBlockCstr(tid, (char*)&bl[0]);
	for(auto t : bl) {
		if(t == BAD_BLOCK_ID)
			return;
		_file.blockIDList.emplace_back(t);
	}

	tid = _file().getBlockX3();
	if(tid == BAD_BLOCK_ID)
		return;
	std::array<tBlockID, Inode::PAGE_CNT_INODE> blx3;
	getBlockCstr(tid, (char*)&blx3[0]);
	for(auto tx3 : blx3) {
		if(tx3 == BAD_BLOCK_ID)
			return;
		getBlockCstr(tx3, (char*)&bl[0]);
		for(auto t : bl) {
			if(t == BAD_BLOCK_ID)
				return;
			_file.blockIDList.emplace_back(t);
		}
	}
}

void FS::FileSystem::fillFileBlocks(File & _file) {
	size_t pos = 0;
	_file.resize(_file().getFileSize());
	for(auto tid : _file.blockIDList) {
		getBlockCstr(tid, _file.cstr(pos));
		pos += BLOCKSIZE;
	}
}

void FS::FileSystem::writeFileBack(tInodeID _id) {
	getPreFile(_id);
	File& file = memFile[_id];
	size_t pos = 0;
	size_t blockCnt = file.data.size() / BLOCKSIZE;
	size_t last = file.data.size() % BLOCKSIZE;
	size_t pageBlockCnt = Inode::PAGE_CNT_INODE;
	if(last)
		blockCnt++;
	else
		last = BLOCKSIZE;

	//error assert
	if(blockCnt > Inode::MAX_FILE_BLOCK_NUM)
		throw std::string("file size overflow");

	//align block id list
	tBlockID tid = BAD_BLOCK_ID;
	while(blockCnt > file.blockIDList.size()) {
		tid = getAvlBlockID();
		if(tid == BAD_BLOCK_ID)
			throw std::string("run out of blocks");
		else {
			blockBitset.set(tid, true);
			superBlock.setFreeBlockCntDown();
		}
		file.blockIDList.emplace_back(tid);
	}
	if(blockCnt < file.blockIDList.size()) {
		for(int i = blockCnt; i < file.blockIDList.size(); ++i) {
			blockBitset.set(file.blockIDList[i], false);
			superBlock.setFreeBlockCntUp();
		}
		file.blockIDList.resize(blockCnt);
	}

	//write blocks
	for(int i = 0; i < blockCnt - 1; ++i) {
		writeBlock(file.blockIDList[i], file.cstr(i * BLOCKSIZE));
	}
	writeBlock(file.blockIDList[blockCnt - 1], file.cstr((blockCnt - 1) * BLOCKSIZE), last);

	//write else in inode
	file().setBlockCnt(blockCnt);
	file().setFileSize(file.data.size());
	blockCnt++;
	file.blockIDList.emplace_back(BAD_BLOCK_ID);
	//write block id in inode
	//X1
	size_t i = 0;
	for(; i < 10; ++i) {
		if(i == blockCnt)
			break;
		file().setBlockX1(i, file.blockIDList[i]);
	}
	//assert
	if(i == blockCnt) {
		tid = file().getBlockX2();
		if(tid != BAD_BLOCK_ID) {
			file().setBlockX2(BAD_BLOCK_ID);
			blockBitset.set(tid, false);
			superBlock.setFreeBlockCntUp();
		}
		tid = file().getBlockX3();
		if(tid != BAD_BLOCK_ID) {
			file().setBlockX3(BAD_BLOCK_ID);
			blockBitset.set(tid, false);
			superBlock.setFreeBlockCntUp();
		}
		fillFileBlockID(file);
		return;
	}
	//X2
	tid = file().getBlockX2();
	if(tid == BAD_BLOCK_ID) {
		tid = getAvlBlockID();
		blockBitset.set(tid, true);
		superBlock.setFreeBlockCntDown();
		file().setBlockX2(tid);
	}
	if(blockCnt - i <= pageBlockCnt) {
		writeBlock(tid, (char*)&file.blockIDList[i], (blockCnt - i) * sizeof(tBlockID));
		tid = file().getBlockX3();
		if(tid != BAD_BLOCK_ID) {
			file().setBlockX3(BAD_BLOCK_ID);
			blockBitset.set(tid, false);
			superBlock.setFreeBlockCntUp();
		}
		fillFileBlockID(file);
		return;
	}
	else {
		writeBlock(tid, (char*)&file.blockIDList[i]);
		i += pageBlockCnt;
	}
	//assert
	if(i == blockCnt) {
		tid = file().getBlockX3();
		if(tid != BAD_BLOCK_ID) {
			file().setBlockX3(BAD_BLOCK_ID);
			blockBitset.set(tid, false);
			superBlock.setFreeBlockCntUp();
		}
		fillFileBlockID(file);
		return;
	}
	//X3
	tid = file().getBlockX3();
	std::vector<tBlockID> arr(pageBlockCnt, BAD_BLOCK_ID);
	if(tid == BAD_BLOCK_ID) {
		tid = getAvlBlockID();
		blockBitset.set(tid, true);
		superBlock.setFreeBlockCntDown();
		file().setBlockX3(tid);
	}
	else {
		getBlockCstr(tid, (char*)&arr[0]);
	}
	size_t t = 0;
	for(t = 0; t < pageBlockCnt; ++t) {
		if(arr[t] == BAD_BLOCK_ID) {
			arr[t] = getAvlBlockID();
			blockBitset.set(arr[t], true);
			superBlock.setFreeBlockCntDown();
		}

		if(blockCnt - i < pageBlockCnt) {
			if(blockCnt - i == 0)
				break;
			writeBlock(arr[t], (char*)&file.blockIDList[i], (blockCnt - i) * sizeof(tBlockID));
			std::vector<tBlockID> bbuf(256);
			getBlockCstr(arr[t], (char*)&bbuf[0]);
			char bbb[1025];
			getBlockCstr(bbuf[13], bbb);
			break;
		}
		else {
			writeBlock(arr[t], (char*)&file.blockIDList[i]);
			i += pageBlockCnt;
		}
	}
	//write back x3 block
	++t;
	if(t < pageBlockCnt)
		arr[t] = BAD_BLOCK_ID;
	writeBlock(tid, (char*)&arr[0]);
	fillFileBlockID(file);
}

//with the file closed
void FS::FileSystem::deleteFile(tInodeID _id) {
	getPreFile(_id);
	File& file = memFile[_id];
	for(auto& tid : file.blockIDList) {
		blockBitset.set(tid, false);
		superBlock.setFreeBlockCntUp();
		tid = BAD_BLOCK_ID;
	}
	writeFileBack(_id);
	inodeList[_id].reset();
	inodeBitset.set(_id, false);
	memFile.erase(_id);
	superBlock.setFreeInodeCntUp();
}
