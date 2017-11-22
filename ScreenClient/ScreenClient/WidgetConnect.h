#pragma once

#include <QWidget>
#include <qtcpsocket.h>
#include "ui_WidgetConnect.h"

class WidgetConnect : public QWidget
{
	Q_OBJECT

public:
	WidgetConnect(QWidget *parent = Q_NULLPTR);
	~WidgetConnect();
signals:
	void ConnectSuccessed(QTcpSocket *conn);
	void ConnectEnd();
	void RecvedData(QByteArray data);
	private slots:
	void Clicked();
	void SocketConnectedSlot();
	void SocketDisconnectSlot();
	void SocketErrorSlot(QAbstractSocket::SocketError a);
	void SocketReadableSlot();
private:
	Ui::WidgetConnect ui;
	QTcpSocket *m_conn;
};
