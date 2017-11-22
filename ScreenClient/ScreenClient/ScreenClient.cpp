#include "ScreenClient.h"
#include "PLAY_CMDS.h"
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qmessagebox.h>

ScreenClient::ScreenClient(QWidget *parent)
	: QWidget(parent),m_conn(nullptr),m_cmd_idx(0),
	m_capture(nullptr),
	m_threadEncode(nullptr),
	m_udpConn(nullptr),
	m_udpPort(3001)//for test
{
	ui.setupUi(this);
	m_widgetSetup = new WidgetSetup(this);
	m_widgetPlay = new WidgetPlay(this);
	m_widgetConnect = new WidgetConnect(this);

	QObject::connect(m_widgetConnect, &WidgetConnect::ConnectSuccessed,
		this, &ScreenClient::ConnectSuccessed);
	QObject::connect(m_widgetConnect, &WidgetConnect::ConnectEnd,
		this, &ScreenClient::ConnectClosed);
	QObject::connect(m_widgetConnect, &WidgetConnect::RecvedData,
		this, &ScreenClient::NewDataRecved);

	QObject::connect(m_widgetSetup, &WidgetSetup::Setuped,
		this, &ScreenClient::SetupedSlot);
	QObject::connect(m_widgetSetup, &WidgetSetup::Setuped,
		m_widgetPlay, &WidgetPlay::SetDefaultValues);

	QObject::connect(m_widgetPlay, &WidgetPlay::MoveToPos,
		this, &ScreenClient::SendMoveToPos);

	QObject::connect(this, &ScreenClient::AsyncSendSignal, this,
		&ScreenClient::AsyncSendSlot, Qt::BlockingQueuedConnection);

	setWidgetVisiable(PAGE_CONN);
}

ScreenClient::~ScreenClient()
{
	if (nullptr!=m_conn)
	{
		if (m_conn->isOpen())
		{
			m_conn->close();
		}
		m_conn = nullptr;
	}
	ReleaseCaptureStopEncode();

}


void ScreenClient::ConnectClosed()
{
	QMessageBox::warning(nullptr, "socket error",
		"socket closed", QMessageBox::Yes, QMessageBox::Yes);
	setWidgetVisiable(PAGE_CONN);
//stop capture and push
	m_recvCache.clear();
	ReleaseCaptureStopEncode();
}

void ScreenClient::NewDataRecved(QByteArray data)
{
	m_recvCache.append(data);
	if (m_recvCache.size()>=9)
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
			int cur = 4;
			auto idx = (int(m_recvCache[cur++]) << 24) |
				(int(m_recvCache[cur++]) << 16) |
				(int(m_recvCache[cur++]) << 8) |
				(int(m_recvCache[cur++]) << 0);
			PLAY_CMD cmd = PLAY_CMD(int(m_recvCache[cur++]));
			switch (cmd)
			{
			case OPT_CLOSE:
				handleCMDClose(m_recvCache, size);
				break;
			case OPT_CONNECT_RESULT:
				handleCMDConnectResult(m_recvCache, size);
				break;
			case OPT_RESULT:
				handleCMDResult(m_recvCache, size);
				break;
			default:
				break;
			}

			m_recvCache.remove(0, size);
		}
	}
}

void ScreenClient::setWidgetVisiable(SCREEN_OPT_PAGE page)
{
	m_page = PAGE_CONN;
	m_widgetConnect->setVisible(page == PAGE_CONN);
	m_widgetSetup->setVisible(page == PAGE_SETUP);
	m_widgetPlay->setVisible(page == PAGE_PLAY);
}

void ScreenClient::handleCMDClose(QByteArray data, int size)
{
	if (m_conn!=nullptr)
	{
		if (m_conn->isOpen())
		{
			m_conn->close();
		}
	}
	ReleaseCaptureStopEncode();
}

