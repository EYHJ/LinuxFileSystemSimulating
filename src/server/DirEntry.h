#ifndef DIRENTRY_H
#define DIRENTRY_H
#include "BasDef.h"
#include <cstring>
#include <string>

namespace FS {
	class DirEntry {
	public:
		static constexpr ull CLASS_SIZE = 128;
		static constexpr ull NAME_BUF_LENGTH = CLASS_SIZE - 2 * sizeof(bool) - sizeof(tInodeID);

		DirEntry();
		DirEntry(const DirEntry& _Right);
		void reset();

		bool testUsed();
		void setUsed(bool _Right);

		bool testDir();
		bool testFile();
		void setDir();
		void setFile();

		tInodeID getInodeID();
		void setInodeID(tInodeID _InodeID);

		std::string getFilename();
		const char* getFilename_p();
		void setFilename(const std::string& _Name);
	
	protected:
		bool UsedFlag;
		bool dirFlag;
		tInodeID inodeID;
		char filename[NAME_BUF_LENGTH];
	};

}


#endif

