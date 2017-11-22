#pragma once

#include <QObject>
#include <RTP/RTP_UnPacker.h>
#include "VideoEncoderDecoder/Decoder/H264Decoder.h"
#include "SDLWidget/SDLWidget.h"
//#define SAVE_H264 1
class ScreenRenderer : public QObject
{
	Q_OBJECT

public:
	ScreenRenderer(QObject *parent);
	~ScreenRenderer();
	void SetVideoInfo(int width, int height, QString sps64, QString pps64);
	void SetLocation(int x, int y,int w,int h);
	public slots:
	void OnNewDatagram(const char *buf,int len);
private:
	void ReleaseRTPUnpacker();
	void ReleaseDecoder();
	void ReleaseSDL();
private:
	unsigned char *m_sps;
	int m_spsLength;
	unsigned char *m_pps;
	int m_ppsLength;
	int m_width;
	int m_height;
	int m_x;
	int m_y;
#if SAVE_H264
	FILE *m_fp;
#endif // SAVE_H264


	hwss::RTP_UnPacker *m_rtpUnPacker;
	H264Decoder *m_decoder;
	SDLWidget *m_sdlWidgt;
};
