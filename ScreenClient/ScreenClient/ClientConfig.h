#pragma once
#include <qstring.h>
#include <qsettings.h>
class ClientConfig
{
public:
	static ClientConfig &GetInstance();
	~ClientConfig();
	QVariant ReadValue(QString key);
	void SetValue(QString key, QVariant value);
private:
	ClientConfig();
	ClientConfig(const ClientConfig&) = delete;
	QSettings *m_settings;
};

