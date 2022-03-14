#ifndef FILE_H
#define FILE_H
#include "BasDef.h"
#include "Inode.h"
#include <vector>

namespace FS {
	class File {
	public:
		File() {
			inode = nullptr;
		}
		explicit File(Inode* _inode);
		File(const File& _Right);
		Inode& operator() ();

		Inode* inode;
		std::vector<tBlockID> blockIDList;
		std::vector<char> data;

		char* cstr(size_t pos = 0);
		void resize(tFileSize _filesize);
	};
}

#endif

