#include <qjsonobject.h>
#include <qjsondocument.h>
#include "ScreenPlayer.h"
#include "PLAY_CMDS.h"

ScreenPlayer::ScreenPlayer(QObject *parent, int ssrc, QTcpSocket *conn)
	: QObject(parent),
	m_ssrc(ssrc),
	m_conn(conn), 
	m_kcpServer(nullptr),
	m_renderer(nullptr)
{
	QObject::connect(m_conn, &QTcpSocket::readyRead, this, &ScreenPlayer::onNewData);
	QObject::connect(m_conn, &QTcpSocket::disconnected, this, &ScreenPlayer::onDisconnected);

	//create udp server and renderer
	m_kcpServer = new IKCPNetworker();
	m_renderer = new ScreenRenderer(nullptr);
	QObject::connect(m_kcpServer, &IKCPNetworker::ProcessDatagramsSignal, m_renderer, &ScreenRenderer::OnNewDatagram);
}

ScreenPlayer::~ScreenPlayer()
{
	ForceClose();
}

void ScreenPlayer::onNewData() {
	auto buf = m_conn->readAll();
	if (!buf.isEmpty())
	{
		m_recvCache.append(buf);
		if (m_recvCache.length() >= 9)
		{
			int size = (int(m_recvCache[0]) << 24) |
				(int(m_recvCache[1]) << 16) |
				(int(m_recvCache[2]) << 8) |
				(int(m_recvCache[3]) << 0);
			if (size <= 0)
			{
				m_conn->close();
				return;
			}
			if (m_recvCache.size() >= size)
			{
				HandleTCPData(m_recvCache, size);
				m_recvCache.remove(0, size);
			}
		}
	}
}

void ScreenPlayer::onDisconnected() {
	QString strLog;
	strLog.setNum(m_ssrc);
	strLog += " connect closed.";
	AddLog(strLog);
	m_conn->close();
	RemoveConn(m_ssrc);
}

void ScreenPlayer::ForceClose() {
	if (m_conn&&m_conn->isOpen())
	{
		m_conn->close();
		m_conn = nullptr;
	}
	if (nullptr != m_kcpServer)
	{
		delete m_kcpServer;
		m_kcpServer = nullptr;
	}

	if (m_renderer != nullptr)
	{
		delete m_renderer;
		m_renderer = nullptr;
	}
}

void ScreenPlayer::HandleTCPData(QByteArray data, int size)
{
	int cur = 4;
	auto idx = (int(data[cur++]) << 24) |
		(int(data[cur++]) << 16) |
		(int(data[cur++]) << 8) |
		(int(data[cur++]) << 0);
	PLAY_CMD cmd = PLAY_CMD(int(data[cur++]));
	QJsonObject json;
	QJsonDocument doc;
	switch (cmd)
	{
	case OPT_CONNECT:
		handleConnect(data, size);
		break;
	case OPT_SET_VIDEO_INFO:
		handleVideoInfo(data, size);
		break;
	case OPT_SET_LOCATION:
		handleLocation(data, size);
		break;
	case OPT_HEARTBEAT:
		handleHeartbeat(data, size);
		break;
	case OPT_CLOSE:
		handleClose(data, size);
		break;
	case OPT_CONNECT_RESULT:
		handleConnResult(data, size);
		break;
	case OPT_RESULT:
		handleResult(data, size);
		break;
	default:
		break;
	}
}

void ScreenPlayer::handleConnect(QByteArray data, int size)
{
	

	QJsonObject json;
	QJsonDocument doc;
	json.insert("port", m_kcpServer->Port());
	json.insert("ssrc", m_ssrc);
	doc.setObject(json);
	auto jsonArray = doc.toJson();
	auto totalSize = 9 + jsonArray.size();
	QByteArray arrSend;
	arrSend.resize(4);
	arrSend[0] = char((totalSize >> 24) & 0xff);
	arrSend[1] = char((totalSize >> 16) & 0xff);
	arrSend[2] = char((totalSize >> 8) & 0xff);
	arrSend[3] = char((totalSize >> 0) & 0xff);
	arrSend.append(data.data() + 4, 4);
	arrSend.append(char(OPT_CONNECT_RESULT));
	arrSend.append(jsonArray);

	m_conn->write(arrSend);

}

