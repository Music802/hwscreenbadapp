#pragma once
#include "../../CommonUtils/datapacket.h"
#include "H264Encoder.h"
#include <list>
#include <chrono>
#include "../ffmpeg_header.h"

class H264_FFMPEG_Encoder:public H264Encoder
{
public:
	H264_FFMPEG_Encoder(int width, int height, int fps, int bitrate, H264ImgType imgType);
	~H264_FFMPEG_Encoder();
	virtual bool GetVideoInfo(hwss::datapacket &sps, hwss::datapacket &pps) __override;
	virtual bool Encode(void *picIn, long long  timestamp, std::list<hwss::timedpacket> &outFrames) __override;
private:
	H264_FFMPEG_Encoder(const H264_FFMPEG_Encoder&)=delete;
	bool InitEncoder();
	void ReleaseEncoder();
	void ReadSPSPPS(unsigned char * data, int size);
	char* GetFrame(char *ptr_data, int buf_size, char **ptr_frame, int &frame_len);
private:
	int m_w, m_h, m_fps;
	int m_bitrate;
	AVCodec *m_codec;
	AVCodecContext *m_codecCtx;
	AVPacket *m_pkt264;
	bool m_inited;
	AVPixelFormat m_pixFMT;
	int64_t m_frameIdx;

	unsigned char *m_sps;
	int m_spsSize;
	unsigned char *m_pps;
	int m_ppsSize;
};

