#include "TCPServer.h"
#include "Config.h"


TCPServer::TCPServer(QObject *parent)
	: QObject(parent), m_tcpServer(nullptr), m_shutdowned(false)
{
	m_maxConn = Config::GetInstance().MaxConn();
	m_tcpServer.listen(QHostAddress::Any, Config::GetInstance().Port());
	QObject::connect(&m_tcpServer,&QTcpServer::newConnection,
		this, &TCPServer::onNewConn);
}

TCPServer::~TCPServer()
{
	m_shutdowned = true;
	std::lock_guard<std::mutex> guard(m_muxPlayers);
	for (auto it:m_players )
	{
		delete it.second;
	}
	m_players.clear();
}

void TCPServer::AddLogSlot(QString log)
{
	AddLog(log);
}

void TCPServer::onNewConn() {

	std::lock_guard<std::mutex> gurad(m_muxPlayers);
	if (m_players.size()>=m_maxConn)
	{
		auto fd = m_tcpServer.nextPendingConnection();
		fd->close();
		return;
	}
	auto fd = m_tcpServer.nextPendingConnection();
	auto id = QUuid::createUuid().toString();
	ScreenPlayer *player = new ScreenPlayer(nullptr, id, fd);
	m_players[id] = player;


	QObject::connect(player, &ScreenPlayer::RemoveConn, this, &TCPServer::RemoveConn);
	QObject::connect(player, &ScreenPlayer::AddLog, this, &TCPServer::AddLogSlot);
	QString strLog = "new conn id = "+id+"   from" + fd->peerAddress().toString();
	AddLog(strLog);
}

void TCPServer::RemoveConn(QString id) {
	if (m_shutdowned)
	{
		return;
	}
	std::lock_guard<std::mutex> guard(m_muxPlayers);
	auto it = m_players.find(id);
	if (it!=m_players.end())
	{
		delete it->second;
		m_players.erase(it);
	}
}
