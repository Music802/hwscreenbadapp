#pragma once
#include <qstring.h>
class Config
{
public:
	static Config &GetInstance();
	~Config();
	bool SetFileUrl(QString fileName);
	int Port();
	int MaxConn();
	int RTPMaxDelaySeq();
private:
	Config();
	int m_port;
	int m_maxConn;
	int m_rtpMaxDelaySeq;
};

