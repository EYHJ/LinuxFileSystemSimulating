#include "Session.h"
#include <fstream>
#include <cstring>

const std::string FS::Session::prompt = "$";
std::multiset<FS::tInodeID> FS::Session::curSet;
CRITICAL_SECTION* FS::Session::cs = nullptr;

FS::Session::Session(tUid _uid, FileSystem & _fs)
	:fsys(_fs),
	current(&fsys.getFile(fsys.superBlock.rootInode)),
	sout(std::string()) {
	uid = _uid;
	usedFlag = true;
	sbuf = nullptr;

	csLock();
	curSet.insert(fsys.superBlock.rootInode);
	csFree();
}

bool FS::Session::isOpen() {
	return usedFlag;
}

void FS::Session::close() {
	csLock();
	fsys.save();
	csFree();
	usedFlag = false;
}

void FS::Session::doCommand(std::istringstream in) {
	current.fromFile();//update current dir
	std::string token;
	in >> token;
	if(token == "exit") {
		close();
	}
	else if(token == "info") {
		in >> token;
		if(token == "-h") {
			sout << " command list, <> means must and [] means optional\n";
			sout << " info [-h]\n";
			sout << " cd <dir_path>\n";
			sout << " chmod <file_path_name> <authority_string>\n";
			sout << " dir [dir_path] [-s]\n";
			sout << " md <new_dir_name> [parent_dir]\n";
			sout << " rd <dir_path>\n";
			sout << " newfile <filename> [parent_dir]\n";
			sout << " cat <file_path_name>\n";
			sout << " copy <file_path_name> <dir>\n";
			sout << " del <file_path_name>\n";
			sout << " check\n";
			sout << " exit\n";
		}
		else {
			SBlock& sb = fsys.superBlock;
			sout << " inode amount: " << fsys.INODE_MAX_SIZE << '\n';
			sout << " inode avaliable num: " << sb.freeInodeCnt << '\n';
			sout << " inode used rate: " << 1 - (double)sb.freeInodeCnt / fsys.INODE_MAX_SIZE << '\n';
			sout << " block size: " << BLOCKSIZE << '\n';
			sout << " block amount: " << fsys.BLOCK_MAX_SIZE << '\n';
			sout << " block avaliable num: " << sb.freeBlockCnt << '\n';
			sout << " block used rate: " << 1 - (double)sb.freeBlockCnt / fsys.BLOCK_MAX_SIZE << '\n';
			sout << " block occupied by system: " << sb.firstBlockPos / BLOCKSIZE;
		}
		soutSend();
	}
	else if(token == "chmod") {
		std::string _authority;
		if(!(in >> token >> _authority)) {
			throw std::string("incomplete command");
		}
		csLock();
		tInodeID tid = fsys.openFile(token, current);
		Inode& _inode = fsys.inodeList[tid];
		csFree();
		if(!_inode.testWrite(uid)) {
			if(tid != current.list["."].getInodeID()) {
				csLock();
				fsys.closeFile(tid);
				csFree();
			}
			throw std::string("you are not allow to write this file");
		}
		csLock();
		if(!_inode.setRW(_authority)) {
			soutUrge("authority string is wrong");
		}
		else {
			sbufConfirmSelf();
		}
		if(tid != current.list["."].getInodeID())
			fsys.closeFile(tid);
		csFree();
	}
	else if(token == "cd") {
		if(in >> token) {
			csLock();
			Directory tem = fsys.openDir(token, current);
			if(!(*tem.dfile)().testRead(uid)) {
				csFree();
				throw std::string("you are not allow to read this dir");
			}
			curSet.insert(tem.list["."].getInodeID());
			curSet.erase(curSet.find(current.list["."].getInodeID()));
			csFree();
			current = tem;
		}
		sbufConfirmSelf();
	}
	else if(token == "dir") {
		Directory tdir = current;
		csLock();
		if(in >> token) {
			if(token != "-s")
				tdir = fsys.openDir(token, current);
			in >> token;
		}
		if(!(*tdir.dfile)().testRead(uid)) {
			csFree();
			throw std::string("you are not allow to read this dir");
		}
		int recursive = 0;
		if(token == "-s")
			recursive++;
		sout << "<filetype>filename  inodeID  authority  ownerUID  filesize\n";
		ls(tdir, recursive);
		csFree();
		soutSend();
	}
	else if(token == "md") {
		if(!(in >> token)) {
			throw std::string("incomplete command");
		}
		if(token.find('/') != std::string::npos) {
			throw std::string("unacceptable file name");
		}
		std::string _filename = token;
		Directory dir = current;
		csLock();
		if(in >> token) {
			dir = fsys.openDir(token, current);
		}
		if(!(*dir.dfile)().testWrite(uid)) {
			csFree();
			throw std::string("you are not allow to write this dir");
		}
		fsys.createDir(uid, dir, _filename);
		csFree();
		sbufConfirmSelf();
	}
	else if(token == "rd") {
		if(!(in >> token)) {
			throw std::string("incomplete command");
		}
		csLock();
		Directory dir = fsys.openDir(token, current);
		if(!(*dir.dfile)().testWrite(uid)) {
			csFree();
			throw std::string("you are not allow to write this dir");
		}
		tInodeID dirInodeID = dir.list["."].getInodeID();
		if(dirInodeID == 0) {
			csFree();
			throw std::string("can not delete the root dir");
		}
		if(curSet.count(dirInodeID)) {
			csFree();
			throw std::string("can not delete somebody's working directory");
		}
		if(dir.list.size() > 2) {
			soutUrge("dir to delete is not empty, input y to confirm\t", false);
			std::istringstream confirmMsg = sin();
			std::string cbuf;
			confirmMsg >> cbuf;
			if(cbuf != "y") {
				csFree();
				sbufConfirmSelf();
				return;
			}
		}
		std::string _filename;
		_filename = (token.rfind('/') == std::string::npos ? token : token.substr(token.rfind('/') + 1));
		tInodeID pid = dir.list[".."].getInodeID(); 
		Directory _parentDir(&fsys.getFile(pid));
		_parentDir.deleteFile(_filename);
		fsys.writeFileBack(pid);

		fsys.deleteDirInside(dir);
		fsys.deleteFile(dirInodeID);
		csFree();
		sbufConfirmSelf();
	}
	else if(token == "newfile") {
		//check command
		if(!(in >> token)) {
			throw std::string("incomplete command");
		}
		//complete context
		std::string _filename = token;
		Directory _context = current;
		csLock();
		if(in >> token) {
			_context = fsys.openDir(token, current);
		}
		if(!(*_context.dfile)().testWrite(uid)) {
			csFree();
			throw std::string("you are not allow to write this dir");
		}
		csFree();

		//get input
		soutUrge("<<input file\n<<input \"-save\" as a single line to save and quit input\n<< \"-quit\" to quit and abandon\n");
		std::string buf;
		while(true) {
			token = sin().str();
			if(token == "-quit") {
				sbufConfirmSelf();
				return;
			}
			else if(token == "-save") {
				break;
			}
			buf.append(token + "\n");
			sbufConfirmSelf();
		}
		if(buf.size() > Inode::MAX_FILE_BLOCK_NUM*BLOCKSIZE) {
			throw std::string("newfile fail: the new file is over size");
		}
		csLock();
		//create file
		tInodeID tid = fsys.createFile();
		fsys.inodeBitset.set(tid, true);
		File& file = fsys.getFile(tid);
		file().setUid(uid);
		file().setReadU(true);
		file().setWriteU(true);
		auto _filesize = buf.size();
		file.resize(_filesize);
		memcpy(file.cstr(), buf.c_str(), _filesize);
		fsys.writeFileBack(tid);
		fsys.closeFile(tid);
		//add to dir
		_context.addFile(tid, _filename);
		fsys.writeFileBack(_context.list["."].getInodeID());
		csFree();
		sbufConfirmSelf();
	}
	else if(token == "cat") {
		if(!(in >> token)) {
			throw std::string("incomplete command");
		}
		csLock();
		tInodeID tid = fsys.openFile(token, current);
		File& file = fsys.getFile(tid);
		if(!file().testRead(uid)) {
			csFree();
			throw std::string("you are not allow to read this file");
		}
		sout << std::string(file.cstr(), file().getFileSize());
		fsys.closeFile(tid);
		csFree();
		soutSend();
	}
	else if(token == "copy") {
		std::string oldname;
		if(!(in >> oldname >> token)) {
			throw std::string("incomplete command");
		}
		size_t _filesize;
		char* _filebuf = nullptr;
		std::string _filename;
		//host to this
		if(oldname.find("<host>") == 0) {
			if(token.find("<host>") == 0) {
				throw std::string("can not copy from host to host");
			}
			std::fstream _filehost(oldname.substr(6), std::ios::in | std::ios::binary);
			if(!_filehost.is_open()) {
				throw std::string("host file does not exist");
			}
			_filename = oldname.substr(oldname.rfind('/') != std::string::npos ? oldname.rfind('/') + 1 : 6);
			_filehost.seekg(0, std::ios::end);
			_filesize = _filehost.tellg();
			if(_filesize > Inode::MAX_FILE_BLOCK_NUM*BLOCKSIZE) {
				throw std::string("host file is over size");
			}
			_filebuf = new char[_filesize + 1];
			_filebuf[_filesize] = '\0';
			_filehost.seekg(std::ios::beg);
			_filehost.read(_filebuf, _filesize);
			_filehost.close();
		}
		else {
			_filename = oldname.substr(oldname.rfind('/') != std::string::npos ? oldname.rfind('/') + 1 : 0);
			//this to host
			if(token.find("<host>") == 0) {
				_filename = oldname.substr(oldname.rfind('/') != std::string::npos ? oldname.rfind('/') + 1 : 0);
				std::string _hostpath = token.substr(6);
				if(!_hostpath.empty()) {
					_hostpath.append("/");
				}
				
				csLock();
				tInodeID _tidthis = fsys.openFile(oldname, current);
				fsys.getPreFile(_tidthis);
				File& _filethis = fsys.getFile(_tidthis);
				if(!_filethis().testRead(uid)) {
					fsys.closeFile(_tidthis);
					csFree();
					throw std::string("you are not allow to read this file");
				}
				std::fstream _filehost(_hostpath + _filename, std::ios::out | std::ios::binary);
				if(!_filehost.is_open()) {
					fsys.closeFile(_tidthis);
					csFree();
					throw std::string("can not create host file to copy");
				}
				_filehost.write(_filethis.cstr(), _filethis().getFileSize());

				fsys.closeFile(_tidthis);
				csFree();
				_filehost.close();
				sbufConfirmSelf();
				return;
			}
			//this fs to this
			csLock();
			tInodeID tid = fsys.openFile(oldname, current);
			File& _file = fsys.getFile(tid);
			if(!_file().testRead(uid)) {
				fsys.closeFile(tid);
				csFree();
				throw std::string("you are not allow to read this file");
			}
			_filesize = _file().getFileSize();
			_filebuf = new char[_filesize +1];
			_filebuf[_filesize] = '\0';
			memcpy(_filebuf, _file.cstr(), _filesize);
			fsys.closeFile(tid);
			csFree();
		}
		csLock();
		Directory dir = fsys.openDir(token, current);
		if(!(*dir.dfile)().testWrite(uid)) {
			delete[] _filebuf;
			csFree();
			throw std::string("you are not allow to write this dir");
		}
		tInodeID tid = fsys.createFile();
		fsys.inodeBitset.set(tid, true);
		File& _file = fsys.getFile(tid);
		_file().setUid(uid);
		_file().setReadU(true);
		_file().setWriteU(true);
		_file.resize(_filesize);
		memcpy(_file.cstr(), _filebuf, _filesize);
		fsys.writeFileBack(tid);
		dir.addFile(tid, _filename);
		fsys.writeFileBack(dir.list["."].getInodeID());
		fsys.closeFile(tid);
		csFree();
		delete[] _filebuf;
		sbufConfirmSelf();
	}
	else if(token == "del") {
		if(!(in >> token)) {
			throw std::string("incomplete command");
		}
		csLock();
		tInodeID tid = fsys.openFile(token, current);
		File& _file = fsys.getFile(tid);
		if(!_file().testWrite(uid)) {
			csFree();
			throw std::string("you are not allow to write this file");
		}
		auto pattern = token.rfind('/');
		Directory _dir = current;
		std::string _filename = token;

		if(pattern != std::string::npos) {
			_filename = token.substr(pattern + 1);
			_dir = fsys.openDir(token.substr(0, pattern), current);
		}

		if(_dir.list[_filename].testDir()) {
			fsys.closeFile(tid);
			throw std::string("can not del a dir, use command \"rd\" instead");
		}

		fsys.deleteFile(tid);
		_dir.deleteFile(_filename);
		fsys.writeFileBack(_dir.list["."].getInodeID());
		csFree();
		sbufConfirmSelf();
	}
	else if(token == "check") {
		for(size_t i = 0; i < fsys.INODE_MAX_SIZE; ++i) {
			if(fsys.inodeList[i].testUsed() != fsys.inodeBitset.test(i)) {
				csLock();
				fsys.inodeBitset.set(i, fsys.inodeList[i].testUsed());
				csFree();
			}
		}
		csLock();
		fsys.save();
		csFree();
		soutUrge("self check and fix are done");
	}
	else {
		soutUrge("unknow command");
	}
}

