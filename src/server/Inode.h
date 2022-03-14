#ifndef INODE_H
#define INODE_H
#include "BasDef.h"
#include <bitset>
#include <cstring>
#include <string>
#include <vector>

namespace FS {
	class Inode {
	public:
		static constexpr ull CLASS_SIZE = 64;
		static constexpr tLinkCnt MIN_LINK_CNT = char(0x00);
		static constexpr tLinkCnt MAX_LINK_CNT = char(0xff);
		static constexpr size_t PAGE_CNT_INODE = BLOCKSIZE / sizeof(tBlockID);
		static constexpr size_t MAX_FILE_BLOCK_NUM = 9 + PAGE_CNT_INODE + PAGE_CNT_INODE * PAGE_CNT_INODE;

		Inode();
		Inode(const Inode& _Right);
		void reset();

		bool testUsed();
		void setUsed(bool _Flag);

		bool testWrite(tUid _Uid);
		bool testRead(tUid _Uid);
		std::string getModeString();
		void setWriteU(bool _Flag);
		void setWrite(bool _Flag);
		void setReadU(bool _Flag);
		void setRead(bool _Flag);
		bool setRW(const std::string& _au);

		/*tInodeID getID();
		void setID(tInodeID _ID);*/

		tUid getUid();
		void setUid(tUid _Uid);

		tLinkCnt getLinkCnt();
		bool setLinkUp();
		bool setLinkDown();
		void setLink(tLinkCnt _Val);

		tFileSize getFileSize();
		void setFileSize(tFileSize _Size);

		tBlockID getBlockCnt();
		void setBlockCnt(tBlockID _Val);

		std::vector<tBlockID> getBlockX1();
		const tBlockID* getBlockX1_p();
		tBlockID getBlockX1(int _Pos);
		tBlockID getBlockX2();
		tBlockID getBlockX3();
		void setBlockX1(int _Pos, tBlockID _Val);
		void setBlockX2(tBlockID _Val);
		void setBlockX3(tBlockID _Val);


	protected:
		std::bitset<32> bits;
		//tInodeID id;
		tUid uid;
		tLinkCnt linkCnt;
		tFileSize filesize;
		tBlockID blockCnt;
		tBlockID blockX1[10];
		tBlockID blockX2;
		tBlockID blockX3;
	};
}

#endif

