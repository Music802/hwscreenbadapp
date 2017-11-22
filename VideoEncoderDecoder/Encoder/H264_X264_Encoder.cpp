#include "H264_X264_Encoder.h"


//#pragma comment(lib,"x264/libx264")
//
//X264Encoder::X264Encoder()
//{
//}
//
//
//X264Encoder::~X264Encoder()
//{
//}
//
//bool X264Encoder::Init(int fps, int width, int height)
//{
//	m_fps = fps;
//	m_width = width;
//	m_height = height;
//	if (0 != x264_param_default_preset(&m_param, "veryfast", 0)) {
//		return false;
//	}
//
//	m_param.rc.i_vbv_max_bitrate = 100000;
//	m_param.rc.i_vbv_buffer_size = m_param.rc.i_vbv_max_bitrate;
//
//	m_param.rc.i_rc_method = X264_RC_CRF;
//	m_param.rc.f_rf_constant = 20;
//
//	m_param.b_vfr_input = 1;
//	m_param.i_width = width;
//	m_param.i_height = height;
//	m_param.vui.b_fullrange = 0;          //specify full range input levels
//	m_param.i_keyint_max = fps * 4;      //keyframe every 4 sec, should make this an option
//	m_param.i_keyint_min = fps;
//
//	m_param.i_fps_num = fps;
//	m_param.i_fps_den = 1;
//
//	m_param.i_timebase_num = 1;
//	m_param.i_timebase_den = 1000;
//
//	m_param.i_csp = X264_CSP_NV12;
//
//	m_x264 = x264_encoder_open(&m_param);
//
//	return true;
//}

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
