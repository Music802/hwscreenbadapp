#include "Config.h"
#include <qfile.h>
#include <qjsonobject.h>
#include <qjsondocument.h>
#include <qjsonvalue.h>


Config::Config():m_port(0)
{
}


Config & Config::GetInstance()
{
	// TODO: insert return statement here
	static Config cfg;
	return cfg;
}

Config::~Config()
{
}

bool Config::SetFileUrl(QString fileName)
{
	QFile fp(fileName);
	if (!fp.open(QIODevice::ReadOnly))
	{
		return false;
	}
	auto bytesData = fp.readAll();

	QJsonParseError err;
	auto doc = QJsonDocument::fromJson(bytesData, &err);
	if (!doc.isNull()&&err.error==QJsonParseError::NoError)
	{
		if (doc.isObject())
		{
			auto obj = doc.object();
			if (obj.contains("port"))
			{
				if (obj.value("port").isDouble())
				{
					m_port = obj.value("port").toInt();
				}
			}
			if (obj.contains("maxConn"))
			{
				m_maxConn = obj.value("maxConn").toInt();
			}
			if (obj.contains("rtpMaxDelaySeq"))
			{
				m_rtpMaxDelaySeq = obj.value("rtpMaxDelaySeq").toInt();
			}
			if (m_maxConn<1)
			{
				m_maxConn = 1;
			}
			if (m_rtpMaxDelaySeq<5)
			{
				m_rtpMaxDelaySeq = 5;
			}
		}
	}
	else {
		fp.close();
		return false;
	}

	fp.close();
	return true;
}

int Config::Port()
{
	return m_port;
}

int Config::MaxConn()
{
	return m_maxConn;
}

int Config::RTPMaxDelaySeq()
{
	return m_rtpMaxDelaySeq;
}
