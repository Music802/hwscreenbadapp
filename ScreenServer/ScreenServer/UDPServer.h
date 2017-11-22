#pragma once

#include <QObject>
#include <qudpsocket.h>

class UDPServer : public QObject
{
	Q_OBJECT

public:
	UDPServer(QObject *parent);
	~UDPServer();
	int UDPPort();
signals:
	void NewDatagrams(QByteArray data);
private slots:
	void onNewData();
private:
	void Shutdown();

private:
	QUdpSocket *m_conn;
};
