#pragma once
#include <stdint.h>
#include "../../CommonUtils/datapacket.h"
#include "H264Encoder.h"
#include <list>
//#include <x264.h>
//extern "C"
//{
//#pragma comment(lib,"libx264.lib")
//}
class H264_X264_Encoder
{
public:
	H264_X264_Encoder(int width, int height, int fps, int bitrate, H264ImgType imgType);
	~H264_X264_Encoder();
	virtual bool GetVideoInfo(hwss::datapacket &sps, hwss::datapacket &pps) __override;
	virtual bool Encode(void *picIn, long long  timestamp, std::list<hwss::timedpacket> &outFrames) __override;
private:
	H264_X264_Encoder(const H264_X264_Encoder&);
	/*x264_param_t m_param;
	x264_t *m_x264;

	x264_picture_t m_pixOut;
	x264_nal_t *m_pp_nal;
	int m_pi_nal;
	int m_width, m_height;
	int m_fps;*/
};

