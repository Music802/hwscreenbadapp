#include "ScreenServer.h"

ScreenServer::ScreenServer(QWidget *parent)
	: QWidget(parent),m_tcpServer(parent)
{
	ui.setupUi(this);
	QObject::connect(&this->m_tcpServer, &TCPServer::AddLog,
		this, &ScreenServer::AddLog);
}

void ScreenServer::AddLog(QString log) {
	ui.textEdit->append(log);
}
