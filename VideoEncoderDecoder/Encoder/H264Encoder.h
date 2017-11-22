#pragma once
#include <list>
#include "../../CommonUtils/datapacket.h"
#include "../ffmpeg_header.h"
enum H264EncoderType
{
	H264_Encoder_FFMPEG,
	H264_Encoder_X264
};
class H264Encoder
{
public:
	static H264Encoder *CreateEncoder(H264EncoderType encoderType,
		int width,int height,int fps,int bitrate, H264ImgType imgType);
	virtual bool GetVideoInfo(hwss::datapacket &sps, hwss::datapacket &pps) = 0;
	virtual bool Encode(void *picIn, long long  timestamp, std::list<hwss::timedpacket> &outFrames) = 0;
	virtual ~H264Encoder();
	H264Encoder();
private:
	H264Encoder(const H264Encoder&);
};