void ScreenPlayer::handleVideoInfo(QByteArray data, int size)
{
	
	//parse info
	bool result = true;
	int width, height;
	QString sps64, pps64;
	do
	{
		QJsonObject jsonIn;
		QJsonDocument docIn;
		QByteArray tmpArray;
		QJsonParseError err;
		tmpArray.resize(size - 9);
		memcpy(tmpArray.data(), data.data() + 9, size - 9);
		docIn = QJsonDocument::fromJson(tmpArray, &err);
		if (docIn.isNull() || err.error != QJsonParseError::NoError|| !docIn.isObject()) {
			result = false;
			break;
		}else {
			auto obj = docIn.object();
			if (obj.contains("w"))
			{
				width = obj.value("w").toInt();
			}
			else {
				result = false;
				break;
			}

			if (obj.contains("h"))
			{
				height = obj.value("h").toInt();
			}
			else {
				result = false;
				break;
			}

			if (obj.contains("sps64"))
			{
				sps64 = obj.value("sps64").toString();
			}
			else {
				result = false;
				break;
			}

			if (obj.contains("pps64"))
			{
				pps64 = obj.value("pps64").toString();
			}
			else {
				result = false;
				break;
			}
		}
	} while (0);

	//set video info
	if (result)
	{
		m_renderer->SetVideoInfo(width, height, sps64, pps64);
	}
	QString strLog = "set video info\n width=" + QString::number(width) +
		"  height=" + QString::number(height) + " \n sps:" + sps64 +
		" \n pps:" + pps64;
	AddLog(strLog);
	//send result
	{
		QJsonObject json;
		QJsonDocument doc;
		json.insert("result", result);
		doc.setObject(json);
		auto jsonArray = doc.toJson();
		auto totalSize = 9 + jsonArray.size();
		QByteArray arrSend;
		arrSend.resize(4);
		arrSend[0] = char((totalSize >> 24) & 0xff);
		arrSend[1] = char((totalSize >> 16) & 0xff);
		arrSend[2] = char((totalSize >> 8) & 0xff);
		arrSend[3] = char((totalSize >> 0) & 0xff);
		arrSend.append(data.data() + 4, 4);
		arrSend.append(char(OPT_RESULT));
		arrSend.append(jsonArray);

		m_conn->write(arrSend);
	}
}

void ScreenPlayer::handleLocation(QByteArray data, int size)
{
	bool result = false;
	int x = 0, y = 0, w = 0, h = 0;
	do
	{
		QJsonObject jsonIn;
		QJsonDocument docIn;
		QByteArray tmpArray;
		QJsonParseError err;
		tmpArray.resize(size - 9);
		memcpy(tmpArray.data(), data.data() + 9, size - 9);
		docIn = QJsonDocument::fromJson(tmpArray, &err);
		if (docIn.isNull() || err.error != QJsonParseError::NoError|| !docIn.isObject()) {
			break;
		}
		else
		{
			auto obj = docIn.object();

			if (obj.contains("x")&&obj.contains("y") && obj.contains("h") && obj.contains("h"))
			{
				x = obj.value("x").toInt();
				y = obj.value("y").toInt();
				w = obj.value("w").toInt();
				h = obj.value("h").toInt();
			}
			else
			{
				result = false;
				break;
			}
		}
		result = true;
	} while (0);

	if (result)
	{
		QString strlog = "set location: x=" + QString::number(x) + "   y=" + QString::number(y);
		AddLog(strlog);
		m_renderer->SetLocation(x, y,w,h);
	}

	//send result
	{
		QJsonObject json;
		QJsonDocument doc;
		json.insert("result", result);
		doc.setObject(json);
		auto jsonArray = doc.toJson();
		auto totalSize = 9 + jsonArray.size();
		QByteArray arrSend;
		arrSend.resize(4);
		arrSend[0] = char((totalSize >> 24) & 0xff);
		arrSend[1] = char((totalSize >> 16) & 0xff);
		arrSend[2] = char((totalSize >> 8) & 0xff);
		arrSend[3] = char((totalSize >> 0) & 0xff);
		arrSend.append(data.data() + 4, 4);
		arrSend.append(char(OPT_RESULT));
		arrSend.append(jsonArray);

		m_conn->write(arrSend);
	}
}

void ScreenPlayer::handleHeartbeat(QByteArray data, int size)
{
}

void ScreenPlayer::handleClose(QByteArray data, int size)
{
	//just close connect
	ForceClose();
}

void ScreenPlayer::handleConnResult(QByteArray data, int size)
{

}

void ScreenPlayer::handleResult(QByteArray data, int size)
{
}
