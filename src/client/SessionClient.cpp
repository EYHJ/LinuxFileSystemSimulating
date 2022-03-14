#include "SessionClient.h"
#include <Windows.h>

const std::string SessionClient::prompt = "$";

SessionClient::SessionClient() {
	sbuf = nullptr;
}

void SessionClient::setSessionBuffer(char * _buf) {
	sbuf = _buf;
	sbuf[SBUF_SIZE - 1] = '\0';
}

//server to client
bool SessionClient::s2c(std::string& sinbuf) {
	while(true) {
		if(sbuf[0] == 's')
			break;
		else if(sbuf[0] == '1')
			return false;
		Sleep(25);
	}
	sinbuf.clear();
	while(true) {
		while(sbuf[0] != 's')
			Sleep(25);
		sinbuf.append(std::string(sbuf + 2));
		if(sbuf[1] == '0') {
			sbuf[0] = '0';
			break;
		}
		else
			sbuf[0] = 'c';
	}
	return true;
}

void SessionClient::c2s(const std::string& _dataout) {
	size_t rsize = SBUF_SIZE - 3;
	size_t pos = 0;
	size_t size = _dataout.size();

	size_t wsize;
	bool needMore;
	while(pos < size) {
		while(sbuf[0] != '1')
			Sleep(25);
		sbufClr();
		wsize = size - pos;
		needMore = wsize > rsize;
		memcpy(sbuf + 2, &_dataout[pos], needMore ? rsize : wsize);
		sbufMore(needMore);
		sbuf[0] = 'c';
		pos += rsize;
	}
}

void SessionClient::sbufClr() {
	memset(sbuf + 2, 0, SBUF_SIZE - 2);
}

void SessionClient::sbufMore(bool _Flag) {
	if(_Flag)
		sbuf[1] = '1';
	else
		sbuf[1] = '0';
}
