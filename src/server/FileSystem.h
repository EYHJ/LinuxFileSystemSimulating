#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include "BasicFileIO.h"
#include "File.h"
#include "Directory.h"
#include <unordered_map>

namespace FS {
	class FileSystem:
		public BasicFileIO {
	public:
		explicit FileSystem(const char* const _discName);
		FileSystem() = delete;
		FileSystem(const FileSystem&) = delete;

		Directory openDir(const std::string& _path, Directory context);
		tInodeID openFile(std::string _path, Directory& context);
		void createDir(tUid _uid, Directory& _parent, const std::string _name);
		void deleteDirInside(Directory& _dir);
		void closeFile(tInodeID _id);

		tInodeID createFile();
		tInodeID getPreFile(tInodeID _id);
		File& getFile(tInodeID _id);
		void writeFileBack(tInodeID _id);
		void deleteFile(tInodeID _id);

	//private:
		std::unordered_map<tInodeID, File> memFile;

		void fillFileBlockID(File& _file);
		void fillFileBlocks(File& _file);

	};
}
#endif

