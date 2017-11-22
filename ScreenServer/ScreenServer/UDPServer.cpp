#define USE_RECV_DATAGRAM (QT_VERSION_MAJOR>=5&&QT_VERSION_MINOR>=6)

#include "UDPServer.h"

#if USE_RECV_DATAGRAM
#else
#include <qnetworkdatagram.h>
#endif // USE_RECV_DATAGRAM


UDPServer::UDPServer(QObject *parent)
	: QObject(parent),m_conn(0)
{
	m_conn = new	QUdpSocket(this);
	m_conn->bind(QHostAddress::Any);

	QObject::connect(m_conn, &QUdpSocket::readyRead, this, &UDPServer::onNewData);


}

UDPServer::~UDPServer()
{
	Shutdown();
}

int UDPServer::UDPPort()
{
	if (m_conn!=nullptr)
	{
		return m_conn->localPort();
	}
	return -1;
}

void UDPServer::onNewData()
{
#if USE_RECV_DATAGRAM
	while (m_conn->hasPendingDatagrams())
	{
		QByteArray datagram;
		QHostAddress sender;
		quint16 senderPort;

		datagram.resize(m_conn->pendingDatagramSize());
		m_conn->readDatagram(datagram.data(),datagram.size(),
			&sender, &senderPort);
		NewDatagrams(datagram);
	}
#else

	while (m_conn->hasPendingDatagrams())
	{
		auto buf = m_conn->receiveDatagram();

		NewDatagrams(buf.data());
	}
#endif // 0

}

void UDPServer::Shutdown()
{
	if (m_conn!=nullptr)
	{
		if (m_conn->isOpen()) {
			m_conn->close();
		}
		m_conn = nullptr;
	}
}
