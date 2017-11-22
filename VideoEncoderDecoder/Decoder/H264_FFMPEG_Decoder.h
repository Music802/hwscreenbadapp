#pragma once
#include "H264Decoder.h"
#include "../ffmpeg_header.h"
class H264_FFMPEG_Decoder :
	public H264Decoder
{
public:
	H264_FFMPEG_Decoder();
	~H264_FFMPEG_Decoder();
	virtual std::list<std::shared_ptr<H264_IMG_Data>> Decode(hwss::timedpacket frame);
private:
	bool InitDecoder();
	std::list<std::shared_ptr<H264_IMG_Data>> decodeFrame();
private:
	AVCodec *m_codec;
	AVCodecContext *m_codecCtx;
	AVCodecParserContext *m_codecParserCtx;
	AVPacket *m_packet;
	AVFrame *m_frame;
};

