#include <iostream>
#include <fstream>
#include <cstring>
#include <Windows.h>
#include <thread>
#include "FileSystem.h"
#include "Session.h"
using namespace std;
using namespace FS;

void multiSession(FileSystem& _fs, tUid _uid, string _bufName);

int main() {
	//initialize virtual disc
	FileSystem *fs = nullptr;
	bool bc = true;
	while(bc) {
		try {
			fs = new FileSystem("fs.disc");
			bc = false;
			cout << " connect virtual disc successfully" << endl;
		}
		catch(std::string errorMsg) {
			cout << " fail to initialize" << endl;
			cout << " " << errorMsg << endl;
			fstream disc("fs.disc", ios::out | ios::binary);
			size_t discsize = FS::BLOCKSIZE * 1024 * 100;
			char *buf = new char[discsize + 1];
			memset(buf, 0, discsize + 1);
			disc.write(buf, discsize);
			disc.close();
			delete[] buf;
		}
	}

	//set to manage user
	//set<tUid> uidset;
	set<tUid> uidset;

	//memory mapping
	HANDLE hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		Session::SBUF_SIZE,
		"SessionIndex"
	);

	if(!hMapFile) {
		delete fs;
		cout << "can not create shared memory, try to restart the program";
		getchar();
		return 1;
	}

	LPVOID mmp = MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		Session::SBUF_SIZE
	);

	//create critical section and mutex
	HANDLE mutex = CreateMutex(NULL, true, "LoginMutex");
	if(!mutex) {
		delete fs;
		UnmapViewOfFile(mmp);
		CloseHandle(hMapFile);
		cout << "can not synchronize, try to restart the program";
		getchar();
		return 1;
	}

	CRITICAL_SECTION cs4session;
	CRITICAL_SECTION cs4main;
	InitializeCriticalSection(&cs4session);
	InitializeCriticalSection(&cs4main);


	//run
	Session::setCriticalSection(&cs4session);
	Session session(0, *fs);
	session.setSessionBuffer((char*)mmp);
	string token;
	tUid uid;
	int uidInt;
	bool working = true;

	//thread laji([&working, &cs4main, &abandonUid, &uidmap] {
	//	while(working) {
	//		EnterCriticalSection(&cs4main);
	//		if(!abandonUid.empty()) {
	//			for(tUid _uid : abandonUid) {
	//				uidmap.erase(_uid);
	//			}
	//		}
	//		LeaveCriticalSection(&cs4main);
	//		Sleep(25);
	//	}
	//});

	ReleaseMutex(mutex);
	while(working) {
		istringstream in = session.sin();
		in >> uidInt;
		uid = uidInt;
		if(!working)
			break;
		if(uidset.count(uid)) {
			session.soutUrge("fail");
		}
		else {
			string sbufName = "SessionBuffer";
			sbufName.append(uid, 'f');
			EnterCriticalSection(&cs4main);
			uidset.insert(uid);
			thread* aNewSession = new thread([&, uid, sbufName] {
				string nameWithEnld = sbufName;
				nameWithEnld.append("\n");
				multiSession(*fs, uid, nameWithEnld);
				EnterCriticalSection(&cs4main);
				uidset.erase(uid);
				if(uidset.empty()) {
					WaitForSingleObject(mutex, INFINITE);
					working = false;
					((char*)mmp)[0] = '0';
					Sleep(30);
					((char*)mmp)[0] = 'c';
				}
				LeaveCriticalSection(&cs4main);
				});
			LeaveCriticalSection(&cs4main);

			session.soutUrge(sbufName);
		}

	}

	delete fs;
	UnmapViewOfFile(mmp);
	CloseHandle(hMapFile);
	CloseHandle(mutex);
	DeleteCriticalSection(&cs4session);
	DeleteCriticalSection(&cs4main);

	return 0;
}


void multiSession(FileSystem& _fs, tUid _uid, string _bufName) {
	//memory mapping
	HANDLE hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		Session::SBUF_SIZE,
		_bufName.c_str()
	);

	if(!hMapFile) {
		cout << "create shared memory for " << _uid << " fail" << endl;
		return;
	}

	LPVOID mmp = MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		Session::SBUF_SIZE
	);

	Session subSession(_uid, _fs);
	subSession.setSessionBuffer((char*)mmp);
	while(subSession.isOpen()) {
		try {
			subSession.doCommand(subSession.sin());
		}
		catch(string errorMag) {
			subSession.soutUrge(errorMag);
		}
	}

	UnmapViewOfFile(mmp);
	CloseHandle(hMapFile);
}
