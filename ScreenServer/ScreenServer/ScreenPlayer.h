#pragma once

#include <QObject>
#include <qtcpsocket.h>
#include <qstring.h>
#include <qevent.h>
#include "ScreenRenderer.h"
#include "UDPServer.h"

class ScreenPlayer : public QObject
{
	Q_OBJECT

public:
	ScreenPlayer(QObject *parent,QString id,QTcpSocket *conn);
	~ScreenPlayer();
signals:
	void RemoveConn(QString id);
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
	QString m_id;
	QTcpSocket *m_conn;
	QByteArray m_recvCache;
	ScreenRenderer *m_renderer;
	UDPServer *m_udpSvr;
	int m_ssrc;
};
