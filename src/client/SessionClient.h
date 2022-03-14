#ifndef SESSIONCLIENT_H
#define SESSIONCLIENT_H
#include <iostream>
#include <cstring>
#include <string>

class SessionClient {
public:
	SessionClient();
	SessionClient(const SessionClient&) = delete;
	static const std::string prompt;
	
	void setSessionBuffer(char* _buf);
	bool s2c(std::string& sinbuf);
	void c2s(const std::string& _dataout);

private:
	char* sbuf;
	static constexpr size_t SBUF_SIZE = 1024;

	void sbufClr();
	void sbufMore(bool _Flag);
};
#endif

