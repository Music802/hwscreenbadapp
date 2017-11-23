#pragma once

#include <QObject>
#include <qtcpserver.h>
#include <qtcpsocket.h>
#include <quuid.h>
#include "ScreenPlayer.h"
#include "RandomIntSet.h"
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
		void RemoveConn(int id);
		void AddLogSlot(QString log);
private slots :
	void onNewConn();
	
private:
	int m_maxConn;
	QTcpServer m_tcpServer;
	std::mutex m_muxPlayers;
	std::map<int, ScreenPlayer*> m_players;
	RandomIntSet m_ssrcSet;
	bool m_shutdowned;
};