void ScreenClient::handleCMDConnectResult(QByteArray data, int size)
{
	if (size <= 9)
	{
		handleCMDClose(data, size);
		return;
	}
	QJsonObject obj;
	QJsonDocument doc;
	QJsonParseError err;
	QByteArray tmpArray;
	tmpArray.resize(size - 9);
	memcpy(tmpArray.data(), data.data() + 9, size - 9);
	doc = QJsonDocument::fromJson(tmpArray, &err);
	if (err.error != QJsonParseError::NoError || doc.isNull() || !doc.isObject())
	{
		handleCMDClose(data, size);
		return;
	}
	obj = doc.object();
	if (obj.contains("port") && obj.contains("ssrc"))
	{
		m_udpPort = obj.value("port").toInt();
		m_ssrc = obj.value("ssrc").toInt();
	}
	else {
		handleCMDClose(data, size);
		return;
	}
}

void ScreenClient::handleCMDResult(QByteArray data, int size)
{
	if (size<=9)
	{
		handleCMDClose(data, size);
		return;
	}
	QJsonObject obj;
	QJsonDocument doc;
	QJsonParseError err;
	QByteArray tmpArray;
	tmpArray.resize(size - 9);
	memcpy(tmpArray.data(), data.data() + 9, size - 9);
	doc = QJsonDocument::fromJson(tmpArray, &err);
	if (err.error!=QJsonParseError::NoError||doc.isNull()||!doc.isObject())
	{
		handleCMDClose(data, size);
		return;
	}
	obj = doc.object();
	if (obj.contains("result"))
	{
		if (false==obj.value("result").toBool())
		{
			handleCMDClose(data, size);
			return;
		}
	}
	else {
		handleCMDClose(data, size);
		return;
	}
}

void ScreenClient::SendPlayCMD(PLAY_CMD cmd, QByteArray json)
{
	if (++m_cmd_idx>0xffffff)
	{
		m_cmd_idx = 0;
	}
	QByteArray arrSend;
	arrSend.resize(9);
	int size = 9 + json.size();
	int cur = 0;
	arrSend[cur++] = (size >> 24) & 0xff;
	arrSend[cur++] = (size >> 16) & 0xff;
	arrSend[cur++] = (size >> 8) & 0xff;
	arrSend[cur++] = (size >> 0) & 0xff;

	arrSend[cur++] = (m_cmd_idx >> 24) & 0xff;
	arrSend[cur++] = (m_cmd_idx >> 16) & 0xff;
	arrSend[cur++] = (m_cmd_idx >> 8) & 0xff;
	arrSend[cur++] = (m_cmd_idx >> 0) & 0xff;

	arrSend[cur++] = cmd;
	if (json.size()>0)
	{
		arrSend.append(json);
	}
	int ret=m_conn->write(arrSend);
	if (ret<=0)
	{
		ret = -1;
	}
}

void ScreenClient::ConnectSuccessed(QTcpSocket *conn)
{
	m_conn = conn;
	//send connect
	QByteArray arr;
	SendPlayCMD(OPT_CONNECT, arr);

	//change to setup
	setWidgetVisiable(PAGE_SETUP);
}

void ScreenClient::SetupedSlot(int x, int y, int w, int h, int dst_x, 
	int dst_y, int dst_w, int dst_h)
{
	//reset ffmpeg
	ReleaseCaptureStopEncode();

	m_capture = new ffcapture();
	int fps = 25;
	if (false == m_capture->SetParams(x, y, w, h, fps, dst_w, dst_h)) {
		QMessageBox::warning(nullptr, "ffmpeg error",
			"ffmpeg set param and init failed", QMessageBox::Yes, QMessageBox::Yes);
		return;
	}
	//create a thread to encode
	m_threadEncode = new std::thread(&ScreenClient::threadEncode, this,dst_w,dst_h);
	
	//send location
	SendMoveToPos(dst_x, dst_y,dst_w,dst_h);
	setWidgetVisiable(PAGE_PLAY);
	
}

void ScreenClient::SendMoveToPos(int x, int y,int w,int h) {


	QJsonDocument jsonDoc;
	QJsonObject jsonObj;
	jsonObj.insert("x", x);
	jsonObj.insert("y", y);
	jsonObj.insert("w", w);
	jsonObj.insert("h", h);
	jsonDoc.setObject(jsonObj);
	auto jsonArray = jsonDoc.toJson();
	SendPlayCMD(OPT_SET_LOCATION, jsonArray);
	
}


