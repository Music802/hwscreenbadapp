#pragma once

#include <QObject>
#include <qtcpserver.h>
#include <qtcpsocket.h>
#include <quuid.h>
#include "ScreenPlayer.h"
#include <map>
#include <mutex>

class TCPServer : public QObject
{
	Q_OBJECT

public:
	TCPServer(QObject *parent=nullptr);
	~TCPServer();
signals:
	void AddLog(QString log);
	public slots :
		void RemoveConn(QString id);
		void AddLogSlot(QString log);
private slots :
	void onNewConn();
	
private:
	int m_maxConn;
	QTcpServer m_tcpServer;
	std::mutex m_muxPlayers;
	std::map<QString, ScreenPlayer*> m_players;
	bool m_shutdowned;
};
