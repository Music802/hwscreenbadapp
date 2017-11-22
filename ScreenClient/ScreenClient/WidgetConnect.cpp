#include "WidgetConnect.h"
#include <qmessagebox.h>

WidgetConnect::WidgetConnect(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	m_conn = new QTcpSocket(this);
	ui.lineIP->setText("127.0.0.1");
	ui.linePort->setText("3000");

	QObject::connect(m_conn, &QTcpSocket::connected, this, &WidgetConnect::SocketConnectedSlot);
	QObject::connect(m_conn, &QTcpSocket::disconnected, this, &WidgetConnect::SocketDisconnectSlot);
	QObject::connect(m_conn, &QTcpSocket::readyRead, this, &WidgetConnect::SocketReadableSlot);
	QObject::connect(m_conn, SIGNAL(error(QAbstractSocket::SocketError)),
		this, SLOT(SocketErrorSlot(QAbstractSocket::SocketError)));

	QObject::connect(ui.BtnConnect, &QPushButton::clicked, this, &WidgetConnect::Clicked);
}

WidgetConnect::~WidgetConnect()
{
	if (m_conn!=nullptr)
	{
		if (m_conn->isOpen())
		{
			m_conn->close();
		}
		m_conn = nullptr;
	}
}

void WidgetConnect::Clicked() {
	if (m_conn->isOpen())
	{
		m_conn->close();
		return;
	}
	m_conn->connectToHost(ui.lineIP->text(), ui.linePort->text().toInt());
}

void WidgetConnect::SocketConnectedSlot() {
	
	
	ConnectSuccessed(m_conn);
}

void WidgetConnect::SocketDisconnectSlot() {
	if (m_conn!=nullptr)
	{
		if (m_conn->isOpen())
		{
			m_conn->close();
		}
		ConnectEnd();
	}
}

void WidgetConnect::SocketReadableSlot() {
	if (m_conn->isReadable())
	{
		auto buf = m_conn->readAll();
		RecvedData(buf);
	}
}

void WidgetConnect::SocketErrorSlot(QAbstractSocket::SocketError a) {
	QMessageBox::warning(nullptr, "connect error",
		"connect failed", QMessageBox::Yes, QMessageBox::Yes);
}