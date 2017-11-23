#include "ClientConfig.h"


ClientConfig::ClientConfig()
{
	m_settings = new QSettings("Client.ini", QSettings::IniFormat);
}


ClientConfig & ClientConfig::GetInstance()
{
	// TODO: insert return statement here
	static ClientConfig config;
	return config;
}

ClientConfig::~ClientConfig()
{
	delete m_settings;
	m_settings = nullptr;
}

QVariant ClientConfig::ReadValue(QString key)
{
	return m_settings->value(key);
}

void ClientConfig::SetValue(QString key, QVariant value)
{
	m_settings->setValue(key, value);
}