void ScreenClient::SendVideoInfo(int w, int h, unsigned char *sps, int spsSize, unsigned char *pps, int ppsSize) {
	QByteArray baSPS, baPPS;
	baSPS.resize(spsSize);
	memcpy(baSPS.data(), sps, spsSize);
	auto sps64=baSPS.toBase64();

	baPPS.resize(ppsSize);
	memcpy(baPPS.data(), pps, ppsSize);
	auto pps64 = baPPS.toBase64();

	QJsonDocument doc;
	QJsonObject obj;
	obj.insert("w", w);
	obj.insert("h", h);
	QString strSps, strPps;
	strSps.append(sps64);
	strPps.append(pps64);
	obj.insert("sps64", strSps);
	obj.insert("pps64", strPps);

	doc.setObject(obj);

	auto arrjson = doc.toJson();

	AsyncSendSignal(OPT_SET_VIDEO_INFO, arrjson);
	//qt bad tcpsocket
	//SendPlayCMD(OPT_SET_VIDEO_INFO, arrjson);
}

void ScreenClient::AsyncSendSlot(PLAY_CMD cmd, QByteArray json)
{
	SendPlayCMD(OPT_SET_VIDEO_INFO, json);
}

void ScreenClient::threadEncode(int w,int h) {
	//Encode wait sps pps
	//send sps pps
	m_encoding = true;
	std::list<hwss::timedpacket> outFrames;
	int result = 0;
	bool firstFrameGoted = false;
	auto rtpPacker = new hwss::RTP_Packer(hwss::Payload_h264, m_ssrc);
	m_udpConn = new QUdpSocket();
	QHostAddress addr;
	addr=(m_conn->peerAddress());
	//m_udpConn->writeDatagram()
	while (m_encoding)
	{
		for (auto i:outFrames)
		{
			delete[]i.data;
		}
		outFrames.clear();

		result = m_capture->GetNextFrame(outFrames);
		if (result<0)
		{
			break;
		}
		if (outFrames.size()>0)
		{
			if (!firstFrameGoted)
			{
				firstFrameGoted = true;
				hwss::datapacket sps, pps;
				m_encoding = m_capture->GetSPSPPS(sps, pps);
				if (!m_encoding)
				{
					break;
				}

				//send sps pps
				SendVideoInfo(w, h, sps.data, sps.size, pps.data, pps.size);

				if (sps.data!=nullptr)
				{
					delete[]sps.data;
				}
				if (pps.data!=nullptr)
				{
					delete[]pps.data;
				}
			}

			if (firstFrameGoted)
			{
				//send h264 frames
				for (auto itFrame:outFrames)
				{
					auto nalType = itFrame.data[0] & 0x1f;
					if (nalType>6)
					{
						//not send sps pps here
						continue;
					}
					unsigned int timestamp =(unsigned int) ((itFrame.timestamp*(hwss::RTP_H264_freq / 1000)) &(0xffffffff - 1));
					auto pktSend = rtpPacker->GenerateH264RTPPackets(itFrame.data, itFrame.size,
						timestamp);
					if (pktSend.size()>0)
					{
						for (auto itSend:pktSend)
						{
							m_udpConn->writeDatagram((const char *)itSend->data, itSend->size,
								addr, m_udpPort);
						}
						//udp send 
					}
				}
				//rtpPacker->GenerateH264RTPPackets()
			}
		}
	}

	for (auto i : outFrames)
	{
		delete[]i.data;
	}
	m_encoding = false;
	delete rtpPacker;

	if (nullptr != m_udpConn)
	{
		if (m_udpConn->isOpen())
		{
			m_udpConn->close();
		}
		delete m_udpConn;
		m_udpConn = nullptr;
	}
}

void ScreenClient::ReleaseCaptureStopEncode()
{
	m_encoding = false;
	if (m_threadEncode!=nullptr)
	{
		if (m_threadEncode->joinable())
		{
			m_threadEncode->join();
		}
		delete m_threadEncode;
		m_threadEncode = nullptr;
	}
	if (m_capture != nullptr)
	{
		delete m_capture;
		m_capture = nullptr;
	}


	if (nullptr != m_udpConn)
	{
		if (m_udpConn->isOpen())
		{
			m_udpConn->close();
		}
		delete m_udpConn;
		m_udpConn = nullptr;
	}
}