std::istringstream FS::Session::sin() {
	std::string sinbuf;
	while(sbuf[0] != '0')
		Sleep(25);
	sbuf[0] = '1';
	while(true) {
		while(sbuf[0] != 'c')
			Sleep(25);
		sinbuf.append(std::string(sbuf + 2));
		if(sbuf[1] == '0') {
			break;
		}
		else
			sbuf[0] = '1';
	}
	return std::istringstream(sinbuf);
}

void FS::Session::soutSend() {
	soutUrge(sout.str());
	sout.str(std::string());
}

void FS::Session::soutUrge(std::string _dataout, bool withEndl) {
	if(withEndl)
		_dataout.append("\n");
	size_t rsize = SBUF_SIZE - 3;
	size_t pos = 0;
	size_t size = _dataout.size();

	size_t wsize;
	bool needMore;
	while(pos < size) {
		while(sbuf[0] != 'c')
			Sleep(25);
		sbufClr();
		wsize = size - pos;
		needMore = wsize > rsize;
		memcpy(sbuf + 2, &_dataout[pos], needMore ? rsize : wsize);
		sbufMore(needMore);
		sbuf[0] = 's';
		pos += rsize;
	}
}

void FS::Session::setSessionBuffer(char * _buf) {
	sbuf = _buf;
	sbuf[0] = '0';
	sbuf[SBUF_SIZE - 1] = '\0';
}

