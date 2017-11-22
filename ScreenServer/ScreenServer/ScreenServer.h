#pragma once

#include <QtWidgets/QWidget>
#include "ui_ScreenServer.h"
#include "TCPServer.h"


class ScreenServer : public QWidget
{
	Q_OBJECT

public:
	ScreenServer(QWidget *parent = Q_NULLPTR);
	public slots:
	void AddLog(QString log);
private:
	Ui::ScreenServerClass ui;
	TCPServer m_tcpServer;
};
