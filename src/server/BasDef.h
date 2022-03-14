#ifndef BASDEF_H
#define BASDEF_H

namespace FS {

	using ull = unsigned long long;
	using tInodeID = unsigned short;
	using tBlockID = unsigned int;
	using tFileSize = unsigned int;
	using tUid = unsigned char;
	using tLinkCnt = char;
	using tdir = unsigned short;

	constexpr size_t BLOCKSIZE = 1024;
	constexpr tBlockID BAD_BLOCK_ID = 0xffffffff;
}

#endif

