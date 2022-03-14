#include "SessionClient.h"
#include <Windows.h>
#include <iostream>
#include <sstream>
using namespace std;

int main() {
	//open login memory mapping
	HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS,
									  NULL, 
									  "SessionIndex");

	if(!hMapFile) {
		cout << "can not load shared memory, try to restart the program";
		getchar();
		return 1;
	}
	LPVOID mmp = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	//open login mutex
	HANDLE mutex = OpenMutex(MUTEX_ALL_ACCESS, true, "LoginMutex");
	if(!mutex) {
		UnmapViewOfFile(mmp);
		CloseHandle(hMapFile);
		cout << "can not synchronize, try to restart the program";
		getchar();
		return 1;
	}

	//create session
	SessionClient sc;
	sc.setSessionBuffer((char*)mmp);
	string buf;

	//login
	cout << " input your user id: ";
	int uidInt;
	cin >> uidInt;
	if(uidInt > 255 || uidInt < 0) {
		UnmapViewOfFile(mmp);
		CloseHandle(hMapFile);
		CloseHandle(mutex);
		cout << "id should between 0 to 255, restart to input again";
		getchar();
		return 1;
	}
	ostringstream int2s("");
	int2s << uidInt;

	WaitForSingleObject(mutex, INFINITE);
	sc.c2s(string(int2s.str()));
	sc.s2c(buf);
	ReleaseMutex(mutex);
	UnmapViewOfFile(mmp);
	CloseHandle(hMapFile);
	if(buf == "fail") {
		CloseHandle(mutex);
		cout << "id " << uidInt << " is already login in order client!";
		getchar();
		return 1;
	}
	//redireact
	hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS,
							   NULL,
							   buf.c_str());
	if(!hMapFile) {
		cout << "can not load shared memory, try to restart the program";
		getchar();
		return 1;
	}
	mmp = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	sc.setSessionBuffer((char*)mmp);
	cin.get();

	//run
	while(true) {
		cout << sc.prompt;
		getline(cin, buf);
		if(buf == "exit")
			break;

		sc.c2s(buf);
		if(sc.s2c(buf))
			cout << buf;
	}

	sc.c2s("exit");
	UnmapViewOfFile(mmp);
	CloseHandle(hMapFile);
	CloseHandle(mutex);
}