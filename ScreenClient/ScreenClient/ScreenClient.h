#pragma once

#include <QtWidgets/QWidget>
#include <qtcpsocket.h>
#include <qudpsocket.h>
#include "ui_ScreenClient.h"
#include "WidgetConnect.h"
#include "WidgetSetup.h"
#include "WidgetPlay.h"
#include "PLAY_CMDS.h"

#include "VideoEncoderDecoder/ffcapture.h"
#include "RTP/RTP_Packer.h"

#include <thread>



class ScreenClient : public QWidget
{
	Q_OBJECT

public:
	ScreenClient(QWidget *parent = Q_NULLPTR);
	~ScreenClient();
private:
	enum SCREEN_OPT_PAGE
{
	PAGE_CONN,
	PAGE_SETUP,
	PAGE_PLAY
};
signals:
	void AsyncSendSignal(PLAY_CMD cmd, QByteArray json);
	private slots:
	void ConnectSuccessed(QTcpSocket *conn);
	void ConnectClosed();
	void NewDataRecved(QByteArray data);
	void SetupedSlot(int x, int y, int w, int h, int dst_x, int dst_y, int dst_w, int dst_h);
	void SendMoveToPos(int x, int y,int w,int h);
	void SendVideoInfo(int w,int h,unsigned char *sps,int spsSize,unsigned char *pps,int ppsSize);
	void AsyncSendSlot(PLAY_CMD cmd, QByteArray json);
private:
	void setWidgetVisiable(SCREEN_OPT_PAGE page);
	void handleCMDClose(QByteArray data,int size);
	void handleCMDConnectResult(QByteArray data, int size);
	void handleCMDResult(QByteArray data, int size);
	void SendPlayCMD(PLAY_CMD cmd, QByteArray json);


	void threadEncode(int w,int h);
	void ReleaseCaptureStopEncode();

	Ui::ScreenClientClass ui;
	WidgetConnect *m_widgetConnect;
	WidgetSetup		*m_widgetSetup;
	WidgetPlay		*m_widgetPlay;

	SCREEN_OPT_PAGE m_page;
	QTcpSocket *m_conn;
	int m_cmd_idx;
	QByteArray m_recvCache;
	int	m_udpPort;
	int m_ssrc;
	QUdpSocket *m_udpConn;

	/*
	screen capture
	*/
	ffcapture *m_capture;
	std::thread *m_threadEncode;
	volatile bool m_encoding;
	bool m_useTCP;
	//rtp
};
