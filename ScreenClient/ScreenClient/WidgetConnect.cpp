#include "WidgetConnect.h"
#include <qmessagebox.h>
#include "ClientConfig.h"

static const QString ConfigIP_KEY = "remoteIP";
static const QString ConfigPort_KEY = "remotePort";

WidgetConnect::WidgetConnect(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	m_conn = new QTcpSocket(this);
	auto remoteIP = ClientConfig::GetInstance().ReadValue(ConfigIP_KEY).toString();
	auto remotePort = ClientConfig::GetInstance().ReadValue(ConfigPort_KEY).toString();
	if (remoteIP.size()<=0)
	{
		remoteIP = "127.0.0.1";
	}
	if (remotePort.size()<=0)
	{
		remotePort = "3000";
	}
	ui.lineIP->setText(remoteIP);
	ui.linePort->setText(remotePort);

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

	auto remoteIP = ui.lineIP->text();
	auto remotePort = ui.linePort->text().toInt();

	ClientConfig::GetInstance().SetValue(ConfigIP_KEY, remoteIP);
	ClientConfig::GetInstance().SetValue(ConfigPort_KEY, remotePort);

	m_conn->connectToHost(remoteIP, remotePort);
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