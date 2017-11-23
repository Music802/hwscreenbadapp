#pragma once
#include <stdint.h>
#include "../../CommonUtils/datapacket.h"
#include "H264Encoder.h"
#include <list>
class H264_X264_Encoder
{
public:
	H264_X264_Encoder(int width, int height, int fps, int bitrate, H264ImgType imgType);
	~H264_X264_Encoder();
	virtual bool GetVideoInfo(hwss::datapacket &sps, hwss::datapacket &pps) __override;
	virtual bool Encode(void *picIn, long long  timestamp, std::list<hwss::timedpacket> &outFrames) __override;
private:
	H264_X264_Encoder(const H264_X264_Encoder&);
};

