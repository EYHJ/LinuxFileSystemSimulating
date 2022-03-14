#ifndef BASICFILEIO_H
#define BASICFILEIO_H
#include <fstream>
#include <bitset>
#include <array>
#include <cstring>
#include "BasDef.h"
#include "Inode.h"
#include "DirEntry.h"
#include "SBlock.h"

namespace FS {
	class BasicFileIO {
	public:
		void getBlockCstr(tBlockID _ID, char* _dest);
		void writeBlock(tBlockID _ID, char* _blockStr, size_t cnt = BLOCKSIZE);

		tBlockID getAvlBlockID();
		tInodeID getAvlInodeID();
		Inode& getInode(tInodeID _ID);

		void saveSuperBlock();
		void saveInodeBitset();
		void saveBlockBitset();
		void saveInodeList();
		void save();

		explicit BasicFileIO(const char* const _discName);
		BasicFileIO() = delete;
		BasicFileIO(const BasicFileIO&) = delete;
		virtual ~BasicFileIO();
		void formatting();

		static constexpr ull INODE_MAX_SIZE = 65535;
		static constexpr ull BLOCK_MAX_SIZE = 98303;

		SBlock superBlock;
		std::bitset<INODE_MAX_SIZE> inodeBitset;
		std::bitset<BLOCK_MAX_SIZE> blockBitset;
		std::array<Inode, INODE_MAX_SIZE> inodeList;

	private:
		std::fstream ioFile;
	};
}

#endif