void FS::Session::setCriticalSection(CRITICAL_SECTION * _cs) {
	cs = _cs;
}

void FS::Session::csLock() {
	EnterCriticalSection(cs);
}

void FS::Session::csFree() {
	LeaveCriticalSection(cs);
}

void FS::Session::sbufClr() {
	memset(sbuf + 2, 0, SBUF_SIZE - 2);
}

void FS::Session::sbufMore(bool _Flag) {
	if(_Flag)
		sbuf[1] = '1';
	else
		sbuf[1] = '0';
}

void FS::Session::sbufConfirmSelf() {
	sbuf[0] = '0';
}

void FS::Session::ls(Directory & _dir, int _recursive) {
	for(auto& iter : _dir.list) {
		DirEntry& entry = iter.second;
		std::string filename = entry.getFilename();
		if(filename == "." || filename == "..")
			continue;

		//print files details
		tInodeID tid = entry.getInodeID();
		Inode& inode = fsys.getInode(tid);
		for(int i = 0; i < _recursive - 1; ++i)
			sout << "  ";
		if(entry.testDir())
			sout << "<dir>" << entry.getFilename();
		else
			sout << "<file>" << entry.getFilename();
		sout << "  " << tid;
		sout << "  " << inode.getModeString();
		sout << "  " << int(inode.getUid());
		sout << "  " << inode.getFileSize() << "\n";

		//recusion
		if(_recursive != 0 && entry.testDir()) {
			fsys.getPreFile(tid);
			Directory subdir(&fsys.getFile(tid));
			ls(subdir, _recursive + 1);
		}
	}
}
