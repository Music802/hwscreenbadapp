#include "H264_X264_Encoder.h"

H264_X264_Encoder::H264_X264_Encoder(int width, int height, int fps, int bitrate, H264ImgType imgType)
{
}

H264_X264_Encoder::~H264_X264_Encoder()
{
}

bool H264_X264_Encoder::GetVideoInfo(hwss::datapacket & sps, hwss::datapacket & pps)
{
	return false;
}

bool H264_X264_Encoder::Encode(void * picIn, long long timestamp, std::list<hwss::timedpacket>& outFrames)
{
	return false;
}
