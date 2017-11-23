#pragma once

#include <QObject>
#include <qtcpsocket.h>
#include <qstring.h>
#include <qevent.h>
#include "ScreenRenderer.h"
#include "UDPServer.h"
#include "KCP/QKCPNetworker.h"

class ScreenPlayer : public QObject
{
	Q_OBJECT

public:
	ScreenPlayer(QObject *parent,int ssrc,QTcpSocket *conn);
	~ScreenPlayer();
signals:
	void RemoveConn(int ssrc);
	void AddLog(QString log);
private slots:
	void onNewData();
	void onDisconnected();
	void ForceClose();
private:
	void HandleTCPData(QByteArray data,int size);
	void handleConnect(QByteArray data, int size);
	void handleVideoInfo(QByteArray data, int size);
	void handleLocation(QByteArray data, int size);
	void handleHeartbeat(QByteArray data, int size);
	void handleClose(QByteArray data, int size);
	void handleConnResult(QByteArray data, int size);
	void handleResult(QByteArray data, int size);
private:
	QTcpSocket *m_conn;
	QByteArray m_recvCache;
	ScreenRenderer *m_renderer;
	IKCPNetworker *m_kcpServer;
	int m_ssrc = 0;
};
