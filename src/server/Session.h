#ifndef SESSION_H
#define SESSION_H
#include "FileSystem.h"
#include "Directory.h"
#include <Windows.h>
#include <sstream>
#include <string>
#include <set>
namespace FS {
	class Session {
	public:
		Session() = delete;
		Session(tUid _uid, FileSystem& _fs);

		bool isOpen();
		void close();
		void doCommand(std::istringstream in);

		std::istringstream sin();
		void soutSend();
		void soutUrge(std::string _dataout, bool withEndl = true);
		void setSessionBuffer(char* _buf);
		static void setCriticalSection(CRITICAL_SECTION* _cs);
		
		static const std::string prompt;
		static constexpr size_t SBUF_SIZE = 1024;

	private:
		FileSystem& fsys;
		Directory current;

		static std::multiset<tInodeID> curSet;

		static CRITICAL_SECTION* cs;
		void csLock();
		void csFree();

		char* sbuf;
		void sbufClr();
		void sbufMore(bool _Flag);
		void sbufConfirmSelf();

		tUid uid;
		bool usedFlag;
		std::ostringstream sout;

		void ls(Directory& _dir, int _recursive = 0);
	};
}
#endif
