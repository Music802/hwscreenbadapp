#pragma once

#include <chrono>
#include <stdio.h>
//#define SAVE_H264

#include "ffmpeg_header.h"
#include "Encoder/H264Encoder.h"

class ffcapture
{
public:
	static void Init();
	ffcapture();
	~ffcapture();
	bool SetParams(int x,int y,int w,int h,int fps,int dstW=0,int dstH=0);
	int	GetNextFrame(std::list<hwss::timedpacket> &outFrames);
	bool GetSPSPPS(hwss::datapacket &sps, hwss::datapacket &pps);
private:
	bool CreateCapturer(int x, int y, int w, int h, int fps, int dstW = 0, int dstH = 0);
	void ReleaseCapturer();
	void CreateScaler();
	void ReleaseCaler();
private:
	AVFormatContext *m_fmtCtx;
	AVDictionary *m_options;
	AVInputFormat *m_inputFmt;
	int	m_x;
	int m_y;
	int m_w;
	int m_h;
	int m_fps;
	int m_dstWidth;
	int m_dstHeight;

	AVPacket *m_pktDesktop;
	AVCodecContext *m_codecCtx;
	AVCodec *m_codec;
	int	m_videChannel;
	AVFrame *m_frameSrc;
	AVFrame *m_frameYUV;

	SwsContext *m_sws;
	AVPixelFormat m_yuvFMT;

	H264Encoder *m_encoder;
	std::chrono::milliseconds m_lastTime;
	int64_t m_pts;
#ifdef SAVE_H264
	FILE *m_fp;
#endif // SAVE_H264

};

