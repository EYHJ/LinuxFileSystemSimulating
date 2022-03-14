#ifndef DIRECTORY_H
#define DIRECTORY_H
#include "BasDef.h"
#include "File.h"
#include "DirEntry.h"
#include <cstring>
#include <unordered_map>

namespace FS {
	class Directory {
	public:
		Directory() = delete;
		explicit Directory(File* _Dfile);
		Directory(const Directory& _Right);
		Directory& operator=(const Directory& _Right);
		void fromFile();
		void toFile();
		bool addFile(tInodeID _id, std::string filename, bool isDir = false);
		void deleteFile(const std::string& _filename);

		std::unordered_map<std::string, DirEntry> list;
		File* dfile;
	};
}
#endif

