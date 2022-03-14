#include "Directory.h"

FS::Directory::Directory(File * _Dfile)
	:dfile(_Dfile){
	fromFile();
}

FS::Directory::Directory(const Directory & _Right)
	: dfile(_Right.dfile), list(_Right.list) {
	fromFile();
}

FS::Directory & FS::Directory::operator=(const Directory & _Right) {
	dfile = _Right.dfile;
	list = _Right.list;
	fromFile();
	return *this;
}

void FS::Directory::fromFile() {
	tFileSize filesize = dfile->data.size();
	std::vector<DirEntry> tem(filesize / DirEntry::CLASS_SIZE);
	memcpy(&tem[0], dfile->cstr(), filesize);
	list.clear();
	for(auto& iter : tem) {
		list.insert(std::make_pair(iter.getFilename(), iter));
	}
}

//haven't write back to disc
void FS::Directory::toFile() {
	std::vector<DirEntry> tem;
	for(auto& iter : list) {
		tem.emplace_back(iter.second);
	}

	tFileSize filesize = tem.size() * DirEntry::CLASS_SIZE;
	dfile->resize(filesize);
	memcpy(dfile->cstr(), &tem[0], filesize);
}

//haven't write back to disc
bool FS::Directory::addFile(tInodeID _id, std::string _filename, bool _isDir) {
	if(list.count(_filename))
		throw std::string(_filename + " is already exist");

	list.insert(std::make_pair(_filename, DirEntry()));
	auto& iter = list[_filename];

	iter.setInodeID(_id);
	iter.setFilename(_filename);
	if(_isDir)
		iter.setDir();
	iter.setUsed(true);
	toFile();
	return true;
}

//haven't write back to disc
void FS::Directory::deleteFile(const std::string & _filename) {
	if(!list.count(_filename))
		throw std::string("can not delete file that does not exist");
	list.erase(_filename);
	toFile();
}
